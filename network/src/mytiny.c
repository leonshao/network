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
#include <string.h>			// strcasecmp()
#include <sys/mman.h>		// mmap(), PROT_READ, MAP_PRIVATE
#include <sys/stat.h>		// stat()

void process_req(int connfd);
void error_to_client(int connfd, char *cause, char * errnum, char *shortmsg, char *longmsg);
void read_req_header(io_t *io_buf);
void serve_static(int connfd, char *filename);
void get_file_type(char *filename, char *filetype);


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
	char * type;
	if ((type = strstr(filename, "."))) {
		sprintf(filetype, "text/%s", ++type);
	}
}

/* http request handler */
void process_req(int connfd) {
	io_t io_buf;
	char buf[BUF_LEN];
	char method[10], uri[BUF_LEN], version[10];

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
	}

	/* print request header */
	read_req_header(&io_buf);

	/* parse URI */

	/* serve for request*/
	serve_static(connfd, "home.html");
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
		return EXIT_FAILURE;
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

}
