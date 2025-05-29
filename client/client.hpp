#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <vector>
#include <list>
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
	NUM_OF_CODES
};

using namespace std;
extern list<string> userConns;
extern mutex queueMtx;
extern mutex writeMtx;
extern mutex userMtx;

	// Store usernames connected
extern vector<uint8_t> readBuf;
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

struct renderItem{
	Win* target;
	renderCode rcode;
	vector<chtype> data;
};

extern vector<renderItem> renderQueue;

//vector<uint8_t> getTextInput();
	// Gets text input for a message. Pad the text input with null
	// terminators to fill out the struct

UiContext interfaceStart();

//void dealThreads(int sockFd, UiContext& context);
//void updateUserWindow(WIN& window);

WINDOW* createWindow(int height, int width, int starty, int startx, bool boxOn, bool scroll);

Win createUserWin();
Win createMsgWin();
Win createInputWin();

vector<uint8_t> getWindowInput(Win& window, UiContext& context);

void appendToWindow(Win& window, string& inputStr, attr_t attributes, int prescroll);

processRender(renderItem rItem);

//void serverValidate(ServerValidate& pkt, UiContext& context);
//void serverConnect(ServerConnect& pkt, UiContext& context);
//void serverBroadMsg(ServerBroadMsg& pkt, UiContext& context);
//void serverDisconnect(ServerDisconnect& pkt, UiContext& context);
//
//void inputThread(UiContext& context, bool& ready, condition_variable& writeCv, mutex& writeMtx);
//void readThread(int servFd, UiContext& context);
//void writeThread(int servFd, bool& ready, condition_variable& writeCv, mutex& writeMtx);
//
//int protocolParser(Packet* pkt, UiContext& context);
