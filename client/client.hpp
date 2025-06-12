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

// List of users connected to server
extern list<string> userConns; 

extern vector<uint8_t> readBuf;
extern vector<uint8_t> writeBuf;
extern string inputBuf;
extern int epollFd;

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
	string addr = "127.0.0.1";
	uint16_t port = 8080;
	string username = "Dimmy";
};

extern connInfo clientInfo;

UiContext interfaceStart();

void dealThreads(int sockFd, UiContext& context);

// Window creation funcs
WINDOW* createWindow(int height, 
					 int width, 
					 int starty, 
					 int startx, 
					 bool boxOn, 
					 bool scroll);
Win createUserWin();
Win createMsgWin();
Win createInputWin();

// Formatting funcs
vector<chtype> formatMessage(string& message, string& username);
vector<chtype> formatDisMessage(string& username);
vector<chtype> formatConMessage(string& username);


string getWindowInput(Win& window, UiContext& context);
void appendToWindow(Win& window, vector<chtype> inputVec, int prescroll);

int protocolParser(Packet* pkt, UiContext& context);

void updateUserWindow(UiContext& context);

void handleCh(UiContext& context, int ch, int servFd);
void handleWrite(int servFd, UiContext& context);
void handleRead(int servFd, UiContext& context);

void serverValidate(ServerValidate& pkt, UiContext& context);
void serverConnect(ServerConnect& pkt, UiContext& context);
void serverBroadMsg(ServerBroadMsg& pkt, UiContext& context);
void serverDisconnect(ServerDisconnect& pkt, UiContext& context);

inline char getBaseChar(chtype ch);
void pushBackStr(string str, vector<chtype>& outBuf, attr_t attr);

int networkStart();
	// Returns socket # if good

int startUp();
	// Inits the client and does the handshake with the server

void sendOneConn(int servFd);

bool recvOneVal(int servFd);
