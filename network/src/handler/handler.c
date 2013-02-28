/*
 * handler.c
 *
 *  Created on: Feb 27, 2013
 *      Author: leonshao
 */
#include "handler.h"
#include "io/ioutil.h"
#include <unistd.h>		// fork(), STDOUT_FILENO, execve(), __environ, dup2()
#include <stdio.h>		// sprintf()
#include <string.h>		// strcasecmp(), strcpy(), strcat()
#include <stdlib.h>		// setenv()
#include <sys/mman.h>	// mmap(), PROT_READ, MAP_PRIVATE
#include <sys/wait.h>	// wait()
#include <fcntl.h>		// O_RDONLY

#define BUF_LEN 8192

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

/* get and return static file content */
void serve_static(int connfd, char *filename, int filesize) {
	char buf[BUF_LEN], filetype[BUF_LEN];
	int filefd;
	char * srcp;

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

void error_to_client(int connfd, char *cause, char *errnum, char *shortmsg, char *longmsg){
	char body[BUF_LEN], buf[BUF_LEN];
	int body_size = 0;

	/* build http response body */
	sprintf(body, "<html><title>Tiny Error</title>");
	sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
	sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
	sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
	sprintf(body, "%s<hr><em>The Tiny Web Server</em>\r\n", body);

	/* send the http response */
	body_size = (int)strlen(body);
	sprintf(buf, "HTTP/1.1 %s %s\r\n", errnum, shortmsg);
	sprintf(buf, "%sContent-type: text/html\r\n", buf);
	sprintf(buf, "%sContent-length: %d\r\n\r\n", buf, body_size);
	io_writen(connfd, buf, strlen(buf));
	io_writen(connfd, body, body_size);
}
