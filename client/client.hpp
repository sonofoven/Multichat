#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <vector>
#include <list>
#include <queue>
#include <string>
#include <condition_variable>
#include <algorithm>
#include <functional>
#include <thread>
#include <ctime>
#include "../protocol.hpp"

#include <ncurses.h>

#define HALIGN 2
#define VALIGN 1

using namespace std;

extern list<string> userConns; // List of users connected to server

	// Store usernames connected
extern vector<uint8_t> readBuf;

extern mutex writeMtx;
extern condition_variable writeCv; // Activate the writeBuf
extern vector<uint8_t> writeBuf;

extern mutex msgMtx; // May need to add a queue but should be good 4 now


struct Win{
	WINDOW* bordWin;		
	WINDOW* textWin;		 
	vector<chtype> screenBuf;

	Win() :
		bordWin(nullptr),
		textWin(nullptr),
		screenBuf() {}
};

struct UiContext{
	Win* userWin;
	Win* msgWin;
	Win* inputWin;

	UiContext(Win* u, 
			  Win* m, 
			  Win* i) 
			  : 
			  userWin(u), 
			  msgWin(m), 
			  inputWin(i) {}
};

// For storing connection information
struct connInfo{
	string addr;
	uint16_t port;
	string username = "Jimmy";
};

extern connInfo clientInfo;

UiContext interfaceStart();

void dealThreads(int sockFd, UiContext& context);

WINDOW* createWindow(int height, int width, int starty, int startx, bool boxOn, bool scroll);

Win createUserWin();
Win createMsgWin();
Win createInputWin();

vector<uint8_t> processOneChar(Win& window, UiContext& context, int ch, vector<uint8_t> outBuf);

vector<chtype> formatMessage(vector<uint8_t> message, const char* username);

vector<chtype> formatPktMessage(ServerBroadMsg& pkt);
vector<chtype> formatDisMessage(const char* username);
vector<chtype> formatConMessage(const char* username);


vector<uint8_t> getWindowInput(Win& window, UiContext& context);
void appendToWindow(Win& window, vector<chtype> inputVec, int prescroll);

void readThread(int servFd, UiContext& context);
void writeThread(int servFd);

int protocolParser(Packet* pkt, UiContext& context);
void userInput(UiContext& context);

void updateUserWindow(UiContext& context);

void serverValidate(ServerValidate& pkt, UiContext& context);
void serverConnect(ServerConnect& pkt, UiContext& context);
void serverBroadMsg(ServerBroadMsg& pkt, UiContext& context);
void serverDisconnect(ServerDisconnect& pkt, UiContext& context);

inline char getBaseChar(chtype ch);
void pushBackStr(string str, vector<chtype>& outBuf, attr_t attr);
