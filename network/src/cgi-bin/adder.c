/*
 * adder.c
 *
 *  Created on: Feb 26, 2013
 *      Author: leonshao
 */
#include <stdlib.h>		// getenv()
#include <string.h>		// strchr()
#include <stdio.h>		// sprintf(), fflush()

#define BUF_LEN 1024
#define CONTENT_BUFSIZE 8192

int main(void) {
	char *buf, *p;
	char arg1[BUF_LEN], arg2[BUF_LEN], content[CONTENT_BUFSIZE];
	int n1=0, n2=0;

	/* get arguments from environment QUERY_STRING set by the server */
	if ((buf = getenv("QUERY_STRING")) != NULL) {
		/* args format : 1500&211 */
		p = strchr(buf, '&');
		*p = '\0';	// overwrite '&' with '\0', split to two strings

		strcpy(arg1, buf);
		strcpy(arg2, p+1);
		n1 = atoi(arg1);
		n2 = atoi(arg2);
	}

	sprintf(content, "<p>The answer is : %d + %d = %d\r\n", n1, n2, n1 + n2);

	printf("Content-type: text/html\r\n");
	printf("Content-length: %d\r\n\r\n", (int)strlen(content));
	printf("%s", content);
	fflush(stdout);

	exit(0);
}
