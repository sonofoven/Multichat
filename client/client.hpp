#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/signalfd.h>
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

using namespace std;
using namespace std::filesystem;

enum uiState {
	FILE_DETECT,
	FORM_FILL,
	RECONNECT,
	MESSENGING
};

// For storing connection information
struct connInfo{ // This stays global
	string addr;
	uint16_t port;
	string username;
};

struct ContextState{
	uiState state;

	virtual int startUp() = 0;
	virtual int handleInput(int ch) = 0;
	virtual int tearDown() = 0;
};

extern int epollFd;
extern int servFd;
extern shared_mutex fileMtx;
extern connInfo clientInfo;

extern mutex logMtx;
extern queue<unique_ptr<Packet>> logQueue;
extern condition_variable queueCv;
extern thread logT;
extern atomic<bool> reaccLogTarget;

// UI / Window IO
void redrawUi();

// Logging //
void startLog(); 
void logLoop(); 
void appendToLog(unique_ptr<Packet> pkt);
