#pragma once

#include <ncurses.h>
#include <form.h>
#include "client.hpp"

using namespace std;

// Window/UI structs
struct Win{
	WINDOW* bordWin;	   
	WINDOW* textWin;	   
	vector<chtype> screenBuf;
	int firstVisChIdx;
	int lastVisChIdx;

	Win() :
		bordWin(nullptr),
		textWin(nullptr),
		screenBuf(),
		firstVisChIdx(0),
		lastVisChIdx(0) {}
};

struct UiContext{
	Win* userWin;
	Win* msgWin;
	Win* inputWin;
	bool uiDisplayed;
	bool alignedOnBottom;

	UiContext(Win* u, 
			  Win* m, 
			  Win* i) 
			  : 
			  userWin(u), 
			  msgWin(m), 
			  inputWin(i),
			  uiDisplayed(true), 
			  alignedOnBottom(true){}

};

// Setup
void interfaceStart();
UiContext setupWindows();
void setupForm();

// Formatting tools
void pushBackStr(string str, vector<chtype>& outBuf, attr_t attr);
vector<chtype> formatMessage(time_t time, string& message, string& username);
vector<chtype> formatDisMessage(time_t time, string& username);
vector<chtype> formatConMessage(time_t time, string& username);
string formatTime(time_t timestamp);
string dateStr(int day);


// Window creation
Win* createUserWin(int lines, int cols);
Win* createMsgWin(string title, int lines, int cols);
Win* createInputWin(int lines, int cols);
WINDOW* createWindow(int height, 
					 int width, 
					 int starty, 
					 int startx, 
					 bool boxOn, 
					 bool scrollOn);

// Window I/O
string getWindowInput(Win& window, UiContext& context);
void appendMsgWin(UiContext& context, vector<chtype> inputVec);
void updateUserWindow(UiContext& context);
void handleCh(UiContext& context, int ch, int servFd);
inline char getBaseChar(chtype ch);
void restoreHistory(UiContext& context);

// Redrawing
void redrawInputWin(Win* window, int lines, int cols);
void redrawUserWin(Win* window, int lines, int cols);
void redrawMsgWin(Win* window, int lines, int cols);

void restoreStringToWin(Win* window);
void restoreTextToWin(Win* window);
void restoreTextScrolled(UiContext& context);

// Scrolling
int calcLineCount(vector<chtype>& screenBuf, WINDOW* win);
void scrollBottom(UiContext& context);
void scrollUp(UiContext& context);
void scrollDown(UiContext& context);

int linesAbove(int pos, Win* win);
int linesBelow(int pos, Win* win);
int prevLinesAway(int pos, Win* window, int lineOffset);
int nextLinesAway(int pos, Win* window, int lineOffset);
void addLine(UiContext& context, int startPos, int dir);
int getTopCh(int botLine, Win* win);
int getBotCh(int topLine, Win* win);
