/*
 * mytiny.c
 *
 *  Created on: Nov 28, 2012
 *      Author: leonshao
 */

#include "socketutil.h"
#include "io/ioutil.h"
#include <errno.h>			// errno, EAGAIN
#include <netdb.h>			// gethostbyaddr

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

void echo(int connfd) {
	size_t n;
	char buf[BUF_LEN];
	io_t io_buf;

	io_readinitb(&io_buf, connfd);
	while((n = io_readlineb(&io_buf, buf, BUF_LEN)) != 0){
		printf("server receive %d bytes data: %s", n, buf);

		io_writen(connfd, buf, n);
	}
}


int main(int argc, char** argv) {
	int listenfd, port, connfd, clientaddrlen;
	struct sockaddr_in clientaddr;
	struct hostent * hostp;

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
		hostp = gethostbyaddr((const char *) &clientaddr.sin_addr.s_addr,
				sizeof(clientaddr.sin_addr.s_addr), AF_INET);
		if (hostp != NULL ) {
			printf("Get connection from %s (%s:%d)\n", hostp->h_name,
					inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
			echo(connfd);
		}

		// do not mistake to listenfd!!!
		// printf("closing fd: %d\n", connfd);
		close(connfd);
	}

}
