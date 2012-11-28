/*
 * socketutil.c
 *
 *  Created on: Nov 28, 2012
 *      Author: leonshao
 */

#include <string.h>			// bzero()
#include "socketutil.h"

int open_listenfd(int port) {
	int listenfd, optval=1;
	struct sockaddr_in serveraddr;

	// 1. create socket fd
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		// <bits/socket.h>: AF_INET, SOCK_STREAM
		printf("socket fail!\n");
		return -1;
	}

	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
			(const void *)&optval, sizeof(int)) < 0) {
		printf("setsockopt fail!\n");
		return -1;
	}

	// set server address
	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((uint16_t)port);

	// 2. bind socket to server address
	if (bind(listenfd, (SA *)&serveraddr, sizeof(serveraddr)) < 0) {
		printf("bind fail!\n");
		return -2;
	}

	// 3. listen to the socket
	if (listen(listenfd, 512) < 0) {
		printf("listen fail!\n");
		return -3;
	}

	return listenfd;
}
