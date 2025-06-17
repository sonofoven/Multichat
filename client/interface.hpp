#pragma once

#include <ncurses.h>
#include <form.h>
#include <string>
#include <vector>
#include <ctime>
#include <iostream>
#include <sys/epoll.h>
#include "client.hpp"

using namespace std;

// Window/UI structs
struct Win{
	WINDOW* bordWin;		
	WINDOW* textWin;		 
	vector<chtype> screenBuf;

	Win() :
		bordWin(nullptr),
		textWin(nullptr),
		screenBuf() {}
};

struct UiContext{
	Win* userWin;
	Win* msgWin;
	Win* inputWin;

	UiContext(Win* u, 
			  Win* m, 
			  Win* i) 
			  : 
			  userWin(u), 
			  msgWin(m), 
			  inputWin(i) {}
};

// Setup
UiContext interfaceStart();
void setupForm();

// Formatting tools
void pushBackStr(string str, vector<chtype>& outBuf, attr_t attr);
vector<chtype> formatMessage(time_t time, string& message, string& username);
vector<chtype> formatDisMessage(time_t time, string& username);
vector<chtype> formatConMessage(time_t time, string& username);
string formatTime(time_t timestamp);
string dateStr(int day);


// Window creation
Win createUserWin();
Win createMsgWin();
Win createInputWin();
WINDOW* createWindow(int height, 
					 int width, 
					 int starty, 
					 int startx, 
					 bool boxOn, 
					 bool scroll);

// Window I/O
string getWindowInput(Win& window, UiContext& context);
void appendToWindow(Win& window, vector<chtype> inputVec, int prescroll);
void updateUserWindow(UiContext& context);
void handleCh(UiContext& context, int ch, int servFd);
inline char getBaseChar(chtype ch);
