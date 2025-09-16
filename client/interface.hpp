#pragma once

#include <ncurses.h>
#include <form.h>
#include <menu.h>
#include "client.hpp"

#define MAX_MSG_BUF 500
#define PAD_BUF_MULTI 5
#define MENU_HEIGHT 20
#define MENU_WIDTH 40
#define FIELD_OFFSET 4
#define PORT_MAX 65535


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
		if (confMenu){
			unpost_menu(confMenu);
			free_menu(confMenu);
		}

		for (int i = 0; i < (int)myItems.size(); i++){
			if (myItems[i]){
				free_item(myItems[i]);
			}
		}

		if (confWin){
			delwin(confWin);
		}

		if (subWin){
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
		int ncols = NAMELEN + 1;
		int begY = VALIGN + rows/6;
		int begX = HALIGN;

		// Create underying formWin
		formWin = derwin(bordWin,
						nlines, ncols,
						begY, begX);

		for (int i = 0; i < (int)fieldNames.size(); i++){
			
			int desiredLen = NAMELEN;
			switch(i){
				case 0:
					desiredLen = NAMELEN - 1;
					break;

				case 1:
					desiredLen = 5;
					break;

				case 2:
					desiredLen = NAMELEN;
					break;
			}


			// Create box windows
			int relY = getpary(formWin);
			int relX = getparx(formWin);

			int height = 3;
			int width = desiredLen + 3;
			int startY = relY - 1 + (i * FIELD_OFFSET);
			int startX = relX - 1;

			WINDOW* boxWin = derwin(bordWin,
									height, width,
									startY, startX);

			fieldBoxes.push_back(boxWin);

			// Create new fields
			height = 1;
			width = desiredLen + 1;
			startY = i * FIELD_OFFSET;
			startX = 0;
			
			FIELD* newField = new_field(height, width,
										startY, startX,
										0, 0);

			switch(i){
				case 0: // IP
					break;

				case 1: // Port
					set_field_type(newField,
								   TYPE_INTEGER,
								   0,
								   0, PORT_MAX);
					break;

				case 2: // Name
					set_field_type(newField,
								   TYPE_ALNUM,
								   NAMELEN);
					break;
			}

			formFields.push_back(newField);
		}

		formFields.push_back(NULL);
	}

	void setForm(){
		// Set field options
		for (FIELD* f : formFields) {
			if (f){
				field_opts_off(f, O_AUTOSKIP); 
			}
		}

		// New form
		confForm = new_form(formFields.data());

		form_opts_off(confForm, O_BS_OVERLOAD);
		//form_opts_off(confForm, O_NL_OVERLOAD);

		// Set main window and sub window
		set_form_win(confForm, bordWin);
		set_form_sub(confForm, formWin);
	}

	bool validIpCh(int idx, int ch){
		return idx == 0 && (ch == '.' || (ch > 47 && ch < 58));
	}

	void handleInput(){

		// Set special keys
		keypad(bordWin, TRUE);

		int ch;
		while ((ch = wgetch(bordWin)) != '\n' && ch != KEY_ENTER && ch != '\r'){
			curs_set(1);
			switch(ch){
				case KEY_DOWN:
					form_driver(confForm, REQ_NEXT_FIELD);
					form_driver(confForm, REQ_END_LINE);
					break;

				case KEY_UP:
					form_driver(confForm, REQ_PREV_FIELD);
					form_driver(confForm, REQ_END_LINE);
					break;

				case '\t':
					form_driver(confForm, REQ_NEXT_FIELD);
					form_driver(confForm, REQ_END_LINE);
					break;

				case KEY_BACKSPACE:
				case 127:
				case '\b': 
					form_driver(confForm, REQ_DEL_PREV);
					break;

				default: {
						int rc = form_driver(confForm, REQ_NEXT_CHAR);
						if (rc != E_REQUEST_DENIED){
							form_driver(confForm, REQ_END_LINE);

							int idx = field_index(current_field(confForm)); 

							if (idx != 0 || validIpCh(idx, ch)){
								form_driver(confForm, ch);
							}
						}
					}
					break;
			}
		}
	}
	
	// Do this after posting form
	void refresh(){
		wrefresh(bordWin);

		int begY = getbegy(formWin);

		wrefresh(formWin);
		for (int i = 0; i < (int)fieldNames.size(); i++){
			
			begY += i * FIELD_OFFSET;

			// Prepend names before fields
			mvwprintw(bordWin,
					  begY, HALIGN,
					  fieldNames[i].c_str());

			box(fieldBoxes[i], 0, 0);
			wrefresh(fieldBoxes[i]);
		}
	}

	void freeAll(){
		if (confForm){
			unpost_form(confForm);
			free_form(confForm);
		}

		for (int i = 0; i < (int)formFields.size(); i++){
			free_field(formFields[i]);
		}

		for (int i = 0; i < (int)fieldBoxes.size(); i++){
			delwin(fieldBoxes[i]);
		}

		if (bordWin){
			delwin(bordWin);
		}

		if (formWin){
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

