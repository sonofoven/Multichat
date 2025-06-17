#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <vector>
#include <list>
#include <queue>
#include <string>
#include <algorithm>
#include <functional>
#include <thread>
#include <ctime>
#include "../protocol.hpp"

#define HALIGN 2
#define VALIGN 1

using namespace std;

// For storing connection information
struct connInfo{
	string addr = "127.0.0.1";
	uint16_t port = 8080;
	string username = "Yimmy";
};

// List of users connected to server
extern list<string> userConns; 

extern connInfo clientInfo;
extern vector<uint8_t> readBuf;
extern vector<uint8_t> writeBuf;
extern string inputBuf;
extern int epollFd;

struct UiContext;

int protocolParser(Packet* pkt, UiContext& context);

void handleRead(int servFd, UiContext& context);
void handleWrite(int servFd);
int drainReadFd(int servFd);

void serverValidate(ServerValidate& pkt, UiContext& context);
void serverConnect(ServerConnect& pkt, UiContext& context);
void serverBroadMsg(ServerBroadMsg& pkt, UiContext& context);
void serverDisconnect(ServerDisconnect& pkt, UiContext& context);

int networkStart();
	// Returns socket # if good

int startUp();
	// Inits the client and does the handshake with the server

void sendOneConn(int servFd);

bool recvOneVal(int servFd);
