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
#include <chrono>
#include "../protocol.hpp"

#include <ncurses.h>

#define HALIGN 2
#define VALIGN 1


using namespace std;
using namespace std::chrono;
extern list<string> userConns;
	// Store usernames connected
extern vector<uint8_t> readBuf;
extern vector<uint8_t> writeBuf;

typedef struct WIN_t{
	WINDOW* textWin; // Window that holds the text
	WINDOW* bordWin; // Window that hold the border
	vector<chtype> screenBuf; // Buffer that keeps track of all data for window
} WIN;

typedef struct UiContext_t{
	WIN* userWin;
	WIN* msgWin;
	WIN* inputWin;
	mutex& ncursesMtx; // For doing any ncurses modifications

	UiContext_t(WIN* u, WIN* m, WIN* i, mutex& n) : userWin(u), msgWin(m), inputWin(i), ncursesMtx(n) {}

  	UiContext_t& operator=(const UiContext_t&) = delete; // Removes the copy from the reference
} UiContext;

vector<uint8_t> getTextInput();
	// Gets text input for a message. Pad the text input with null
	// terminators to fill out the struct

UiContext interfaceStart();
void dealThreads(int sockFd, UiContext& context);
void updateUserWindow(WIN& window);

WINDOW* createWindow(int height, int width, int starty, int startx);

WIN createLeftWin();
WIN createTopWin();
WIN createBotWin();

vector<uint8_t> getWindowInput(WIN& window, UiContext& context);

void printToWindow(WIN& window, vector<uint8_t> inputData);

void serverValidate(ServerValidate& pkt, UiContext& context);
void serverConnect(ServerConnect& pkt, UiContext& context);
void serverBroadMsg(ServerBroadMsg& pkt, UiContext& context);
void serverDisconnect(ServerDisconnect& pkt, UiContext& context);

void inputThread(UiContext& context, bool& ready, condition_variable& writeCv, mutex& writeMtx);
void readThread(int servFd, UiContext& context);
void writeThread(int servFd, bool& ready, condition_variable& writeCv, mutex& writeMtx);

int protocolParser(Packet* pkt, UiContext& context);
