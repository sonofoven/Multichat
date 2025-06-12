#include <netinet/in.h>
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
#include <atomic>
#include <csignal>

#include "../protocol.hpp"

#define BACKLOG_MAX 20
#define MAX_BUF_SIZE 65536

using namespace std;

class clientConn {
public:
	vector<uint8_t> readBuf;
	vector<uint8_t> writeBuf;
	int fd;
	uint32_t epollMask = EPOLLIN | EPOLLET;
	string username;
	bool markToDie = false;

	clientConn () : fd(-1) { // Default constructor
		readBuf.reserve(CHUNK);
		writeBuf.reserve(CHUNK);
	}
	clientConn (int fileDescriptor) : fd(fileDescriptor) {
		readBuf.reserve(CHUNK);
		writeBuf.reserve(CHUNK);
	}
};

// fd to clientConnectionMap
extern unordered_map<int, clientConn> clientMap;
extern mutex clientMapMtx;

// username to fd
extern unordered_map<string, int> userMap;

// 
extern int epollFd;

//int* listenFdPtr;

int makeListenSocket(sockaddr_in address);
	// Returns a listen socket given an empty addr

void addFdToEpoll(int epoll, int fd);
	// Adds a file descriptor to the epoll pool


void modFdEpoll(int fd, int ops);
	// Modifies a file descriptor within the epoll pool

int handleRead(clientConn& client);

int handleWrite(clientConn& client);

void acceptLoop(int listenFd);
	// Runs on a new thread, constantly accepts
	// new conns and adds them to the client
	// list and epoll pool

int protocolParser(Packet* packet, clientConn& sender);

size_t parsePacketLen(uint8_t* data);


int drainReadPipe(int fd, clientConn& client);
	// Read the read pipe til we run out
	// or until we can't anymore


void killClient(int fd);

void killUser(string& user);

void killServer(int code);

//// Utils

string validateUser(string& username);
	// Makes sure the username is of correct length
	// Makes sure the username is not taken
	// Returns string if it is a valid username

void clientConnect(ClientConnect& pkt, clientConn& sender);

void clientBroadMsg(ClientBroadMsg& pkt, clientConn& sender);

void clientServerMessage(ClientServMsg& pkt, clientConn& sender);



void dropClient(int fd);
	// Informs other clients of drop if registered
	// kills otherwise

clientConn* lockFindCli(int fd);
	// Lock down the map
	// Find the client corresponding to the fd
	// release the lock

string usernameExists(string& username);
	// Null if the username already exists
	// Stringifies cstring if it doesn't
	// This is b/c we're going use it

bool valConnMsg(string& username, clientConn& sender);

void serializeToAllButSender(Packet& pkt, clientConn& sender);
