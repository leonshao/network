CC = gcc
CFLAGS = -O2 -Wall -I .

INCLUDE = -I../src
VPATH = io

SOURCES = socketutil.c \
          ioutil.c
OBJECTS = $(SOURCES:.c=.o)

all: mytiny client

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c $(filter %.c, $^)
	
mytiny: mytiny.c $(OBJECTS)
	$(CC) $(CFLAGS) -o mytiny $^

client: client.c $(OBJECTS)
	$(CC) $(CFLAGS) -o client $^


clean:
	rm -f *.o mytiny client *~

