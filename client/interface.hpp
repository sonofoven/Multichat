#pragma once

#include <ncurses.h>
#include <form.h>
#include <menu.h>
#include "client.hpp"

#define MAX_MSG_BUF 500
#define PAD_BUF_MULTI 5
#define MENU_HEIGHT 10
#define MENU_WIDTH 40
#define FIELD_OFFSET 3


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

		int n = (int)msgBuf.size(); int start = (n == MAX_MSG_BUF) ? writeIdx : 0;

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

struct MenuContext{
	WINDOW* confWin = NULL;
	WINDOW* subWin = NULL;
	MENU* confMenu = NULL;
	vector<string> choices;
	vector<ITEM*> myItems;

	MenuContext(WINDOW* w, 
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
		if (!confMenu){
			unpost_menu(confMenu);
			free_menu(confMenu);
		}

		for (int i = 0; i < (int)myItems.size(); i++){
			if (!myItems[i]){
				free_item(myItems[i]);
			}
		}

		if (!confWin){
			delwin(confWin);
		}

		if (!subWin){
			delwin(subWin);
		}

	}
};

struct FormContext{
	WINDOW* bordWin = NULL;
	WINDOW* formWin = NULL;
	FORM* confForm = NULL;
	vector<string> fieldNames;
	vector<FIELD*> formFields;
	vector<WINDOW*> fieldBoxes;

	FormContext(WINDOW* w, 
				vector<string> f)
				:
				bordWin(w),
				fieldNames(move(f)){

		int fieldNum = (int)fieldNames.size();
		formFields.reserve(fieldNum);
		fieldBoxes.reserve(fieldNum);

		int rows, cols;
		getmaxyx(bordWin, rows, cols);

		int nlines = (rows*3)/4 - VALIGN;
		int ncols = NAMELEN;
		int begY = VALIGN + rows/4;
		int begX = HALIGN + cols/4;

		// Create underying formWin
		formWin = derwin(bordWin,
						nlines, ncols,
						begY, begX);

		for (int i = 0; i < (int)fieldNames.size(); i++){

			// Create new fields
			int height = 1;
			int width = NAMELEN;
			int startY = getbegy(formWin) + (i * FIELD_OFFSET);
			int startX = getbegx(formWin);
			
			FIELD* newField = new_field(height, width,
										startY, startX,
										0, 0);

			formFields.push_back(newField);

			// Create box windows
			height = height + 2;
			width = width + 2;
			startY = getbegy(formWin) - 1 + (i * FIELD_OFFSET);
			startX = getbegx(formWin) - 1;

			WINDOW* boxWin = derwin(bordWin,
									height, width,
									startY, startX);

			fieldBoxes.push_back(boxWin);
		}

		formFields.push_back(NULL);
	}

	void setForm(){
		// Set field options
		set_field_back(formFields[0], A_UNDERLINE);
		field_opts_off(formFields[0], O_AUTOSKIP);

		set_field_back(formFields[1], A_UNDERLINE); 
		field_opts_off(formFields[1], O_AUTOSKIP);
		
		set_field_back(formFields[2], A_UNDERLINE); 
		field_opts_off(formFields[2], O_AUTOSKIP);

		// New form
		confForm = new_form(formFields.data());


		/* Set main window and sub window */
		set_form_win(confForm, bordWin);
		set_form_sub(confForm, formWin);
	}

	void handleInput(){
		// Set special keys
		keypad(bordWin, TRUE);

		int ch;
		while ((ch = wgetch(bordWin)) != '\n'){
			switch(ch){
				case KEY_DOWN:
					form_driver(confForm, REQ_NEXT_FIELD);
					form_driver(confForm, REQ_END_LINE);
				case KEY_UP:
					form_driver(confForm, REQ_PREV_FIELD);
					form_driver(confForm, REQ_END_LINE);
				case '\b':
					form_driver(confForm, REQ_DEL_PREV);
				default:
					form_driver(confForm, ch);
			}
		}
	}
	
	// Do this after posting form
	void refresh(){
		wrefresh(bordWin);
		wrefresh(formWin);


		int begY = getbegy(formWin);

		for (int i = 0; i < (int)fieldNames.size(); i++){
			
			begY += i * FIELD_OFFSET;

			// Prepend names before fields
			mvwprintw(bordWin,
					  begY, HALIGN,
					  fieldNames[i].c_str());

			box(fieldBoxes[i], 0, 0);
		}
	}

	void freeAll(){
		if (!confForm){
			unpost_form(confForm);
			free_form(confForm);
		}

		for (int i = 0; i < (int)formFields.size(); i++){
			if (!formFields[i]){
				free_field(formFields[i]);
			}

			if (!fieldBoxes[i]){
				delwin(fieldBoxes[i]);
			}
		}

		if (!bordWin){
			delwin(bordWin);
		}

		if (!formWin){
			delwin(formWin);
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
int configForm();

// Menu
int menuSetup(string caption, vector<string> choices);
int configMenu();
int reconnectMenu();
WINDOW* centerWin(WINDOW* parent, string& title, string& caption, int height, int width);

