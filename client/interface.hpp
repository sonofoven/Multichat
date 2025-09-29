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
#define REDRAW_WAIT_MS 250

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
	int maxMsgLine(int maxCols);
};

struct ChatContext{

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
	atomic<bool> termLog = false;


	// Control funcs //
	int startProcess();
	int termProcess();
	void freeAll();

	// Epoll/Fd Setup //
	void setupEpoll();
	void modFds();

	// Ui //
	void setupWindows();
	void appendMsgWin(unique_ptr<formMsg>& formStr, bool redraw);
	void handleCh(int ch); //  
	void scrollBottom();
	void scrollUp();
	void scrollDown();
	void replayMessages();
	void refreshFromCurs();
	void shiftPad();
	void updateUserWindow();

	// Logging //
	void startLog(); 
	void stopLog(); 
	void restoreHistory();
	void logLoop(); 
	void appendToLog(unique_ptr<Packet> pkt);

	// Network Setup/Handling //
	void servFdStart();
	int networkStart();
	void sendOneConn();
	bool recvOneVal();
	void handleRead();
	void handleWrite();
	int drainReadFd();

	// Packet handling //
	int protocolParser(Packet* pkt);
	void serverValidate(ServerValidate& pkt);
	void serverConnect(ServerConnect& pkt);
	void serverBroadMsg(ServerBroadMsg& pkt);
	void serverDisconnect(ServerDisconnect& pkt);

	// Redraw //
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

	int menuSetup(vector<string> choices, string caption);
	int handleCh(int ch);

	void freeAll();
};



struct FormContext{

	WINDOW* bordWin = NULL;
	WINDOW* formWin = NULL;
	FORM* confForm = NULL;
	vector<string> fieldNames;
	vector<FIELD*> formFields;
	vector<WINDOW*> fieldBoxes;

	FormContext(WINDOW* w, vector<string> f);

	void setForm();
	bool validIpCh(int idx, int ch);
	int handleCh(int ch);
	string getFieldValue(FIELD* field);
	void updateConnInfo();
	static bool fileCreate();
	static bool fileVerify();
	static bool checkCliInfo();
	static bool validateIpv4(string str);
	static optional<vector<string>> octetTokenize(string str);
	
	// Do this after posting form
	void refreshForm();
	void freeAll();
};

struct ChatState : ContextState {
	ChatState() {state = MESSENGING;}
	unique_ptr<ChatContext> Chat;

	int startUp() override;
	int handleInput(int ch) override;
	int tearDown() override;

};

struct FormState : ContextState {
	FormState() {state = FORM_FILL;}
	unique_ptr<FormContext> Form;

	int startUp() override;
	int handleInput(int ch) override;
	int tearDown() override;

};

struct ReconnectState : ContextState {
	ReconnectState() {state = RECONNECT;}
	unique_ptr<MenuContext> Menu;

	int startUp() override;
	int handleInput(int ch) override;
	int tearDown() override;

};

struct FileState : ContextState {
	FileState() {state = FILE_DETECT;}
	unique_ptr<MenuContext> Menu;

	int startUp() override;
	int handleInput(int ch) override;
	int tearDown() override;

};


struct ContextController{
	unique_ptr<ContextState> curState = make_unique<FileState>();

	// Epoll starts at very beginning
	//int epollFd = -1;
	epoll_event events[MAX_EVENTS];

	// Sig -> fd
	int winchFd = -1;
	void setupEpoll();

	// Controllers setups the network connection
	void controlEpoll();
	void addServFd(int newServFd);
	void rmServFd(int oldServFd);

	// Input
	int handleChar();
	void handleWinch();
	int handleServFd(uint32_t event);

	// Transitions
	void stateChange(int status);
	void switchIntoChat();
};


// Setup
void interfaceStart();

// Formatting tools
void pushBackStr(string str, vector<chtype>& outBuf, attr_t attr);
unique_ptr<formMsg> formatMessage(time_t time, string& message, string& username); 
unique_ptr<formMsg> formatDisMessage(time_t time, string& username); 
unique_ptr<formMsg> formatConMessage(time_t time, string& username); 
string formatTime(time_t timestamp);
string dateStr(int day);
inline char getBaseChar(chtype ch);


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
WINDOW* centerWin(WINDOW* parent, string& title, string& caption, int height, int width);

// Scrolling
int lineCount(const unique_ptr<formMsg>& formStr, int maxCols);

// Logging
time_t getLatestLoggedMsgTime();
list<path> detectLogFiles();
void addToLog(string str, list<path>& logFiles);
void weenLogFiles(list<path>& logFiles);
path logFilePath();
path getLogDir();
path getConfDir();
