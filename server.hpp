#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>


#include <errno.h>

#include <cstring>
#include <iostream>
#include <thread>

#define BACKLOG_MAX 20
#define MAX_EVENTS 30
#define PORT 8080


using namespace std;


int makeListenSocket(sockaddr_in address);
	// Returns a listen socket given an empty addr

void addFdToEpoll(int epoll, int fd);

void handleAccept(int epollFd, int listenFd);

void handleReadOrWrite(int fd);

void acceptLoop(int listenFd, int epollFd);
