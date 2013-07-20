/*
 * tiny-select.c
 *
 *  Created on: 2013-7-13
 *      Author: leon
 */

#include "socket/socketutil.h"
#include "handler/handler.h"

typedef struct {
	fd_set read_set;
	fd_set ready_set;
	int nready;
	int max_index;
	int clientfd[FD_SETSIZE];
} pool;

void init_client_fd_set(int listenfd, pool * pool) {
	int i;
	pool->nready = 0;
	pool->max_index = 0;
	for (i=0; i<FD_SETSIZE; i++)
		pool->clientfd[i] = -1;

	FD_ZERO(&pool->read_set);
	FD_SET(listenfd, &pool->read_set);
}

void add_client(int fd, pool * pool) {
	int i;
	pool->nready--;	// event for listen fd, need to adjust the ready fd nums
	for (i=0; i<FD_SETSIZE; i++)
		if (pool->clientfd[i] < 0) {
			pool->clientfd[i] = fd;
			FD_SET(fd, &pool->read_set);
			if (i > pool->max_index)
				pool->max_index = i;
			break;
		}
}

void process_clients(pool * pool) {
	int i, connfd;
	// pay attention to the max_index boundary
	// the max_index should be handled so needs the i=max_index one
	for (i=0; (i <= pool->max_index) && (pool->nready > 0); i++) {
		connfd = pool->clientfd[i];
		if FD_ISSET(connfd, &pool->ready_set) {
			pool->nready--;
			process_req(connfd);

			// once complete, clear the resource
			close(connfd);
			pool->clientfd[i] = -1;
			FD_CLR(connfd, &pool->read_set);
			sleep(5);
		}
	}
}

void print_pool(pool * pool) {
	int i;
	printf("nready: %d\n", pool->nready);
	printf("max_index: %d\n", pool->max_index);
	printf("clientfd:\n");
	for (i=0; i<=pool->max_index; i++) {
		printf("%d\t", pool->clientfd[i]);
	}
	printf("\n");
}

int main(int argc, char** argv) {
	int listenfd, port, connfd, clientaddrlen;
	struct sockaddr_in clientaddr;
	pool pool;

	if (argc != 2) {
		printf("Usage: %s port\n", argv[0]);
		return 1;
	}

	port = atoi(argv[1]);
	// start server
	listenfd = open_listenfd(port);
	clientaddrlen = sizeof(clientaddr);

	init_client_fd_set(listenfd, &pool);
	while (1) {
		// init set every loop to check the listen fd and all active client fds
		pool.ready_set = pool.read_set;
		pool.nready = select(FD_SETSIZE, &pool.ready_set, NULL, NULL, NULL);

		if FD_ISSET(listenfd, &pool.ready_set) {
			connfd = accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientaddrlen);
			// add client fd to client fd array and read_set
			add_client(connfd, &pool);
		}

		// process active fd
		process_clients(&pool);
		print_pool(&pool);
	}
}
