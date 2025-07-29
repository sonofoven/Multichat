#include "interface.hpp"

void interfaceStart(){
	initscr();
	noecho();
	cbreak(); 
	curs_set(0);
	refresh();
}

UiContext setupWindows(){
	int rows, cols;
	getmaxyx(stdscr, rows, cols);

	Win* input = createInputWin(rows, cols);
	Win* messages = createMsgWin(serverName, rows, cols);
	Win* users = createUserWin(rows, cols);

	UiContext uiContext = UiContext(users, messages, input);
	updateUserWindow(uiContext);

	return uiContext;
}

WINDOW* createWindow(int height, int width, int starty, int startx, bool boxOn, bool scroll){
	WINDOW* localWin;
	localWin = newwin(height, width, starty, startx);

	if (boxOn){
		box(localWin, 0, 0);
	}
	
	if (scroll){
		scrollok(localWin, TRUE);
		idlok(localWin, TRUE);
	}

	wrefresh(localWin);

	return localWin;
}


Win* createUserWin(int lines, int cols) {
	int height = lines;
	int width = cols / 6;
	int startY = 0;
	int startX = 0;
	bool boxOn = true;
	bool scrollOn = true;
	string title = "| Users |";

	Win* window = new Win();
	
	window->bordWin = createWindow(height, 
								  width, 
								  startY, 
								  startX, 
								  boxOn, 
								  !scrollOn);

	window->textWin = createWindow(height - (2*VALIGN), 
								  width - (2*HALIGN), 
								  startY + VALIGN, 
								  startX + HALIGN,
								  !boxOn,
								  scrollOn);

	
	int leftPad = (width - title.length())/2;
	mvwprintw(window->bordWin, 0, leftPad, title.c_str());

	wrefresh(window->bordWin);

	return window;
}


Win* createMsgWin(string title, int lines, int cols){
	int height = lines - (lines /6);
	int userWidth = cols/6;
	int width = cols - userWidth;
	int startY = 0;
	int startX = cols / 6;
	bool boxOn = true;
	bool scrollOn = true;

	Win* window = new Win();

	window->bordWin = createWindow(height, 
								  width, 
								  startY, 
								  startX,
								  boxOn, 
								  !scrollOn);

	window->textWin = createWindow(height - (2*VALIGN), 
							width - (2*HALIGN), 
							startY + VALIGN, 
							startX + HALIGN,
							!boxOn,
							scrollOn);

	int leftPad = (width - title.length())/2;
	title = "| "+ title + " |";
	mvwprintw(window->bordWin, 0, leftPad, title.c_str());

	wrefresh(window->bordWin);

	return window;
}

Win* createInputWin(int lines, int cols){
	int height = lines / 6;
	int userWidth = cols/6;
	int width = cols - userWidth;
	int startY = lines - (lines /6);
	int startX = cols / 6;
	bool boxOn = true;
	bool scrollOn = true;

	Win* window = new Win();
	
	window->bordWin = createWindow(height, 
								  width, 
								  startY, 
								  startX,
								  boxOn,
								  !scrollOn);

	window->textWin = createWindow(height - (2*VALIGN), 
								  width - (2*HALIGN), 
								  startY + VALIGN, 
								  startX + HALIGN,
								  !boxOn,
								  scrollOn);

	nodelay(window->textWin, TRUE);
	keypad(window->textWin, TRUE);

	return window;
}


void redrawInputWin(Win* window, int lines, int cols){
	delwin(window->bordWin);
	delwin(window->textWin);

	int userWidth = cols/6;
	int boxHeight = lines / 6;
	int boxWidth = cols-userWidth;
	int boxStartY = lines - boxHeight;
	int boxStartX = cols  / 6;

	int textHeight = boxHeight - (2 * VALIGN);
	int textWidth = boxWidth  - (2 * HALIGN);
	int textStartY = boxStartY + VALIGN;
	int textStartX = boxStartX + HALIGN;

	window->bordWin = createWindow(
		boxHeight, boxWidth, boxStartY, boxStartX,
		true, false
	);
	window->textWin = createWindow(
		textHeight, textWidth, textStartY, textStartX,
		false, true
	);


	nodelay(window->textWin, TRUE);
	keypad(window->textWin, TRUE);

	wrefresh(window->bordWin);
	// Append text to textWin
	restoreStringToWin(window);
	wrefresh(window->textWin);
}


void redrawUserWin(Win* window, int lines, int cols){
	delwin(window->bordWin);
	delwin(window->textWin);

	int boxHeight = lines;
	int boxWidth = cols / 6;
	int boxStartY = 0;
	int boxStartX = 0;

	int textHeight = boxHeight - (2 * VALIGN);
	int textWidth = boxWidth  - (2 * HALIGN);
	int textStartY = VALIGN;
	int textStartX = HALIGN;

	window->bordWin = createWindow(
		boxHeight, boxWidth, boxStartY, boxStartX,
		true, false
	);
	window->textWin = createWindow(
		textHeight, textWidth, textStartY, textStartX,
		false, true
	);


	string title = "| Users |";
	int leftPad = (boxWidth - title.length()) / 2;
	mvwprintw(window->bordWin, 0, leftPad, "%s", title.c_str());

	wrefresh(window->bordWin);

	// Append text to textWin
	wrefresh(window->textWin);
}


void redrawMsgWin(Win* window, int lines, int cols){
	delwin(window->bordWin);
	delwin(window->textWin);
	
	int userWidth = cols/6;
	int boxHeight = lines - (lines / 6);
	int boxWidth = cols-userWidth;
	int boxStartY = 0;
	int boxStartX = cols / 6;

	int textHeight = boxHeight - (2 * VALIGN);
	int textWidth = boxWidth  - (2 * HALIGN);
	int textStartY = VALIGN;
	int textStartX = boxStartX + HALIGN;

	window->bordWin = createWindow(
		boxHeight, boxWidth, boxStartY, boxStartX,
		true, false
	);
	window->textWin = createWindow(
		textHeight, textWidth, textStartY, textStartX,
		false, true
	);


	string title = "| " + serverName + " |";
	int leftPad = (boxWidth - title.length()) / 2;
	mvwprintw(window->bordWin, 0, leftPad, "%s", title.c_str());

	wrefresh(window->bordWin);

	restoreTextToWin(window);
	wrefresh(window->textWin);
}


void handleCh(UiContext& context, int ch, int servFd){
	refresh();

	Win& inWin = *context.inputWin;
	WINDOW* inWindow = inWin.textWin;


	int row, col;
	getmaxyx(inWindow, row, col);


	size_t maxChar = min(MAXMSG, (row * col) - 1);

	int y, x; // Current y and x pos
	getyx(inWindow, y, x);

	// Handle Backspace
	if (ch == KEY_BACKSPACE){ 

		// Backspace
		if (inputBuf.empty()){
			// If there is nothing, don't do anything
			return;
		}

		// Remove the last of the input
		inputBuf.pop_back();

		if (x == 0 && y > 0){
			// If at left edge
			y--;
			x = col - 1;

		} else {
			x--;
		}

		wmove(inWindow, y, x);

		wdelch(inWindow);
		wrefresh(inWindow);
		return;
	}

	// Handle enter
	if (ch == '\n' || ch == KEY_ENTER){
		if (inputBuf.empty()){
			return;
		}

		// Clear the window
		werase(inWindow);
		wmove(inWindow, 0, 0);
		wrefresh(inWindow);
		
		// Format string and output it
		vector<chtype> formatStr = formatMessage(time(NULL), inputBuf, clientInfo.username);
		appendToWindow(*context.msgWin, formatStr, 1);
		wrefresh(context.msgWin->textWin);

		// Append packet to writeBuf
		ClientBroadMsg pkt = ClientBroadMsg(inputBuf);
		pkt.serialize(writeBuf);

		// Send a modded packet to the log
		ServerBroadMsg logPkt = ServerBroadMsg(clientInfo.username, inputBuf);
		unique_ptr<ServerBroadMsg> upkt = make_unique<ServerBroadMsg>(logPkt);
		appendToLog(move(upkt));


		// If initial send didn't work, mark fd for output checking
		ssize_t sent = write(servFd, writeBuf.data(), writeBuf.size());
		if (sent > 0){
			writeBuf.erase(writeBuf.begin(), writeBuf.begin() + sent);	

		} else if (sent < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
			//Big error, handle
			perror("Init write");
			exit(1);
		}

		if (!writeBuf.empty()) {
			epollModify(epollFd, servFd, EPOLLIN|EPOLLOUT|EPOLLET|EPOLLHUP|EPOLLERR, EPOLL_CTL_MOD);
		}
		
		inputBuf.clear();

		return;
		
	}

	if (inputBuf.size() >= maxChar || (ch < 32 || ch >= 127)){
		// Can't add no more or not a real char
		return;

	} 

	if (x >= col - 1){

		// If hit the end of the line
		waddch(inWindow, ch);
		wmove(inWindow, y + 1, 0);
		inputBuf.push_back(ch);

	} else {

		// Normal movement
		waddch(inWindow, ch);
		wmove(inWindow, y, x + 1);
		inputBuf.push_back(ch);
	}

	wrefresh(inWindow);
	return;
}

vector<chtype> formatMessage(time_t time, string& message, string& username){
	vector<chtype> outBuf;

	pushBackStr(formatTime(time), outBuf, A_DIM);

	outBuf.push_back((chtype)' ' | A_BOLD);
	outBuf.push_back((chtype)'[' | A_BOLD);

	// Append username to the beginning of the message in bold
	for (char& ch : username){
		outBuf.push_back((chtype)ch | A_BOLD);
	}

	outBuf.push_back((chtype)']' | A_BOLD);
	outBuf.push_back((chtype)' ');
	
	// Convert uint8_t message to chtype

	for (char& ch : message){
		outBuf.push_back((chtype)ch);
	}

	return outBuf;
}

vector<chtype> formatDisMessage(time_t time, string& username){
	vector<chtype> outBuf;
	pushBackStr(formatTime(time), outBuf, A_DIM);

	outBuf.push_back((chtype)' ' | A_BOLD);
	pushBackStr("<--	", outBuf, A_DIM);

	pushBackStr(username, outBuf, A_DIM);

	pushBackStr(" has disconnected	-->", outBuf, A_DIM);

	return outBuf;
}

vector<chtype> formatConMessage(time_t time, string& username){
	vector<chtype> outBuf;
	pushBackStr(formatTime(time), outBuf, A_DIM);

	outBuf.push_back((chtype)' ' | A_BOLD);
	pushBackStr("<--	", outBuf, A_DIM);

	pushBackStr(username, outBuf, A_DIM);

	pushBackStr(" has connected	-->", outBuf, A_DIM);

	return outBuf;

}

string formatTime(time_t timestamp){
	tm stampTime = *localtime(&timestamp);
	
	time_t curTime = time(NULL);
	tm currentTime = *localtime(&curTime);

	char outBuf[64];
	string str;

	if (currentTime.tm_year != stampTime.tm_year){
		// Different year
		strftime(outBuf, sizeof(outBuf), "%b %Y", &stampTime);
		str += outBuf;

	} else if (currentTime.tm_mon != stampTime.tm_mon){
		// Different month
		strftime(outBuf, sizeof(outBuf), "%b ", &stampTime);
		str += outBuf + to_string(stampTime.tm_mday) + dateStr(stampTime.tm_mday);

	} else if (currentTime.tm_mday != stampTime.tm_mday && (currentTime.tm_mday - stampTime.tm_mday) > 6){
		// Different day, different week
		str += to_string(stampTime.tm_mday) + dateStr(stampTime.tm_mday);
		strftime(outBuf, sizeof(outBuf), " %I:%M%p", &stampTime);
		str += outBuf;

	} else if (currentTime.tm_mday != stampTime.tm_mday){
		// Different day, same week
		strftime(outBuf, sizeof(outBuf), "%a %I:%M%p", &stampTime);
		str += outBuf;

	} else {
		// Same day
		strftime(outBuf, sizeof(outBuf), "%I:%M%p", &stampTime);
		str += outBuf;
	}
	
	return str;

}

string dateStr(int day){
	string ending;

	if (day % 100 >= 11 && day % 100 <= 13){
		ending = "th";
	} else {
		switch (day % 10){
			case 1:
				ending = "st";
				break;
			case 2:
				ending = "nd";
				break;
			case 3:
				ending = "rd";
				break;
			default:
				ending = "th";
		}
	}
	return ending;
}



void appendToWindow(Win& window, vector<chtype> chTypeVec, int prescroll){

	WINDOW* win = window.textWin;

	int row, col;
	getmaxyx(win, row, col);

	curs_set(0);

	// Scroll an amount before adding
	wscrl(win, prescroll);

	if (prescroll > 0){
		// Adding to the bottom
		wmove(win, row - 1, 0);

	} else if (prescroll < 0){
		// Adding to the top of win
		wmove(win, 0, 0);

	} else {
		// Adding to the end of what I already have
	}

	// Mark beginning of line
	window.screenBuf.push_back('\0');

	for (chtype& ch : chTypeVec){
		window.screenBuf.push_back(ch);
		waddch(win, ch);
	}

	wrefresh(win);
}

void restoreTextToWin(Win* window){
	
	int row, col;
	getmaxyx(window->textWin, row, col);

	for (chtype & ch : window->screenBuf){
		if (ch == '\0'){
			wscrl(window->textWin, 1);
			wmove(window->textWin, row - 1, 0);
		} else {
			waddch(window->textWin, ch);
		}
	}
}

void restoreStringToWin(Win* window){
	werase(window->textWin);
	for (char & ch : inputBuf){
		waddch(window->textWin, (chtype)ch);
	}
}


void updateUserWindow(UiContext& context){
	WINDOW* win = context.userWin->textWin;

	werase(win);

	int row, col;
	getmaxyx(win, row, col);

	curs_set(0);

	int y = 0;

	for (const string& str : userConns){
		if (y >= row){
			break;
		}

		wattron(win, A_BOLD);
		mvwprintw(win, y++, 0, "[%s]", str.c_str());
		wattroff(win, A_BOLD);
	}
	wrefresh(win);
}

// These all update the message window

inline char getBaseChar(chtype ch){
	return (char)(ch & A_CHARTEXT);
}

void pushBackStr(string str, vector<chtype>& outBuf, attr_t attr){
	for(char& ch : str){
		outBuf.push_back((chtype)ch | attr);
	}
}

void redrawUi(UiContext& context, int lines, int cols){
	erase();
	refresh();

	if (lines < MIN_LINES || cols < MIN_COLS){
		// Display brief warning message
		const char* winErrMsg = "Window size is too small";
		const size_t strLength = strlen(winErrMsg);
		mvprintw((lines - strLength)/2, cols/2, winErrMsg);
		refresh();
		
		windowDisplayed = false;
		return;
	}

	redrawInputWin(context.inputWin, lines, cols);
	redrawUserWin(context.userWin, lines, cols);
	redrawMsgWin(context.msgWin, lines, cols);
	updateUserWindow(context);

	windowDisplayed = true;
}
