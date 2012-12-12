/*
 * client.c
 *
 *  Created on: Dec 5, 2012
 *      Author: leonshao
 */

#include "socketutil.h"

void send_req(int fd) {
	char buf[BUF_LEN];
	snprintf((char *)&buf, sizeof(buf), "Hi.");
	write(fd, (char *)&buf, strlen((char *)&buf) + 1);
}

void process_resp(int fd) {
	int recvbytes;
	char buf[BUF_LEN];

	read(fd, (char *)&buf, sizeof(buf));
	printf("client receive data: %s\n", (char *)&buf);

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

	send_req(clientfd);
	process_resp(clientfd);

	close(clientfd);

	return EXIT_SUCCESS;
}
