#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include <unordered_map>
#include <queue>
#include <vector>
#include <algorithm>
#include <array>

#include <cstring>
#include <iostream>
#include <thread>
#include <mutex>

#include "../protocol.hpp"

#define BACKLOG_MAX 20
#define MAX_EVENTS 30
#define MAX_BUF_SIZE 65536

using namespace std;

class clientConn {
public:
	vector<uint8_t> readBuf;
	vector<uint8_t> writeBuf;
	int fd;
	uint32_t epollMask = EPOLLIN | EPOLLET;

	clientConn () : fd(-1) { // Default constructor
		readBuf.reserve(CHUNK);
		writeBuf.reserve(CHUNK);
	}
	clientConn (int fileDescriptor) : fd(fileDescriptor) {
		readBuf.reserve(CHUNK);
		writeBuf.reserve(CHUNK);
	}
};

//uint16_t& value = *(vec.begin + bytes);

int makeListenSocket(sockaddr_in address);
	// Returns a listen socket given an empty addr

void addFdToEpoll(int epoll, int fd);
	// Adds a file descriptor to the epoll pool


void modFdEpoll(int epollFd, int fd, int ops);
	// Modifies a file descriptor within the epoll pool

int handleRead(int epollFd, clientConn& client);

int handleWrite(int epollFd, clientConn& client);

void acceptLoop(int listenFd, int epollFd);
	// Runs on a new thread, constantly accepts
	// new conns and adds them to the client
	// list and epoll pool

int protocolParser(Packet* packet, clientConn& sender);

size_t parsePacketLen(uint8_t* data);

void clientServerMessage(Packet* packet, clientConn& sender);

int drainReadPipe(int fd, clientConn& client);
	// Read the read pipe til we run out
	// or until we can't anymore

void killClient(int fd);

void killServer(int code);
