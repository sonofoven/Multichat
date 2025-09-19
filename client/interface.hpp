#pragma once

#include <ncurses.h>
#include <form.h>
#include <menu.h>
#include "client.hpp"

#define MAX_MSG_BUF 500
#define PAD_BUF_MULTI 5
#define MENU_HEIGHT 10
#define MENU_WIDTH 40
#define FIELD_OFFSET 4
#define PORT_MAX 65535


using namespace std;


struct formMsg {
	vector<chtype> header;
	vector<chtype> message;

	formMsg() :
		header(),
		message(){
			message.reserve(MAXMSG);
		}
};

// Window/UI structs
struct Win{
	WINDOW* bordWin = nullptr;	   
	WINDOW* textWin = nullptr;	   
	
	void freeWin();
};


struct MsgWin : Win{
	vector<unique_ptr<formMsg>> msgBuf;
	int cursOffset = 0; // Curs line offset from top
	int occLines = 0;  // Occupied LINES total
	int writeIdx = 0; // Buffer handling

	MsgWin(){
		msgBuf.reserve(MAX_MSG_BUF);
	}

	void addMsg(unique_ptr<formMsg> formStr);
	void replayMessages(UiContext& context);
	int maxMsgLine(int maxCols);
	void shiftPad(UiContext& context);
};

struct ChatContext : ContextState{
	state = MESSENGING;

	Win* userWin;
	MsgWin* msgWin;
	Win* inputWin;

	list<string> userConns = {};
	vector<uint8_t> readBuf;
	
	vector<uint8_t> writeBuf;
	string inputBuf;
	string serverName; 
	
	mutex logMtx;
	queue<unique_ptr<Packet>> logQueue;
	condition_variable queueCv;
	thread logT;
	atomic<bool> stopLog = false;

	int servFd = -1;
	int epollFd = -1;
	epoll_event events[MAX_EVENTS];


	// Control funcs
	int startProcess();
	int runProcess();
	int termProcess();
	void freeAll();

	// Epoll/Fd Setup
	void setupEpoll();
	void modFds();

	// Ui / Scrolling
	void setupWindows();
	void appendMsgWin(unique_ptr<formMsg>& formStr, bool redraw);
	void handleCh(int ch);
	void scrollBottom();
	void scrollUp();
	void scrollDown();
	void refreshFromCurs();

	//Logging
	void startLog();
	void stopLog();
	void restoreHistory();
	void logLoop();
	void appendToLog(unique_ptr<Packet> pkt);

	// Network Setup/Handling
	void setupFd();
	void networkStart();
	void sendOneConn();
	void recvOneVal();
	int drainReadFd();
	void handleRead();
	void handleWrite();

	// Packet handling
	int protocolParser(Packet* pkt);
	void serverValidate(Packet* pkt);
	void serverConnect(Packet* pkt);
	void serverBroadMsg(Packet* pkt);
	void serverDisconnect(Packet* pkt);

	// Redraw
	void redrawChat(int lines, int cols);
	void redrawMsgWin(int lines, int cols);
	void redrawInputWin(int lines, int cols);
	void redrawUserWin(int lines, int cols);
	

};

struct MenuContext{
	WINDOW* confWin = NULL;
	WINDOW* subWin = NULL;
	MENU* confMenu = NULL;
	vector<string> choices;
	vector<ITEM*> myItems;

	MenuContext(WINDOW* w, vector<string> c);

	void freeAll();
};



struct FormContext{
	state = FORM_FILL;

	WINDOW* bordWin = NULL;
	WINDOW* formWin = NULL;
	FORM* confForm = NULL;
	vector<string> fieldNames;
	vector<FIELD*> formFields;
	vector<WINDOW*> fieldBoxes;

	FormContext(WINDOW* w, vector<string> f);

	void setForm();
	bool validIpCh(int idx, int ch);

	void handleInput();
	string getFieldValue(FIELD* field);

	// File manipulation
	bool updateFile();
	path getConfDir();
	bool fileCreate();
	bool fileVerify();
	bool checkCliInfo();
	bool validateIpv4(string str);
	optional<vector<string>> octetTokenize(string str);
	
	// Do this after posting form
	void refresh();
	void freeAll();
};

struct WinErrState : ContextState {

}

struct ChatState : ContextState {

}

struct FormState : ContextState {

}

struct ReconnectState : ContextState {

}

struct FileState : ContextState {

}

// Setup
void interfaceStart();
UiContext setupWindows();
void setupForm();

// Formatting tools
void pushBackStr(string str, vector<chtype>& outBuf, attr_t attr);
unique_ptr<formMsg> formatMessage(time_t time, string& message, string& username); 
unique_ptr<formMsg> formatDisMessage(time_t time, string& username); 
unique_ptr<formMsg> formatConMessage(time_t time, string& username); 
string formatTime(time_t timestamp);
string dateStr(int day);


// Window creation
Win* createUserWin(int lines, int cols);
MsgWin* createMsgWin(string title, int lines, int cols); 
Win* createInputWin(int lines, int cols);
WINDOW* createWindow(int height, 
					 int width, 
					 int starty, 
					 int startx, 
					 bool boxOn, 
					 bool scrollOn);

// Window I/O
string getWindowInput(Win& window, UiContext& context);

void updateUserWindow(UiContext& context);
void handleCh(UiContext& context, int ch, int servFd); 
inline char getBaseChar(chtype ch);
void restoreHistory(UiContext& context); 

// Redrawing
void redrawInputWin(Win* window, int lines, int cols);
void redrawUserWin(Win* window, int lines, int cols);
void redrawMsgWin(Win* window, int lines, int cols); 

// Scrolling
int lineCount(const unique_ptr<formMsg>& formStr, int maxCols);
void scrollBottom(UiContext& context);
void scrollUp(UiContext& context);
void scrollDown(UiContext& context);
void refreshFromCurs(UiContext& context);

// Form
int configForm();

// Menu
int menuSetup(string caption, vector<string> choices);
int configMenu();
int reconnectMenu();
WINDOW* centerWin(WINDOW* parent, string& title, string& caption, int height, int width);

