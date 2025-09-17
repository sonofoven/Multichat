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
	
	void freeWin(){
		if (textWin){
			delwin(textWin);
		}

		if (bordWin){
			delwin(textWin);
		}
	}
};

void appendMsgWin(UiContext& context, unique_ptr<formMsg>& formStr, bool redraw);
string getFieldValue(FIELD* field);

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

struct ChatContext : ContextState{
	state = MESSENGING;

	Win* userWin;
	MsgWin* msgWin;
	Win* inputWin;

	list<string> userConns = {};
	vector<uint8_t> readBuf;
	connInfo clientInfo;
	
	vector<uint8_t> writeBuf;
	string inputBuf;
	string serverName; 
	
	shared_mutex fileMtx;

	mutex logMtx;
	queue<unique_ptr<Packet>> logQueue;
	condition_variable queueCv;
	thread logT;
	atomic<bool> stopLog = false;

	int servFd ;
	int epollFd = -1;
	epoll_event events[MAX_EVENTS];
	
	int startUp() override {

	}

	int running() override {

	}

	int tearDown() override {

	}

	int startProcess(){
		// Setup network connection
		setupFd();
		if (servFd < 0){
			return -1;
		}

		// Setup logging
		startLog();

		// Setup Epoll
		setupEpoll();
		if (epollFd == -1){
			return -1;
		}

		// Setup windows
		setupWindows();

		restoreHistory(this);

		modFds();
	}

	void modFds(){
		fcntl(servFd, F_SETFL, O_NONBLOCK);
		fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
		epollModify(epollFd, servFd, EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR, EPOLL_CTL_ADD);
		epollModify(epollFd, STDIN_FILENO, EPOLLIN | EPOLLET, EPOLL_CTL_ADD);
	}

	void setupWindows(){
		int rows, cols;
		getmaxyx(stdscr, rows, cols);

		inputWin = createInputWin(rows, cols);
		msgWin = createMsgWin(serverName, rows, cols);
		userWin = createUserWin(rows, cols);

		updateUserWindow(this);
	}

	void setupEpoll(){
		// Create our epoll instance
		epollFd = epoll_create1(0);
	}

	void startLog(){
		stopLog = false;
		logT(logLoop);
		logT.detach();
	}

	void stopLog(){
		stopLog = true;
		logT.join();
	}

	void setupFd(){
		int servFd = networkStart();
		if(servFd < 0){
			// Can't connect
			return;
		}

		// send validation
		sendOneConn(servFd);

		// wait for validation
		if (!recvOneVal(servFd)){
			servFd = -1;
		}
	}


	void freeAll(){
		if (userWin){
			userWin.freeWin();
		}

		if (msgWin){
			msgWin.freeWin();
		}

		if (inputWin){
			inputWin.freeWin();
		}
		erase();
		refresh();
	}

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

struct FileDetectMenu : ContextState{
	state = FILE_DETECT;

	int startUp() override {

	}

	int running() override {

	}

	int tearDown() override {

	}
}

struct ReconnectMenu : ContextState{
	state = RECONNECT;

	int startUp() override {

	}

	int running() override {

	}

	int tearDown() override {

	}
}

struct FormContext : ContextState{
	state = FORM_FILL;

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

		int nlines = (rows*3)/4 - VALIGN*2;
		int ncols = NAMELEN + 3;
		int begY = VALIGN*2 + FIELD_OFFSET;
		int begX = HALIGN + NAMELEN;

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

	int startUp() override {

	}

	int running() override {

	}

	int tearDown() override {

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
		form_driver(confForm, REQ_VALIDATION);
	}

	bool updateFile(){
		// Updates and checks file
		clientInfo.addr = getFieldValue(formFields[0]); 
		clientInfo.port = (uint16_t)stoi(getFieldValue(formFields[1])); 
		clientInfo.username = getFieldValue(formFields[2]); 

		if(!fileCreate()){
			return false;
		} else {
			return fileVerify();
		}

	}
	
	// Do this after posting form
	void refresh(){
		wrefresh(bordWin);

		int begY = getpary(formWin);
		int startY = 0;

		wrefresh(formWin);

		for (int i = 0; i < (int)fieldNames.size(); i++){
			startY = begY + i * FIELD_OFFSET;

			// Prepend names before fields
			mvwprintw(bordWin,
					  startY, HALIGN*3,
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

		erase();
		refresh();
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

