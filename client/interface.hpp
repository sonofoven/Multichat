#pragma once

#include <ncurses.h>
#include <form.h>
#include <menu.h>
#include "client.hpp"

#define MAX_MSG_BUF 500
#define PAD_BUF_MULTI 5
#define MENU_HEIGHT 10
#define MENU_WIDTH 40


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

void appendMsgWin(UiContext& context, unique_ptr<formMsg>& formStr, bool redraw);

struct MsgWin : Win{
	vector<unique_ptr<formMsg>> msgBuf;
	int cursOffset; // Curs line offset from top
	int occLines;  // Occupied LINES total
	int writeIdx = 0; // Buffer handling

	MsgWin() :
		msgBuf(),
		cursOffset(0),
		occLines(0){
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
		int start = (n == MAX_MSG_BUF) ? writeIdx : 0;

		for (int i = 0; i < n; i++){
			int idx = (start + i) % MAX_MSG_BUF;
			appendMsgWin(context, msgBuf[idx], true);
		}

		// Set to bottom
		cursOffset = occLines;
	}

	int maxMsgLine(int maxCols){
		// Max lines a single message can be
		int lines = 0;
		int maxHeadLen = NAMELEN * 2;
		int maxMsgLen = MAXMSG;

		if (maxHeadLen >= maxCols){
			lines = maxHeadLen / maxCols;
			maxHeadLen %= maxCols;
		}

		int lineWidth = maxCols - maxHeadLen;
		if (lineWidth <= 0){
			lineWidth = 1;
		}


		lines += (maxMsgLen + lineWidth - 1) / lineWidth;

		return lines;
	}

	void shiftPad(UiContext& context){
		// Clears pad and shifts everything to the top
		werase(textWin);
		wmove(textWin, 0, 0);
		wrefresh(textWin);
		replayMessages(context);
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

struct ConfMenuContext{
	WINDOW* confWin = NULL;
	WINDOW* subWin = NULL;
	MENU* confMenu = NULL;
	vector<string> choices;
	vector<ITEM*> myItems;

	ConfMenuContext(WINDOW* w, 
					vector<string> c)
					:
					confWin(w),
					choices(move(c)){

						myItems.reserve(choices.size());

						int menuWidth = (int)choices.size();

						for (const string& choice : choices){
							myItems.push_back(new_item(choice.c_str(), choice.c_str()));
							menuWidth += choice.length();
						}

						confMenu = new_menu(myItems.data());

						int rows, cols;
						getmaxyx(confWin, rows, cols);

						int nlines = rows/2 - VALIGN;
						int ncols = menuWidth;
						int begY = VALIGN + rows/2;
						int begX = (cols - HALIGN)/2 - menuWidth/2;

						subWin = derwin(confWin,
										nlines, ncols,
										begY, begX);
					}

	void freeAll(){
		if (confMenu != NULL){
			unpost_menu(confMenu);
			free_menu(confMenu);
		}

		for (int i = 0; i < (int)choices.size(); i++){
			if (myItems[i] != NULL){
				free_item(myItems[i]);
			}
		}

		if (confWin != NULL){
			delwin(confWin);
		}

		if (subWin != NULL){
			delwin(subWin);
		}

	}
};

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
void configForm();

// Menu
bool configMenu();
bool reconnectMenu();
WINDOW* centerWin(WINDOW* parent, string& title, string& caption, int height, int width);

