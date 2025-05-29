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
#include <time.h>
#include <locale.h>
#include <stdio.h>
#include "../protocol.hpp"

#include <notcurses/notcurses.h>

#define HALIGN 2
#define VALIGN 1


using namespace std;
extern list<string> userConns;
	// Store usernames connected

extern vector<uint8_t> readBuf;
extern vector<uint8_t> writeBuf;

struct Panel {
	ncplane* bordPlane;		
	ncplane* textPlane;		 
	unique_ptr<mutex> ncMtx;	// mutex to control panel writes
	vector<nccell> screenBuf;	// Output cacheing
	bool dirty;

	Panel() :
		bordPlane(nullptr),
		textPlane(nullptr),
		ncMtx(make_unique<mutex>()),
		screenBuf(),
		dirty(false) {}

	//Panel(const Panel&) = delete;
	//Panel& operator=(const Panel&) = delete;
};

struct UiContext {
	notcurses* nc;	// context to do global things
	ncplane* stdp;
	Panel* userPanel;
	Panel* msgPanel;
	Panel* inputPanel;
	
	UiContext(notcurses* n, 
			  ncplane* s, 
			  Panel* u, 
			  Panel* m, 
			  Panel* i) 
			  : 
			  nc(n), 
			  stdp(s),
			  userPanel(u), 
			  msgPanel(m), 
			  inputPanel(i) {}
	
	UiContext& operator=(const UiContext&) = delete;
};

UiContext interfaceStart();
//void dealThreads(int sockFd, UiContext& context);
//void updateUserWindow(UiContext& context);


ncplane* createPanel(ncplane* parentPlane, 
					 unsigned height, 
					 unsigned width, 
					 int starty, 
					 int startx, 
					 uint64_t flags, 
					 bool box);

Panel createInputPanel(ncplane* parentPlane);
Panel createMsgPanel(ncplane* parentPlane);
Panel createUserPanel(ncplane* parentPlane);

void addBox(ncplane* plane);
//vector<uint8_t> getPanelInput(Panel& window, UiContext& context);

//void printToWindow(Panel& window, vector<uint8_t> inputData);

//void serverValidate(ServerValidate& pkt, UiContext& context);
//void serverConnect(ServerConnect& pkt, UiContext& context);
//void serverBroadMsg(ServerBroadMsg& pkt, UiContext& context);
//void serverDisconnect(ServerDisconnect& pkt, UiContext& context);

//void inputThread(UiContext& context, bool& ready, condition_variable& writeCv, mutex& writeMtx);
//void readThread(int servFd, UiContext& context);
//void writeThread(int servFd, bool& ready, condition_variable& writeCv, mutex& writeMtx);

//int protocolParser(Packet* pkt, UiContext& context);
