#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <endian.h>
#include <unordered_map>
#include <deque>
#include <vector>
#include <algorithm>


#include <errno.h>
#include <array>
#include <cstring>
#include <iostream>
#include <thread>

#define BACKLOG_MAX 20
#define MAX_EVENTS 30
#define PORT 8080

#define CHUNK 4096


using namespace std;

enum opcode {
	CONNECT,
	MSG,
	MSGBULK,
	DISCONNECT
};

class clientConn {
public:
	deque<uint8_t> readBuf;
	deque<uint8_t> writeBuf;
	int fd;

	clientConn () : fd(-1) {} // Default constructor
	clientConn (int fileDescriptor) : fd(fileDescriptor) {}
};

struct PacketHeader {
	uint16_t length;
	uint16_t opcode;
};

int makeListenSocket(sockaddr_in address);
	// Returns a listen socket given an empty addr

void addFdToEpoll(int epoll, int fd);
	// Adds a file descriptor to the epoll pool


void modFdEpoll(int epollFd, int fd, int ops);
	// Modifies a file descriptor within the epoll pool

void handleRead(int epollFd, clientConn& client);

void handleWrite(int epollFd, clientConn& client);

void acceptLoop(int listenFd, int epollFd, unordered_map<int, clientConn>& clients);
	// Runs on a new thread, constantly accepts
	// new conns and adds them to the client
	// list and epoll pool

PacketHeader extractHeader(clientConn& client);
	// Read from the beginning of the read buffer
	// to attain the header

void drainReadPipe(int fd, clientConn& client);
	// Read the read pipe til we run out
	// or until we can't anymore
