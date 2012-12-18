/*
 * client.c
 *
 *  Created on: Dec 5, 2012
 *      Author: leonshao
 */

#include "socketutil.h"


void send_and_recv(int fd) {
	char buf[BUF_LEN];
	printf("please input data to send:\n");

	// fgets is terminated until EOF(Ctrl-D)
	while(fgets((char *)&buf, sizeof(buf), stdin) != NULL) {
		// send input data to server via fd
		if (write(fd, (char *)&buf, strlen((char *)&buf) + 1) > 0) {
			read(fd, (char *)&buf, sizeof(buf));
			printf("client receive data: %s\n", (char *)&buf);

		}
		else {
			perror("write fail: ");
			return;
		}
	}
}


int main(int argc, char** argv) {
	int clientfd, port;
	char *hostname;

	if (argc != 3) {
		printf("Usage: %s hostname/ip port\n", argv[0]);
		return EXIT_FAILURE;
	}

	hostname = argv[1];
	port = atoi(argv[2]);
	if ((clientfd = open_clientfd(hostname, port)) < 0) {
		return EXIT_FAILURE;
	}

	send_and_recv(clientfd);

	close(clientfd);

	return EXIT_SUCCESS;
}
