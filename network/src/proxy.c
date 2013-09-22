/*
 * proxy.c
 *
 *  Created on: 2013-9-5
 *      Author: leon
 */

#include "socket/socketutil.h"
#include "handler/handler.h"

typedef struct {
	char hostname[LINE_LEN];
	int port;
} hostinfo_t;

void verify_req(int connfd, char * req_line_p) {
	int filefd;
	req_line_t req_line;	// parsed method, uri and version
	// char * uri_p = (char *)&req_line.uri;

	// method not support
	if (check_req(req_line_p, &req_line)) {
		error_to_client(connfd, (char *)&req_line.method, "501", "Not Implemented",
				"Server does not implement this method");
		return;
	}

	// uri is in block file
	filefd = open("log.txt", O_RDWR, S_IRUSR);
}

int parse_config(char *config, hostinfo_t *hostinfo) {
	char buf[BUF_LEN];
	char *p, *p_next;

	strcpy(buf, config);
	p = strchr(buf, ' ');
	if (p == NULL) {
		p = strchr(buf, '\t');
	}

	if (p) {
		*p++ = '\0';
		while (*p == ' ' || *p == '\t') {
			*p++ = '\0';
		}

		strcpy(hostinfo->hostname, buf);
		hostinfo->port = atoi(p);
		return 0;
	}

	printf("configuration error!\n");
	return 1;
}

void print_hostinfo(hostinfo_t *hostinfo) {
	printf("%s\t%d\n", hostinfo->hostname, hostinfo->port);
}

void proxy_handler(int connfd) {
	int clientfd, n, filefd;
	io_t io_buf;			// to store all the req content
	char line_buf[BUF_LEN];
	char buf[8192];
	char header[BUF_LEN];	// to get the req line
	io_t io_file_buf;
	hostinfo_t hostinfo;

	// 1. check HTTP req, redirect or block
	io_initbuf(&io_buf, connfd);
	/* read header into buf */
	io_readlineb(&io_buf, header, BUF_LEN);

	// 2. redirect req to server
	// get server info from config file
	filefd = open("config.txt", O_RDWR, S_IRUSR);
	io_initbuf(&io_file_buf, filefd);
	io_readlineb(&io_file_buf, line_buf, BUF_LEN);
	parse_config(line_buf, &hostinfo);
	close(filefd);
	// print_hostinfo(&hostinfo);

	clientfd = open_clientfd(hostinfo.hostname, hostinfo.port);
	n = write(clientfd, header, strlen(header));
	while ((n = read(clientfd, &buf, 8192)) > 0) {
		printf("%s\n", buf);
		io_writen(connfd, buf, strlen(buf));
	}
	close(clientfd);
	close(connfd);

	// 3. logging
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

	while (1) {
		connfd = accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientaddrlen);

		proxy_handler(connfd);
	}
}
