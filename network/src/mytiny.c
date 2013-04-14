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
#include <stdlib.h>			// setenv()
#include <signal.h>			// signal(), SIG_ERR

void echo(int connfd) {
	size_t n;
	char buf[BUF_LEN];
	io_t io_buf;

	io_initbuf(&io_buf, connfd);
	while((n = io_readlineb(&io_buf, buf, BUF_LEN)) != 0){
		printf("server receive %d bytes data: %s", (int)n, buf);

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

	// install signal handler
	signal(SIGCHLD, sigchld_handler);
	signal(SIGPIPE, sigpipe_handler);

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
