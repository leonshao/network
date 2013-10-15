CC = gcc
CFLAGS = -O2 -Wall -I. -I..

LIB = -lpthread

VPATH = ../socket \
		../io \
        ../handler

SOURCES = socketutil.c \
          ioutil.c \
          handler.c
OBJECTS = $(SOURCES:.c=.o)

OUTPUT = mytiny \
         client \
         tiny-multiprocess \
         tiny-select \
         tiny-multithread \
         tiny-prethreads

all: $(OUTPUT)

$(OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) -c $<
#.SUFFIXES: .c .o
#.c.o:
#	$(CC) $(CFLAGS) -c $(filter %.c, $^)

$(OUTPUT): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $@.c $^ $(LIB)

clean:
	rm -f *.o mytiny client tiny-multiprocess tiny-select tiny-multithread tiny-prethreads test *~
