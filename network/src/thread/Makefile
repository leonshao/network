CC = gcc
CFLAGS = -O2 -Wall -I .

LIB = -lpthread
SOURCES = hello_n.c
OUTPUT = hello

$(OUTPUT):
	$(CC) $(CFLAGS) -o $(OUTPUT) $(SOURCES) $(LIB)

clean:
	rm -f *.o hello *~