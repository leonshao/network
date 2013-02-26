/*
 * mytiny.c
 *
 *  Created on: Nov 28, 2012
 *      Author: leonshao
 */

#include "socketutil.h"
#include "io/ioutil.h"
#include <errno.h>			// errno, EAGAIN
#include <netdb.h>			// gethostbyaddr()
#include <stdio.h>			// sscanf()
#include <string.h>			// strcasecmp(), strcpy(), strcat()
#include <sys/mman.h>		// mmap(), PROT_READ, MAP_PRIVATE
#include <sys/stat.h>		// stat()
#include <unistd.h>			// fork(), STDOUT_FILENO, execve(), __environ
#include <stdlib.h>			// setenv()
#include <sys/wait.h>		// wait()

void process_req(int connfd);
void error_to_client(int connfd, char *cause, char * errnum, char *shortmsg, char *longmsg);
void read_req_header(io_t *io_buf);
void serve_static(int connfd, char *filename);
void get_file_type(char *filename, char *filetype);
int parse_uri(char *uri, char *filename, char *cgi_args);
void serve_dynamic(int connfd, char *filename, char *cgi_args);


void error_to_client(int connfd, char *cause, char * errnum, char *shortmsg, char *longmsg){

}

void read_req_header(io_t *io_buf) {
	io_print_buf(io_buf);
	return;
}

/* get and return static file content */
void serve_static(int connfd, char *filename) {
	char buf[IO_BUFSIZE], filetype[BUF_LEN];
	int filesize, filefd;
	struct stat file_stat;
	char * srcp;

	stat(filename, &file_stat);
	filesize = file_stat.st_size;
	get_file_type(filename, filetype);

	/* send response header */
	sprintf(buf, "HTTP/1.1 200 OK\r\n");
	sprintf(buf, "%sServer: My Web Server\r\n", buf);
	sprintf(buf, "%sContent-type: %s\r\n", buf, filetype);
	/* extra \r\n to terminate header */
	sprintf(buf, "%sContent-length: %d\r\n\r\n", buf, filesize);
	io_writen(connfd, buf, strlen(buf));

	/* send response body */
	filefd = open(filename, O_RDONLY, 0);
	srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, filefd, 0);
	io_writen(connfd, srcp, filesize);

	munmap(srcp, filesize);
	close(filefd);
}

void get_file_type(char *filename, char *filetype) {
	if (strstr(filename, ".html"))
		strcpy(filetype, "text/html");
	else if (strstr(filename, ".jpg"))
		strcpy(filetype, "image/jpeg");
	else if (strstr(filename, ".gif"))
		strcpy(filetype, "image/gif");
	else
		strcpy(filetype, "text/plain");
}

int parse_uri(char *uri, char *filename, char *cgi_args) {
	char * ptr;

	/* request for static content */
	if (!strstr(uri, "cgi-bin")) {
		strcpy(cgi_args, "");			// clear cgi_args
		strcpy(filename, ".");
		strcat(filename, uri);
		if (uri[strlen(uri)-1] == '/')	// the last char in uri is '/', append file name
			strcat(filename, "home.html");
		return 1;
	}
	/* get "cgi-bin" in the uri string, request for dynamic content
	 * /cgi-bin/adder?a=1&b=2 */
	else {
		ptr = index(uri, '?');
		if (ptr) {
			strcpy(cgi_args, ptr+1);
			*ptr = '\0';
		}
		else
			strcpy(cgi_args, "");
		strcpy(filename, ".");
		strcat(filename, uri);
		return 0;
	}
}

void serve_dynamic(int connfd, char *filename, char *cgi_args) {
	char buf[BUF_LEN], *emptylist[] = { NULL };

	/* send response header and create child process to handle dynamic content */
	sprintf(buf, "HTTP/1.1 200 OK\r\n");
	sprintf(buf, "%sServer: My Web Server\r\n", buf);
	io_writen(connfd, buf, strlen(buf));

	if (fork() == 0) {	/* child process */
		/* pass the args to cgi(adder) via environment QUERY_STRING */
		setenv("QUERY_STRING", cgi_args, 1);
		dup2(connfd, STDOUT_FILENO);			/* redirect output to client */
		execve(filename, emptylist, __environ);	/* execute the CGI program */
	}
	wait(NULL);
}


/* http request handler */
void process_req(int connfd) {
	int is_static;
	io_t io_buf;
	char buf[BUF_LEN];
	char method[10], uri[BUF_LEN], version[10];
	char filename[BUF_LEN], cgi_args[BUF_LEN];

	io_initbuf(&io_buf, connfd);
	io_readlineb(&io_buf, buf, BUF_LEN);

	/*
	 * get http request line, split method, URI and version
	 * GET / HTTP/1.1
	 */
	sscanf(buf, "%s %s %s", method, uri, version);

	/* only support method GET */
	if (strcasecmp(method, "GET")) {
		error_to_client(connfd, method, "501", "Not Implemented",
				"Server does not implement this method");
		return;
	}

	/* print request header */
	read_req_header(&io_buf);

	/* parse URI to get filename and args */
	is_static = parse_uri(uri, filename, cgi_args);

	/* serve for request*/
	if (is_static) {
		serve_static(connfd, filename);
	}
	else {
		serve_dynamic(connfd, filename, cgi_args);
	}
}

void echo(int connfd) {
	size_t n;
	char buf[BUF_LEN];
	io_t io_buf;

	io_initbuf(&io_buf, connfd);
	while((n = io_readlineb(&io_buf, buf, BUF_LEN)) != 0){
		printf("server receive %d bytes data: %s", n, buf);

		io_writen(connfd, buf, n);
	}
}


int main(int argc, char** argv) {
	int listenfd, port, connfd, clientaddrlen;
	struct sockaddr_in clientaddr;

	if (argc != 2) {
		printf("Usage: %s port\n", argv[0]);
		return 1;
	}

	port = atoi(argv[1]);

	// start server
	listenfd = open_listenfd(port);
	clientaddrlen = sizeof(clientaddr);
	while(1){
		connfd = accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientaddrlen);
		check_client_addr(&clientaddr);

		// process request
		process_req(connfd);

		// do not mistake to listenfd!!!
		close(connfd);
	}
	return 0;
}
