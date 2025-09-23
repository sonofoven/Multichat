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
struct connInfo{ // This stays global
	string addr;
	uint16_t port;
	string username;
};

struct ContextState{
	uiState state;

	virtual int startUp() = 0;
	virtual int running() = 0;
	virtual int tearDown() = 0;
};

// -2 means bad response, go back
// -1 means winErr or redraw for chat
// 0 means good/first option
// 1 means second option
// 2 means skip state

extern shared_mutex fileMtx;
extern connInfo clientInfo;
extern atomic<bool> redrawQueued;

// UI / Window IO
void redrawUi();

// Signal Handling
void sigwinchHandler(int sig);
