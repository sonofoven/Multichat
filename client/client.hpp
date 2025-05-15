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


using namespace std;

void sendPacket(int servFd, uint8_t* packet, size_t packetLen);

vector<uint8_t> getTextInput();
	// Gets text input for a message. Pad the text input with null
	// terminators to fill out the struct

PacketHeader makeHeader(opcode code, size_t stringLen);
	// Makes a header filled with opcode and stringLen

void interfaceStart();

WINDOW* createWindow(int height, int width, int starty, int startx);

WINDOW* createLeftWin();
WINDOW* createTopWin();
WINDOW* createBotWin();
