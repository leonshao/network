/*
 * handler.c
 *
 *  Created on: Feb 27, 2013
 *      Author: leonshao
 */
#include "handler.h"
#include <unistd.h>		// fork(), STDOUT_FILENO, execve(), __environ, dup2()
#include <stdio.h>		// sprintf(), fprintf(), stdout
#include <string.h>		// strcasecmp(), strcpy(), strcat(), strerror()
#include <stdlib.h>		// setenv(), malloc()
#include <fcntl.h>		// open(), O_RDONLY
#include <sys/mman.h>	// mmap(), PROT_READ, MAP_PRIVATE
#include <sys/wait.h>	// wait(), waitpid()
#include <sys/stat.h>	// stat()
#include <signal.h>		// signal()
#include <errno.h>		// errno, ECHILD

#define BUF_LEN 8192


/* SIGCHLD handler */
void sigchld_handler(int sig) {
	pid_t pid;
	int status;

	/* Wait for child processes to die */
	while ((pid = waitpid(-1, &status, 0)) > 0) {
		printf("Child process %d is reaped, status: %d\n", (int)pid, status);
	}

	if (errno != ECHILD) {
		fprintf(stdout, "waitpid error, %s", strerror(errno));
		exit(0);
	}

	sleep(2);
	return;
}

/* SIGPIPE handler */
void sigpipe_handler(int sig) {
	printf("write to closed conn\n");
}


void print_header_200(char *buf) {
	sprintf(buf, "HTTP/1.1 200 OK\r\n");
	sprintf(buf, "%sServer: My Web Server\r\n", buf);
}

void read_req_header(io_t *io_buf) {
	io_print_buf(io_buf);
	return;
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


void get_post_content(io_t *io_buf, char *cgi_args) {
	char buf[BUF_LEN];
	char *ptr;
	int content_len = 0;

	io_readlineb(io_buf, buf, BUF_LEN);
	while (strcmp(buf, "\r\n")) {
		ptr = strchr(buf, ' ');
		if (ptr) {
			content_len = atoi(ptr + 1);
		}
		io_readlineb(io_buf, buf, BUF_LEN);
	}

	io_readnb(io_buf, cgi_args, content_len);
}

/* parse URI and determine the static/dynamic request */
int parse_uri(char *uri, io_t *io_buf, char *filename, char *cgi_args) {
	char * ptr;

	/* request for header */
	if (strstr(uri, "header")) {
		return 2;
	}
	/* request for static content */
	else if (!strstr(uri, "cgi-bin")) {
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
		else {
			/* get args from POST content*/
			get_post_content(io_buf, cgi_args);
		}
		strcpy(filename, ".");
		strcat(filename, uri);
		return 0;
	}
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

/* get and return static file content */
void serve_static(int connfd, char *filename, int filesize) {
	char buf[BUF_LEN], filetype[BUF_LEN];
	char * srcp;
	int filefd;

	get_file_type(filename, filetype);

	/* send response header */
	print_header_200(buf);
	sprintf(buf, "%sContent-type: %s\r\n", buf, filetype);
	/* extra \r\n to terminate header */
	sprintf(buf, "%sContent-length: %d\r\n\r\n", buf, filesize);
	io_writen(connfd, buf, strlen(buf));

	/* send response body */
	filefd = open(filename, O_RDONLY, 0);

	srcp = malloc(filesize);
	io_readn(filefd, srcp, filesize);
	// srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, filefd, 0);
	/* SIGPIPE is generated if connfd closed */
	io_writen(connfd, srcp, filesize);

	/* remember to free/munmap the resource */
	// munmap(srcp, filesize);
	free(srcp);
	close(filefd);
}


void serve_dynamic(int connfd, char *filename, char *cgi_args) {
	char buf[BUF_LEN], *emptylist[] = { NULL };

	/* send response header and create child process to handle dynamic content */
	print_header_200(buf);
	io_writen(connfd, buf, strlen(buf));

	if (fork() == 0) {	/* child process */
		/* pass the args to cgi(adder) via environment QUERY_STRING */
		setenv("QUERY_STRING", cgi_args, 1);
		dup2(connfd, STDOUT_FILENO);			/* redirect output to client */
		execve(filename, emptylist, __environ);	/* execute the CGI program */
	}
}


void serve_header(int connfd, io_t *io_buf) {
	char buf[BUF_LEN], content[BUF_LEN];
	char *ptr_end, *ptr_start = io_buf->io_buf;
	int header_size = 0;
	content[0] = 0;

	while((ptr_end = strchr(ptr_start, '\n')) != NULL) {
		*ptr_end = 0;
		sprintf(content, "%s<p>%s", content, ptr_start);
		ptr_start = ptr_end + 1;
	}
	header_size = strlen(content);

	print_header_200(buf);
	sprintf(buf, "%sContent-type: text/html\r\n", buf);
	sprintf(buf, "%sContent-length: %d\r\n\r\n", buf, header_size);

	io_writen(connfd, buf, strlen(buf));
	io_writen(connfd, content, strlen(content));
}

int check_req(char * req_line, req_line_t * req_line_ptr) {
	char * method_p 	= (char *)&req_line_ptr->method;
	char * uri_p 		= (char *)&req_line_ptr->uri;
	char * version_p 	= (char *)&req_line_ptr->version;
	/*
	 * get http request line, split method, URI and version
	 * GET / HTTP/1.1
	 */
	sscanf(req_line, "%s %s %s", method_p, uri_p, version_p);

	/* only support method GET/POST */
	if (!strcasecmp(method_p, "POST")) {
		/* method is POST */
	}
	else if (strcasecmp(method_p, "GET")) {
		/* neither POST nor GET */
		return -1;
	}

	return 0;
}

void print_req(req_line_t * req_ptr) {
	printf("%s %s %s\n", req_ptr->method, req_ptr->uri, req_ptr->version);
}


/* http request handler */
void process_req(int connfd) {
	int req_type;
	io_t io_buf;
	req_line_t req_line;
	char buf[BUF_LEN];
	char filename[BUF_LEN], cgi_args[BUF_LEN];
	struct stat file_stat;

	io_initbuf(&io_buf, connfd);

	/* read header into buf */
	io_readlineb(&io_buf, buf, BUF_LEN);

	/* print request header */
	//read_req_header(&io_buf);

	if (check_req((char *)&buf, &req_line)) {
		error_to_client(connfd, (char *)&(req_line.method), "501", "Not Implemented",
				"Server does not implement this method");
		return;
	}

	/* parse URI to get filename and args */
	req_type = parse_uri((char *)&req_line.uri, &io_buf, filename, cgi_args);

	/* serve for request*/
	switch(req_type) {
	case 0:		/* dynamic request */
		/* check if file exists */
		if (stat(filename, &file_stat)) {
			error_to_client(connfd, filename, "404", "Not Found",
					"File not found");
			return;
		}
		if (!(S_ISREG(file_stat.st_mode)) || !(S_IXUSR & file_stat.st_mode)) {
			/* request for not regular file or file can't be executed */
			error_to_client(connfd, filename, "403", "Forbidden",
					"CGI program can't run");
			return;
		}
		serve_dynamic(connfd, filename, cgi_args);
		break;
	case 1:		/* static request */
		/* check if file exists */
		if (stat(filename, &file_stat)) {
			error_to_client(connfd, filename, "404", "Not Found",
					"File not found");
			return;
		}
		if (!(S_ISREG(file_stat.st_mode)) || !(S_IRUSR & file_stat.st_mode)) {
			/* request for not regular file or file can't be read */
			error_to_client(connfd, filename, "403", "Forbidden",
					"File can't read");
			return;
		}
		serve_static(connfd, filename, file_stat.st_size);
		break;
	case 2:		/* header */
		serve_header(connfd, &io_buf);
		break;
	}
}
