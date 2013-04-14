/*
 * hello_n.c
 *
 *  Created on: 2013-4-14
 *      Author: leon
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *thread(void *vargp);

int main(int argc, char **argv) {
	int n = 0, i;
	pthread_t * tidp;

	if (argc != 2) {
		printf("Usage: %s thread_num\n", argv[0]);
		return EXIT_FAILURE;
	}

	n = atoi(argv[1]);
	tidp = malloc(n * sizeof(pthread_t));

	for (i=0; i<n; i++) {
		pthread_create(&tidp[i], NULL, thread, NULL);
		// printf("create thread: %ld\n", (unsigned long int)tid);
	}

	for (i=0; i<n; i++) {
		pthread_join(tidp[i], NULL);
		printf("tid: %ld finished\n", tidp[i]);
	}

	return 0;
}

void *thread(void *vargp) {
	printf("Hello world.\tby tid: %ld\n", pthread_self());
	return NULL;
}
