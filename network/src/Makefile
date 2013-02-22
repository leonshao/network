CC = gcc
CFLAGS = -O2 -Wall -I .

VPATH = io

SOURCES = socketutil.c \
          ioutil.c
OBJECTS = $(SOURCES:.c=.o)

OUTPUT = mytiny \
         client

all: $(OUTPUT)

$(OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) -c $<
#.SUFFIXES: .c .o
#.c.o:
#	$(CC) $(CFLAGS) -c $(filter %.c, $^)

$(OUTPUT): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $@.c $^

clean:
	rm -f *.o mytiny client *~

