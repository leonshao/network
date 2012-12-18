/*
 * mytiny.c
 *
 *  Created on: Nov 28, 2012
 *      Author: leonshao
 */

#include "socketutil.h"
#include <errno.h>			// errno, EAGAIN


void process_req(int fd) {
	int readbytes, flags;
	char buf[BUF_LEN];

	bzero((char *)&buf, sizeof(buf));

	// set fd to nonblock, otherwise it blocks on read()
	flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0) {
		flags = 0;
	}
	fcntl(fd, F_SETFL, flags|O_NONBLOCK);

	// encouter EAGAIN, ignore it and read until resourse available
	while((readbytes = read(fd, (char *)&buf, sizeof(buf))) == -1
			&& errno == EAGAIN);
	printf("server receive data: %s\t readbytes: %d\n", (char *)&buf, readbytes);

	write(fd, (char *)&buf, strlen((char *)&buf) + 1);

}

int main(int argc, char** argv) {
	int listenfd, port, connfd, clientaddrlen;
	struct sockaddr_in clientaddr;

	if (argc != 2) {
		printf("Usage: %s port\n", argv[0]);
		return EXIT_FAILURE;
	}

	if((port = atoi(argv[1])) < 0) {
		printf("port should be greater than 0!\n");
		return EXIT_FAILURE;
	}

	// start server
	if ((listenfd = open_listenfd(port)) < 0){
		return EXIT_FAILURE;
	}

	clientaddrlen = sizeof(clientaddr);
	while(1){
		connfd = accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientaddrlen);
		if (connfd > 0) {
			// process request
			printf("Get connection from %s:%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
			process_req(connfd);
		}

		// do not mistake to listenfd!!!
		printf("closing fd: %d\n", connfd);
		close(connfd);
	}

}
