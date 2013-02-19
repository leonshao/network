/*
 * client.c
 *
 *  Created on: Dec 5, 2012
 *      Author: leonshao
 */

#include "socketutil.h"
#include "io/ioutil.h"


void send_and_recv(int fd) {
	io_t io_buf;
	char buf[BUF_LEN];	// store input and response data

	io_readinitb(&io_buf, fd);

	printf("please input data to send:\n");

	// fgets is terminated until EOF(Ctrl-D)
	while(fgets(buf, sizeof(buf), stdin) != NULL) {
		// send input data to server via fd
		if (io_writen(fd, buf, strlen(buf)) > 0) {
			// read a line from response data in io_buf
			io_readlineb(&io_buf, buf, BUF_LEN);
			fputs(buf, stdout);
			// printf("client receive data: %s\n", (char *)&buf);
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
