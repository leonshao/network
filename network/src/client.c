/*
 * client.c
 *
 *  Created on: Dec 5, 2012
 *      Author: leonshao
 */

#include "socketutil.h"

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

	return EXIT_SUCCESS;
}
