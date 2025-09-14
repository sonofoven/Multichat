#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <csignal>
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
#include <mutex>
#include <atomic>
#include <shared_mutex>
#include <regex>
#include <condition_variable>
#include "../protocol.hpp"

#define HALIGN 2
#define VALIGN 1
#define VERSION "1.0"
#define LOG_DAY_MAX 7
#define STORAGE_DIR ".multiChat"
//#define MIN_LINES 25
//#define MIN_COLS 65

using namespace std;
using namespace std::filesystem;

enum uiState {
	SIZE_ERR,
	FILE_DETECT,
	FORM_FILL,
	RECONNECT,
	MESSENGING
};

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
extern string serverName;

// Logging
extern queue<unique_ptr<Packet>> logQueue;
extern mutex logMtx; // Control the logging queue
extern shared_mutex fileMtx; // Control the file access
extern condition_variable queueCv;
extern atomic<bool> redrawQueued;
extern atomic<bool> windowDisplayed;

void logLoop();
list<path> detectLogFiles();
void addToLog(string str, list<path>& logFiles);
void weenLogFiles(list<path>& logFiles);
path logFilePath();
path getLogDir();
void appendToLog(unique_ptr<Packet> ptr);
time_t getLatestLoggedMsgTime();


// UI / Window IO
struct UiContext;

int protocolParser(Packet* pkt, UiContext& context);

void handleRead(int servFd, UiContext& context);
void handleWrite(int servFd);
int drainReadFd(int servFd);

void serverValidate(ServerValidate& pkt, UiContext& context);
void serverConnect(ServerConnect& pkt, UiContext& context);
void serverBroadMsg(ServerBroadMsg& pkt, UiContext& context);
void serverDisconnect(ServerDisconnect& pkt, UiContext& context);
void redrawUi(UiContext& context, int lines, int cols); //m//*

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

// Signal Handling
void sigwinchHandler(int sig);
