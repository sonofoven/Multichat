#pragma once

#include <ncurses.h>
#include <form.h>
#include "client.hpp"

#define MAX_MSG_BUF 200
#define INIT_PAD_HEIGHT 200
#define MAX_PAD_HEIGHT 2000


using namespace std;


struct formMsg {
	vector<chtype> header;
	vector<chtype> message;

	formMsg() :
		header(),
		message() {
			message.reserve(MAXMSG);
		}
};

// Window/UI structs
struct Win{
	WINDOW* bordWin;	   
	WINDOW* textWin;	   

	Win() :
		bordWin(nullptr),
		textWin(nullptr) {}
};

struct MsgWin : Win{
	vector<unique_ptr<formMsg>> msgBuf;
	int cursIdx; // What message is currently on bot of screen
	int shiftRemainder; // How much message needs to go down til next one (cut off)
	int cursOffset; // Curs line offset from top
	int occLines;  // Occupied LINES total
	//RESTABLISH CURSOFFSET @ REDRAW FROM CURSIDX

	MsgWin() :
		msgBuf(),
		cursIdx(-1),
		shiftRemainder(0),
		cursOffset(0),
		occLines(0){
			msgBuf.reserve(MAX_MSG_BUF);
		}
};

struct UiContext{
	Win* userWin;
	MsgWin* msgWin;
	Win* inputWin;
	bool uiDisplayed;

	UiContext(Win* u, 
			  MsgWin* m, 
			  Win* i) 
			  : 
			  userWin(u), 
			  msgWin(m), 
			  inputWin(i),
			  uiDisplayed(true){}
};

// Setup
void interfaceStart();
UiContext setupWindows(); //m//*
void setupForm();

// Formatting tools
void pushBackStr(string str, vector<chtype>& outBuf, attr_t attr);
unique_ptr<formMsg> formatMessage(time_t time, string& message, string& username); //m//*
unique_ptr<formMsg> formatDisMessage(time_t time, string& username); //m//*
unique_ptr<formMsg> formatConMessage(time_t time, string& username); //m//*
string formatTime(time_t timestamp);
string dateStr(int day);


// Window creation
Win* createUserWin(int lines, int cols);
MsgWin* createMsgWin(string title, int lines, int cols); //m//*
Win* createInputWin(int lines, int cols);
WINDOW* createWindow(int height, 
					 int width, 
					 int starty, 
					 int startx, 
					 bool boxOn, 
					 bool scrollOn);

// Window I/O
string getWindowInput(Win& window, UiContext& context);

void appendMsgWin(UiContext& context, unique_ptr<formMsg> formStr); //m//*
void updateUserWindow(UiContext& context);
void handleCh(UiContext& context, int ch, int servFd); //m//*
inline char getBaseChar(chtype ch);
void restoreHistory(UiContext& context); //m//*

// Redrawing
void redrawInputWin(Win* window, int lines, int cols);
void redrawUserWin(Win* window, int lines, int cols);
void redrawMsgWin(Win* window, int lines, int cols); //m//*

// Scrolling
int lineCount(const unique_ptr<formMsg>& formStr, int maxCols);
void scrollBottom(UiContext& context);
void scrollUp(UiContext& context);
void scrollDown(UiContext& context);
void refreshFromCurs(UiContext& context);
