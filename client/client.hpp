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
#include "../protocol.hpp"

#include <ncurses.h>

#define HALIGN 2
#define VALIGN 1

enum renderCode {
	MESSAGE,
	USERUPDATE,
	USERCONN,
	USERDISC,
	NUM_OF_CODES
};

using namespace std;
extern list<string> userConns; // List of users connected to server
extern mutex userMtx; // Protects userconn list

extern mutex queueMtx; // Protects render queue
extern mutex writeMtx; // Protects writeBuf

	// Store usernames connected
extern vector<uint8_t> readBuf;

extern condition_variable writeCv; // Activate the writeBuf
extern vector<uint8_t> writeBuf;


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

// Render information of each render query
struct renderItem{
	Win* target;
	renderCode rcode;
	vector<chtype> data;
};

// For storing connection information
struct connInfo{
	string addr;
	uint16_t port;
	string username;
};

extern connInfo clientInfo;
extern queue<renderItem> renderQueue;


UiContext interfaceStart();

void dealThreads(int sockFd, UiContext& context);

WINDOW* createWindow(int height, int width, int starty, int startx, bool boxOn, bool scroll);

Win createUserWin();
Win createMsgWin();
Win createInputWin();

vector<uint8_t> processOneChar(Win& window, UiContext& context, int ch, vector<uint8_t> outBuf);

vector<chtype> formatMessage(vector<uint8_t> message, const char* username);

void appendToWindow(Win& window, vector<chtype> inputVec, int prescroll);

void processRender(renderItem rItem);

void updateUserWindow(renderItem rItem);
void updateMessageWindow(renderItem rItem);
void updateUserConn(renderItem rItem);
void updateUserDisc(renderItem rItem);



void inputThread(UiContext& context);
void readThread(int servFd, UiContext& context);
void writeThread(int servFd);

int protocolParser(Packet* pkt, UiContext& context);

void serverValidate(ServerValidate& pkt, UiContext& context);
void serverConnect(ServerConnect& pkt, UiContext& context);
void serverBroadMsg(ServerBroadMsg& pkt, UiContext& context);
void serverDisconnect(ServerDisconnect& pkt, UiContext& context);

inline char getBaseChar(chtype ch);
