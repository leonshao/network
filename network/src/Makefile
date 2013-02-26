CC = gcc
CFLAGS = -O2 -Wall -I .

VPATH = io \
        cgi-bin

SOURCES = socketutil.c \
          ioutil.c
OBJECTS = $(SOURCES:.c=.o)

OUTPUT = mytiny \
         client

all: $(OUTPUT) cgi

$(OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) -c $<
#.SUFFIXES: .c .o
#.c.o:
#	$(CC) $(CFLAGS) -c $(filter %.c, $^)

$(OUTPUT): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $@.c $^

cgi:
	cd cgi-bin; make

clean: cleancgi
	rm -f *.o mytiny client *~

cleancgi:
	cd cgi-bin; make clean
