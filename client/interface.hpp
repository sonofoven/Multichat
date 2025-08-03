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
	chtype* firstVisibleChar;
	chtype* lastVisibleChar;
	chtype* scrollPos;

	Win() :
		bordWin(nullptr),
		textWin(nullptr),
		screenBuf(),
		firstVisibleChar(&screenBuf),
		lastVisibleChar(&screenBuf),
		scrollPos(&screenBuf) {}

};

struct UiContext{
	Win* userWin;
	Win* msgWin;
	Win* inputWin;
	bool uiDisplayed;

	UiContext(Win* u, 
			  Win* m, 
			  Win* i) 
			  : 
			  userWin(u), 
			  msgWin(m), 
			  inputWin(i),
			  uiDisplayed(true) {}

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
int calcLineCount(vector<chtype> screenBuf, int lines, int cols);
void scrollUp(UiContext& context, int lines, int cols);
void scrollDown(UiContext& context, int lines, int cols);
void scrollBottom(UiContext& context, int lines, int cols);

// Scrolling and redrawing
void redrawInputWin(Win* window, int lines, int cols);
void redrawUserWin(Win* window, int lines, int cols);
void redrawMsgWin(Win* window, int lines, int cols);

void restoreStringToWin(Win* window);
void restoreTextToWin(Win* window);
