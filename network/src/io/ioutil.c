/*
 * ioutil.c
 *
 *  Created on: Dec 18, 2012
 *      Author: leonshao
 */

#include "ioutil.h"
#include <errno.h>		// errno, EINTR
#include <string.h>		// memcpy
#include <unistd.h>		// read, write

/* read n bytes (unbuffered) */
ssize_t io_readn(int fd, void *usrbuf, size_t n) {
	size_t nleft = n;
	ssize_t nread = n;
	char *bufp = usrbuf;		/* copy pointer */

	while (nleft > 0) {
		if ((nread = read(fd, bufp, nleft)) < 0) {
			if (errno == EINTR)	/* interrupted by sig handler return */
				nread = 0;		/* reset and call read() again */
			else
				return -1;		/* read() error */
		}
		else if (nread == 0)	/* read() EOF */
			break;

		nleft -= nread;
		bufp += nread;			/* move pointer forward */
	}

	return (n - nread);			/* return >= 0 */
}


/* write n bytes (unbuffered) */
ssize_t io_writen(int fd, void *usrbuf, size_t n){
	size_t nleft = n;
	ssize_t nwritten;
	char *bufp = usrbuf;

	while (nleft > 0) {
		/* write all data in buffer, even if write() return 0 */
		if ((nwritten = write(fd, bufp, nleft)) <= 0) {
			if (errno == EINTR)	/* interrupted by sig handler return */
				nwritten = 0;	/* reset and call write() again */
			else
				return -1; 		/* write() error */
		}
		nleft -= nwritten;
		bufp += nwritten;		/* move pointer forward */
	}

	return n;
}


/* init read buffer struct */
void io_readinitb(io_t *iop, int fd) {
	iop->io_fd		= fd;
	iop->io_cnt 	= 0;
	iop->io_bufptr	= iop->io_buf;
}


/* read n/io_cnt bytes from buffer in *rp to usrbuf */
static ssize_t io_read(io_t *rp, char *usrbuf, size_t n) {
	int cnt;

	/* refill if buf is empty */
	while (rp->io_cnt <= 0) {
		rp->io_cnt = read(rp->io_fd, rp->io_buf, sizeof(rp->io_buf));

		if (rp->io_cnt < 0) {
			if (errno != EINTR)		/* not interrupted error, return -1 */
				return -1;
		}
		else if (rp->io_cnt == 0)	/* read() EOF */
			return 0;
		else
			rp->io_bufptr = rp->io_buf;
	}

	/* copy min(rp->io_cnt, n) bytes */
	// cnt = rp->io_cnt < n ? rp->io_cnt : n
	cnt = n;
	if (rp->io_cnt < n)
		cnt = rp->io_cnt;

	memcpy(usrbuf, rp->io_bufptr, cnt);
	rp->io_bufptr += cnt;
	rp->io_cnt -= cnt;

	return cnt;
}


/* read a text line from buffer in *rp */
ssize_t io_readlineb(io_t *rp, void *usrbuf, size_t maxlen) {
	int n, rc;
	char c, *bufp = usrbuf;

	for (n = 1; n < maxlen; n++) {
		if ((rc = io_read(rp, &c, 1)) == 1) {
			*bufp++ = c;

			if (c == '\n') {
				break;
			}
		}
		else if (rc == 0) {
			if (n == 1)		/* EOF, no data read */
				return 0;
			else			/* EOF, some data was read */
				break;
		}
		else
			return -1;		/* error */
	}

	/* set 0 to end the line */
	*bufp = 0;

	/* return char number of the line */
	return n;
}


/* read n bytes from buffer in *rp */
ssize_t io_readnb(io_t *rp, void *usrbuf, size_t n){
	size_t nleft = n;
	ssize_t nread;
	char *bufp = usrbuf;

	while (nleft > 0) {
		if ((nread = io_read(rp, bufp, nleft)) < 0) {
			if (errno == EINTR)		/* not interrupted error, return -1 */
				nread = 0;			/* reset and call io_read() again */
			else
				return -1;			/* io_read() error*/
		}
		else if (nread == 0)		/* EOF */
			break;

		nleft -= nread;
		bufp  += nread;
	}

	return (n - nleft);
}

