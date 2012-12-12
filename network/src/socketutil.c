/*
 * socketutil.c
 *
 *  Created on: Nov 28, 2012
 *      Author: leonshao
 */

#include <string.h>			// bzero(), bcopy()
#include <netdb.h>			// hostent, gethostbyaddr(), gethostbyname()
#include "socketutil.h"



int open_clientfd(char *hostname, int port) {
	int clientfd;
	struct hostent *hostp;
	struct sockaddr_in serveraddr;
	struct in_addr addr;

	// 1. create client socket fd
	if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("socket fail!\n");
		return -1;
	}

	// 2. get server address
	if (inet_aton(hostname, &addr) != 0) {
		hostp = gethostbyaddr((char *)&addr,sizeof(addr),AF_INET);
	}
	else {
		hostp = gethostbyname(hostname);
	}
	if (hostp == NULL) {
		printf("get host fail\n");
		return -2;
	}

	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	// copy server address from hostent to serveraddr
	bcopy((char *)hostp->h_addr_list[0],
			(char *)&serveraddr.sin_addr.s_addr,
			hostp->h_length);
	serveraddr.sin_port = htons((uint16_t)port);

	if (connect(clientfd, (SA *)&serveraddr, sizeof(serveraddr)) < 0) {
		printf("connect fail!\n");
		return -3;
	}

	return clientfd;
}


int open_listenfd(int port) {
	int listenfd, optval=1;
	struct sockaddr_in serveraddr;

	// 1. create server socket fd
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

	printf("server starts at %s:%d\n", inet_ntoa(serveraddr.sin_addr), port);

	return listenfd;
}
