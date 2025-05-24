#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <vector>
#include <list>
#include <string>
#include "../protocol.hpp"

#include <ncurses.h>

#define HALIGN 2
#define VALIGN 1


using namespace std;

// Store usernames connected
extern list<string> userConns;

typedef struct WIN_t{
	WINDOW* textWin; // Window that holds the text
	WINDOW* bordWin; // Window that hold the border
	vector<uint8_t> screenBuf; // Buffer that keeps track of all data for window
} WIN;

typedef struct UiContext_t{
	WIN* leftWin;
	WIN* topWin;
	WIN* botWin;

	UiContext_t(WIN* l, WIN* t, WIN* b) : leftWin(l), topWin(t), botWin(b) {}

} UiContext;

void sendPacket(int servFd, Packet* pkt);

vector<uint8_t> getTextInput();
	// Gets text input for a message. Pad the text input with null
	// terminators to fill out the struct

void interfaceStart();

WINDOW* createWindow(int height, int width, int starty, int startx);

WIN createLeftWin();
WIN createTopWin();
WIN createBotWin();

vector<uint8_t> getWindowInput(WIN& window);

void printToWindow(WIN window, vector<uint8_t> inputData);


void serverValidate(ServerValidate& pkt, UiContext& context);

void serverConnect(ServerConnect& pkt, UiContext& context);

void serverBroadMsg(ServerBroadMsg& pkt, UiContext& context);

void serverDisconnect(ServerValidate& pkt, UiContext& context);
