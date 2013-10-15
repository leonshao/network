/*
 * proxy.c
 *
 *  Created on: 2013-9-5
 *      Author: leon
 */

#include "socket/socketutil.h"
#include "handler/handler.h"
#include <sys/stat.h>
#include <pthread.h>

#define HOSTINFO_SIZE		32
#define BLOCK_URL_SIZE		64

typedef struct {
	char hostname[LINE_LEN];
	int port;
} hostinfo_t;

hostinfo_t hostinfo[HOSTINFO_SIZE];
int hostinfo_num;
int hostinfo_index;

char block_url_info[BLOCK_URL_SIZE][LINE_LEN];
int block_url_num;

int verify_req(int connfd, char * req_line_p) {
	io_t io_buf;			// to store all the req content
	char header[BUF_LEN];	// to get the req line
	req_line_t req_line;	// parsed method, uri and version
	char *block_url_info_p;
	int i;

	/* read header into buf */
	io_initbuf(&io_buf, connfd);
	io_readlineb(&io_buf, header, BUF_LEN);

	// logging
	logging(&io_buf);

	// method not support
	if (check_req((char *)&header, &req_line)) {
		error_to_client(connfd, (char *)&req_line.method, "501", "Not Implemented",
				"Server does not implement this method");
		return -1;
	}

	// block the url or not
	for (i=0; i<block_url_num; i++) {
		block_url_info_p = (char *)&block_url_info[i];
		if (strstr(block_url_info_p, req_line.uri) && (strlen(block_url_info_p) == strlen(req_line.uri))) {
			printf("url:%s is blocked!\n", req_line.uri);
			// response for the blocking
			error_to_client(connfd, req_line.uri, "403", "Forbidden",
					"Url is blocked");
			return 1;
		}
	}


	strcpy(req_line_p, (char *)&header);

	return 0;
}

int parse_config(char *config, hostinfo_t *hostinfo) {
	char buf[BUF_LEN];
	char *p, *hostname_p;

	strcpy(buf, config);

	// skip space or tab at start position
	p = (char *)&buf;
	while (*p == ' ' || *p == '\t') {
		*p++ = '\0';
	}
	hostname_p = p;

	// skip space or tab between hostname and port
	p = strchr(buf, ' ');
	if (p == NULL) {
		p = strchr(buf, '\t');
	}

	if (p) {
		*p++ = '\0';
		while (*p == ' ' || *p == '\t') {
			*p++ = '\0';
		}

		strcpy(hostinfo->hostname, hostname_p);
		hostinfo->port = atoi(p);
		return 0;
	}

	printf("configuration error!\n");
	return 1;
}

void print_hostinfo(hostinfo_t *hostinfo) {
	printf("%s\t%d\n", hostinfo->hostname, hostinfo->port);
}

void *thread_start(void *vargp) {
	int connfd = *((int *)vargp);
	int clientfd, n;
	char buf[8192];
	char header[BUF_LEN];	// to get the req line
	hostinfo_t *hostinfo_p;
	int result;

	free(vargp);

	// 1. check HTTP req, redirect or block
	result = verify_req(connfd, (char *)&header);
	if (result) {
		close(connfd);
		return NULL;
	}

	// 2. redirect req to server and send back response
	hostinfo_p = &hostinfo[hostinfo_index++];
	if (hostinfo_index == hostinfo_num)
		hostinfo_index = 0;

	bzero((char *)&buf, 8192);
	clientfd = open_clientfd(hostinfo_p->hostname, hostinfo_p->port);
	n = write(clientfd, header, strlen(header));
	while ((n = read(clientfd, &buf, 8192)) > 0) {
		io_writen(connfd, buf, strlen(buf));
	}

	close(clientfd);
	close(connfd);
	return NULL;
}

/* get server info from config file */
int init_config() {
	FILE * config_file;
	char line[LINE_LEN];
	int hostinfo_num = 0;
	char * filename = "config.txt";
	struct stat file_stat;

	if (stat(filename, &file_stat)) {
		// url block file not exists
		return 0;
	}
	config_file = fopen(filename, "r+");
	// printf("orig pos: %ld\n", ftell(config_file));

	while (fgets(line, LINE_LEN, config_file)) {
		parse_config(line, &hostinfo[hostinfo_num++]);
	}
	hostinfo_index = 0;

	return hostinfo_num;
}

int init_block_urls() {
	int block_url_num = 0;
	char line[LINE_LEN];
	FILE * block_urls_file;
	char * chr_p;
	char * filename = "block_url.txt";
	struct stat file_stat;

	if (stat(filename, &file_stat)) {
		// url block file not exists
		return 0;
	}
	block_urls_file = fopen(filename, "r+");
	while (fgets(line, LINE_LEN, block_urls_file)) {
		// remove the \n in the blocking url line
		chr_p = strchr(line, '\n');
		if (chr_p)
			*chr_p = '\0';
		strcpy((char *)&block_url_info[block_url_num++], line);
	}
	return block_url_num;
}

int main(int argc, char** argv) {
	int listenfd, port, *connfdp, clientaddrlen;
	struct sockaddr_in clientaddr;
	pthread_t tid;

	if (argc != 2) {
		printf("Usage: %s port\n", argv[0]);
		return 1;
	}

	port = atoi(argv[1]);
	// start server
	listenfd = open_listenfd(port);
	clientaddrlen = sizeof(clientaddr);

	// init config and blocking urls
	hostinfo_num = init_config();
	if (hostinfo_num == 0) {
		printf("no host info file!\n");
		return 1;
	}
	block_url_num = init_block_urls();

	while (1) {
		connfdp = malloc(sizeof(int));
		*connfdp = accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientaddrlen);
		pthread_create(&tid, NULL, thread_start, (void *)connfdp);
	}
	return 0;
}
