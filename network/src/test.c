/*
 * test.c
 *
 *  Created on: 2013-7-13
 *      Author: leon
 */
#include "socket/socketutil.h"
#include <errno.h>		// errno
#include <pthread.h>

// if no ended with \r\n, no response from server
char * header = "GET / HTTP/1.1\r\n";
char *hostname;
int port;

void get_result(int clientfd) {
	int n;
	char buf[8192];

	while ((n = read(clientfd, &buf, 8192)) > 0)
		printf("%s\n", buf);
}

void *thread_start(void *vargp) {
	int clientfd, n;

	pthread_detach(pthread_self());
	clientfd = open_clientfd(hostname, port);
	n = write(clientfd, header, strlen(header));
	printf("send %d bytes\n", n);
	get_result(clientfd);
	close(clientfd);
	return NULL;
}

int main(int argc, char** argv) {
	int req, req_num, i;
	pthread_t *tidp;

	if (argc != 4) {
		printf("Usage: %s hostname/ip port req-num\n", argv[0]);
		return EXIT_FAILURE;
	}

	hostname 	= argv[1];
	port 		= atoi(argv[2]);
	req_num 	= atoi(argv[3]);

	printf("host:%s, port:%d\n", hostname, port);
	tidp = (pthread_t *)malloc(req_num * sizeof(pthread_t));
	for(req=0; req<req_num; req++) {
		pthread_create((pthread_t *)&tidp[req], NULL, thread_start, NULL);
		printf("Thread %ld is created\n", tidp[req]);
	}

	//for (i=0; i<req_num; i++) {
	//	pthread_join(tidp[i], NULL);
	//}
	while (1);

	return EXIT_SUCCESS;
}
