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
#include <optional>
#include <filesystem>
#include <thread>
#include <ctime>
#include <fstream>
#include "../protocol.hpp"

#define HALIGN 2
#define VALIGN 1

using namespace std;
using namespace std::filesystem;

// For storing connection information
struct connInfo{
	string addr;
	uint16_t port;
	string username;
};
extern connInfo clientInfo;

// List of users connected to server
extern list<string> userConns; 

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

// Server startup & client negotiation
int startUp();
int networkStart();
void sendOneConn(int servFd);
bool recvOneVal(int servFd);

// Config creation/loading/checking
path getConfDir();
bool fileCreate();
bool fileVerify();
bool checkCliInfo();
bool validateIpv4(string str);
optional<vector<string>> octetTokenize(string str);
