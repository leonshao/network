/*
 * handler.h
 *
 *  Created on: Feb 26, 2013
 *      Author: leonshao
 */

#ifndef HANDLER_H_
#define HANDLER_H_

#include "io/ioutil.h"

#define LINE_LEN 1024

typedef struct {
	char uri[LINE_LEN];
	char method[10];
	char version[10];
} req_line_t;
int check_req(char * req_line, req_line_t * req_line_ptr);

void process_req(int connfd);
void error_to_client(int connfd, char *cause, char * errnum, char *shortmsg, char *longmsg);
void read_req_header(io_t *io_buf);
void sigchld_handler(int sig);
void sigpipe_handler(int sig);

#endif /* HANDLER_H_ */
