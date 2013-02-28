/*
 * handler.h
 *
 *  Created on: Feb 26, 2013
 *      Author: leonshao
 */

#ifndef HANDLER_H_
#define HANDLER_H_

void serve_static(int connfd, char *filename, int filesize);
void serve_dynamic(int connfd, char *filename, char *cgi_args);
void error_to_client(int connfd, char *cause, char * errnum, char *shortmsg, char *longmsg);


#endif /* HANDLER_H_ */
