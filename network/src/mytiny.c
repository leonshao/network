/*
 * mytiny.c
 *
 *  Created on: Nov 28, 2012
 *      Author: leonshao
 */

#include "socketutil.h"
#include "io/ioutil.h"
#include "handler/handler.h"
#include <errno.h>			// errno, EAGAIN
#include <netdb.h>			// gethostbyaddr()
#include <stdio.h>			// sscanf()
#include <string.h>			// strcasecmp(), strcpy(), strcat()
#include <sys/mman.h>		// mmap(), PROT_READ, MAP_PRIVATE
#include <sys/stat.h>		// stat()
#include <stdlib.h>			// setenv()
#include <sys/wait.h>		// wait()

void process_req(int connfd);
void read_req_header(io_t *io_buf);
int parse_uri(char *uri, char *filename, char *cgi_args);

void read_req_header(io_t *io_buf) {
	io_print_buf(io_buf);
	return;
}

/* parse URI and determine the static/dynamic request */
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


/* http request handler */
void process_req(int connfd) {
	int is_static;
	io_t io_buf;
	char buf[BUF_LEN];
	char method[10], uri[BUF_LEN], version[10];
	char filename[BUF_LEN], cgi_args[BUF_LEN];
	struct stat file_stat;

	io_initbuf(&io_buf, connfd);
	/* read header into buf */
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

	/* check if file exists */
	if (stat(filename, &file_stat)) {
		error_to_client(connfd, filename, "404", "Not Found",
				"File not found");
		return;
	}

	/* serve for request*/
	if (is_static) {	/* static request */
		if (!(S_ISREG(file_stat.st_mode)) || !(S_IRUSR & file_stat.st_mode)) {
			/* request for not regular file or file can't be read */
			error_to_client(connfd, filename, "403", "Forbidden",
					"File can't read");
			return;
		}
		serve_static(connfd, filename, file_stat.st_size);
	}
	else {				/* dynamic request*/
		if (!(S_ISREG(file_stat.st_mode)) || !(S_IXUSR & file_stat.st_mode)) {
			/* request for not regular file or file can't be executed */
			error_to_client(connfd, filename, "403", "Forbidden",
					"CGI program can't run");
			return;
		}
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
