CC = g++
CFLAGS = -Wall -g -fpic -pedantic

all: client server

client:
	$(CC) $(CFLAGS) -o $@ client.cpp

server:
	$(CC) $(CFLAGS) -o $@ server.cpp

clean:
	rm client server
