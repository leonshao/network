/*
 * tiny-prethreads.c
 *
 *  Created on: 2013-7-20
 *      Author: leon
 */

#include "socket/socketutil.h"
#include "handler/handler.h"
#include <pthread.h>
#include <semaphore.h>

#define THREAD_NUM_ORIG 5
#define THREAD_SIZE		1024
#define SBUF_SIZE		16

pthread_t thread_array[THREAD_SIZE];

/*	producer-consumer utilities	*/
typedef struct {
	int * buf;		// buffer for connfds
	int n;			// slot size
	int front;		// first item index
	int rear;		// last item index
	sem_t mutex;	// mutex access to buffer
	sem_t slots;	// available slots to producer
	sem_t items;	// available items to consumer
} sbuf_t;

sbuf_t sbuf;	// shared buffer

void P(sem_t * s){
	sem_wait(s);
}

void V(sem_t * s){
	sem_post(s);
}

void sbuf_print(sbuf_t * sp){
	int sem_value;

	printf("sp->buf: %p\n", sp->buf);
	printf("sp->n: %d\n", sp->n);
	printf("sp->front: %d\n", sp->front);
	printf("sp->rear: %d\n", sp->rear);

	sem_getvalue(&sp->mutex, &sem_value);
	printf("sp->mutex: %d\n", sem_value);
	sem_getvalue(&sp->slots, &sem_value);
	printf("sp->slots: %d\n", sem_value);
	sem_getvalue(&sp->items, &sem_value);
	printf("sp->items: %d\n", sem_value);
}

void sbuf_init(sbuf_t * sp, int n){
	sp->buf 	= calloc(n, sizeof(int));
	sp->n		= n;
	sp->front	= 0;
	sp->rear	= 0;
	sem_init(&sp->mutex, 0, 1);		// binary value for lock/unlock
	sem_init(&sp->slots, 0, n);		// empty slots
	sem_init(&sp->items, 0, 0);		// zero items
}

void sbuf_deinit(sbuf_t * sp){
	free(sp->buf);
}

void sbuf_insert(sbuf_t * sp, int item){
	P(&sp->slots);
	P(&sp->mutex);

	sp->buf[(sp->rear++) % (sp->n)] = item;
	if (sp->rear > sp->n) {
		sp->rear = 0;
	}

	V(&sp->mutex);
	V(&sp->items);

}

int sbuf_remove(sbuf_t * sp){
	int item;
	P(&sp->items);
	P(&sp->mutex);

	item = sp->buf[(sp->front++) % (sp->n)];
	if (sp->front > sp->n) {
		sp->front = 0;
	}

	V(&sp->mutex);
	V(&sp->slots);
	return item;
}

void *thread_start(void * vargp){
	// consumer
	pthread_t mytid = pthread_self();
	pthread_detach(mytid);

	while (1) {
		int connfd;
		connfd = sbuf_remove(&sbuf);
		process_req(connfd);
		close(connfd);
	}
	return NULL;
}

int main(int argc, char** argv) {
	int listenfd, port, connfd, clientaddrlen, i, end;
	struct sockaddr_in clientaddr;
	pthread_t tid;	// thread id
	int sem_value;
	int old_thread_num, index;

	if (argc != 2) {
		printf("Usage: %s port\n", argv[0]);
		return 1;
	}

	port = atoi(argv[1]);
	// start server
	listenfd = open_listenfd(port);
	clientaddrlen = sizeof(clientaddr);

	// init buffer
	sbuf_init(&sbuf, SBUF_SIZE);
	sbuf_print(&sbuf);

	old_thread_num = index = 0;
	// pre-created threads
	for (i=0; i<THREAD_NUM_ORIG; i++) {
		pthread_create(&tid, NULL, thread_start, NULL);
		thread_array[index++] = tid;
		printf("Thread %ld created\n", tid);
	}


	while (1) {
		connfd = accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientaddrlen);

		// producer
		sbuf_insert(&sbuf, connfd);

		sem_getvalue(&sbuf.slots, &sem_value);
		if (sem_value == 0) {
			// buffer is full, double the threads
			old_thread_num = index;
			end = index;
			for (i=0; i<end; i++) {
				pthread_create(&tid, NULL, thread_start, NULL);
				thread_array[index++] = tid;
				// printf("Thread %ld created\n", tid);
			}
			printf("create, old num: %d, current num: %d\n", old_thread_num, index);
		}
		else if (sem_value == SBUF_SIZE && index > THREAD_NUM_ORIG) {
			old_thread_num = index;
			end = index/2;
			for (i=0; i<end; i++) {
				pthread_cancel(thread_array[--index]);
				thread_array[index] = 0;
			}
			printf("reduce, old num: %d, current num: %d\n", old_thread_num, index);
		}
	}

}
