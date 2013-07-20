/*
 * tiny-multiprocess.c
 *
 *  Created on: 2013-7-13
 *      Author: leon
 */
#include "socket/socketutil.h"
#include "handler/handler.h"
#include <signal.h>			// signal(), SIG_ERR
#include <unistd.h>			// getpid()
#include <sys/types.h>		// pid_t

int main(int argc, char** argv) {
	int listenfd, port, connfd, clientaddrlen;
	struct sockaddr_in clientaddr;
	pid_t pid;

	if (argc != 2) {
		printf("Usage: %s port\n", argv[0]);
		return 1;
	}

	// install signal handler
	signal(SIGCHLD, sigchld_handler);

	port = atoi(argv[1]);
	// start server
	listenfd = open_listenfd(port);
	clientaddrlen = sizeof(clientaddr);
	while(1){
		connfd = accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientaddrlen);
		if (fork() == 0) {
			// in child process
			pid = getpid();
			printf("child process %d handling the request\n", (int)pid);

			close(listenfd);
			process_req(connfd);
			close(connfd);
			exit(0);
		}
		close(connfd);
	}
}
