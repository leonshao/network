/*
 * test.c
 *
 *  Created on: 2013-7-13
 *      Author: leon
 */
#include "socket/socketutil.h"
#include <errno.h>		// errno

void get_result(int clientfd) {
	int n;
	char buf[8192];

	while ((n = read(clientfd, &buf, 8192)) > 0)
		printf("%s\n", buf);
}

int main(int argc, char** argv) {
	int clientfd, port, req_num, req, n;
	char *hostname;
	// if no ended with \r\n, no response from server
	char * header = "GET / HTTP/1.1\r\n";

	if (argc != 4) {
		printf("Usage: %s hostname/ip port req-num\n", argv[0]);
		return EXIT_FAILURE;
	}

	hostname 	= argv[1];
	port 		= atoi(argv[2]);
	req_num 	= atoi(argv[3]);

	for(req=0; req<req_num; req++){
		clientfd = open_clientfd(hostname, port);
		n = write(clientfd, header, strlen(header));
		printf("send %d bytes\n", n);
		get_result(clientfd);
	}

	return EXIT_SUCCESS;
}
