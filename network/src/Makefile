CC = gcc
CFLAGS = -O2 -Wall -I.

LIB = -lpthread

VPATH = socket \
		io \
        handler \
        cgi-bin

SOURCES = socketutil.c \
          ioutil.c \
          handler.c
OBJECTS = $(SOURCES:.c=.o)

OUTPUT = proxy \
         test

all: $(OUTPUT) cgi

$(OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) -c $<
#.SUFFIXES: .c .o
#.c.o:
#	$(CC) $(CFLAGS) -c $(filter %.c, $^)

$(OUTPUT): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $@.c $^ $(LIB)

cgi:
	cd cgi-bin; make

clean: cleancgi
	rm -f *.o proxy test *~

cleancgi:
	cd cgi-bin; make clean
