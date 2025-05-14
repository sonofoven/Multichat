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


typedef struct windowBorderStruct {
	chtype 	ls, rs, ts, bs, tl, tr, bl, br;
}WIN_BORDER;


typedef struct windowStruct {
	int startx, starty;
	int height, width;
	WIN_BORDER border;
}WIN;


void sendPacket(int servFd, uint8_t* packet, size_t packetLen);

vector<uint8_t> getTextInput();
	// Gets text input for a message. Pad the text input with null
	// terminators to fill out the struct

PacketHeader makeHeader(opcode code, size_t stringLen);
	// Makes a header filled with opcode and stringLen

void interfaceStart();

void initWindParams();
