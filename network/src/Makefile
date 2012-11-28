CC = gcc
CFLAGS = -O2 -Wall -I .

# This flag includes the Pthreads library on a Linux box.
# Others systems will probably require something different.

all: mytiny

mytiny: mytiny.c socketutil.o
	$(CC) $(CFLAGS) -o mytiny mytiny.c socketutil.o

socketutil.o:
	$(CC) $(CFLAGS) -c socketutil.c


clean:
	rm -f *.o mytiny *~

