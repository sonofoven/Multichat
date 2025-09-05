#pragma once

#include <ncurses.h>
#include <form.h>
#include "client.hpp"

#define MAX_MSG_BUF 300
#define PAD_HEIGHT 2000


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

void appendMsgWin(UiContext& context, unique_ptr<formMsg>& formStr, bool redraw); //m//*

struct MsgWin : Win{
	vector<unique_ptr<formMsg>> msgBuf;
	int cursIdx; // What message is currently on bot of screen
	int cursOffset; // Curs line offset from top
	int occLines;  // Occupied LINES total
	int writeIdx = 0; // Buffer handling
	bool atTop;

	MsgWin() :
		msgBuf(),
		cursIdx(-1),
		cursOffset(0),
		occLines(0),
		atTop(false){
			msgBuf.reserve(MAX_MSG_BUF);
		}


	void addMsg(unique_ptr<formMsg> formStr){
		if (writeIdx >= (int)msgBuf.size()){
			msgBuf.push_back(move(formStr));
		} else {
			msgBuf[writeIdx] = move(formStr);
		}

		writeIdx = (writeIdx + 1) % MAX_MSG_BUF;
	}
	
	void replayMessages(UiContext& context){
		cursOffset = 0;
		occLines = 0;

		int n = (int)msgBuf.size();
		int tempCursIdx = cursIdx;
		int start = (n == MAX_MSG_BUF) ? writeIdx : 0;

		for (int i = 0; i < n; i++){
			int idx = (start + i) % MAX_MSG_BUF;
			appendMsgWin(context, msgBuf[idx], true);
			if (idx == tempCursIdx){
				cursOffset = occLines;
			}
		}
	}

	void advanceCurs(int lineShift){
		// Goes forward in time (closer to present)
		if ((cursIdx + 1) % MAX_MSG_BUF  == writeIdx){
			return;
		}

		cursIdx = (cursIdx + 1) % MAX_MSG_BUF;
		cursOffset += lineShift;
	}

	void revertCurs(int lineShift){
		// Goes back in time (further from present)
		if ((cursIdx - 1) % MAX_MSG_BUF == writeIdx){
			// Already at oldest
			return;
		}
		int idx = (cursIdx - 1) % MAX_MSG_BUF;

		if (idx >= (int)msgBuf.size()){
			// don't wanna access nonexistent past
			return;
		}

		cursIdx = idx;

		cursOffset -= lineShift;
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
void refreshFromTop(UiContext& context);
