/*
 * ioutil.h
 *
 *  Created on: Dec 18, 2012
 *      Author: leonshao
 */
#ifndef IOUTIL_H_
#define IOUTIL_H_

#include <stdio.h>		// ssize_t

#define IO_BUFSIZE 8192
typedef struct {
    int io_fd;               /* descriptor for this internal buf */
    int io_cnt;              /* unread bytes in internal buf */
    char *io_bufptr;         /* next unread byte in internal buf */
    char io_buf[IO_BUFSIZE]; /* internal buffer */
} io_t;

/* unbuffered read, write */
ssize_t io_readn(int fd, void *usrbuf, size_t n);
ssize_t io_writen(int fd, void *usrbuf, size_t n);

/* buffered read, write */
void io_initbuf(io_t *iop, int fd);
ssize_t io_readlineb(io_t *rp, void *usrbuf, size_t maxlen);
ssize_t io_readnb(io_t *rp, void *usrbuf, size_t n);
void io_print_buf(io_t *rp);

#endif /* IOUTIL_H_ */
