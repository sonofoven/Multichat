#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <vector>
#include <string>
#include "../protocol.hpp"

#include <ncurses.h>

#define HALIGN 2
#define VALIGN 1


using namespace std;

typedef struct {
	WINDOW* textWin; // Window that holds the text
	WINDOW* bordWin; // Window that hold the border
	vector<uint8_t> screenBuf; // Buffer that keeps track of all data for window
} WIN;

void sendPacket(int servFd, Packet* pkt);

vector<uint8_t> getTextInput();
	// Gets text input for a message. Pad the text input with null
	// terminators to fill out the struct

//PacketHeader makeHeader(opcode code, size_t stringLen);
	// Makes a header filled with opcode and stringLen

void interfaceStart();

WINDOW* createWindow(int height, int width, int starty, int startx);

WIN createLeftWin();
WIN createTopWin();
WIN createBotWin();

vector<uint8_t> getWindowInput(WINDOW* win);

void printToWindow(WIN window, vector<uint8_t> inputData);
