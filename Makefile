CC = g++
CFLAGS = -Wall -g -fpic -pedantic

all: client server

client:
	$(CC) $(CFLAGS) -o $@ client.cpp 2> clientErr

server:
	$(CC) $(CFLAGS) -o $@ server.cpp 2> serverErr

clean:
	rm -f client server clientErr serverErr
