/*
 * tiny-multithread.c
 *
 *  Created on: 2013-7-20
 *      Author: leon
 */

#include "socket/socketutil.h"
#include "handler/handler.h"
#include <pthread.h>

void *thread_start(void *vargp) {
	int connfd = *((int *)vargp);
	free(vargp);
	pthread_t mytid = pthread_self();
	pthread_detach(mytid);
	printf("Thread %ld is handling the request\n", mytid);

	// sleep(3);
	process_req(connfd);
	close(connfd);

	return NULL;
}

int main(int argc, char** argv) {
	int listenfd, port, *connfdp, clientaddrlen;
	struct sockaddr_in clientaddr;
	pthread_t tid;	// thread id

	if (argc != 2) {
		printf("Usage: %s port\n", argv[0]);
		return 1;
	}

	port = atoi(argv[1]);
	// start server
	listenfd = open_listenfd(port);
	clientaddrlen = sizeof(clientaddr);

	while (1) {
		connfdp = malloc(sizeof(int));
		*connfdp = accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientaddrlen);
		pthread_create(&tid, NULL, thread_start, (void *)connfdp);
		// printf("Thread:%ld is created\n", tid);
	}
}
