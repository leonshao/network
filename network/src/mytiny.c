/*
 * mytiny.c
 *
 *  Created on: Nov 28, 2012
 *      Author: leonshao
 */

#include "socketutil.h"
#include <unistd.h>		// socklen_t, close()

int main(int argc, char** argv) {
	int listenfd, port, clientfd, clientaddrlen;
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
		clientfd = accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientaddrlen);
		if (clientfd > 0) {
			// process request
			printf("Get connection from %s:%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
		}
		close(listenfd);
	}

}
