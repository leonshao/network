/*
 * socketutil.h
 *
 *  Created on: Nov 28, 2012
 *      Author: leonshao
 */

#ifndef SOCKETUTIL_H_
#define SOCKETUTIL_H_

#include <sys/socket.h>		// socket(), bind(), listen(), accept(), setsockopt()
#include <netinet/in.h>		// sockaddr_in, htonl(), ntohs(), INADDR_ANY
#include <stdio.h>			// printf(), snprintf()
#include <stdlib.h>			// atoi
#include <arpa/inet.h>		// inet_aton(), inet_ntoa()
#include <unistd.h>			// socklen_t, close(), read()
#include <string.h>			// strlen()
#include <fcntl.h>			// fcntl(), F_GETFL, F_SETFL, O_NONBLOCK



#define BUF_LEN 1024

typedef struct sockaddr SA;

int open_listenfd(int port);
int open_clientfd(char *hostname, int port);

#endif /* SOCKETUTIL_H_ */
