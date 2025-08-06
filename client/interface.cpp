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


	if (calcLineCount(window->screenBuf, window->textWin) <= textHeight){
		// If the text takes up the same # of lines, do a normal restore
		restoreTextToWin(window);
	} else {
		//restoreTextScrolled(UiContext, textHeight, textWidth);
	}
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

	if (ch == KEY_UP){
		scrollUp(context);
	}

	if (ch == KEY_DOWN){
		scrollDown(context);
	}

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
		appendMsgWin(context, formatStr);
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
	if (!context.uiDisplayed){
		return;
	}

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
		
		context.uiDisplayed = false;
		return;
	}

	redrawInputWin(context.inputWin, lines, cols);
	redrawUserWin(context.userWin, lines, cols);
	redrawMsgWin(context.msgWin, lines, cols);
	updateUserWindow(context);
	context.uiDisplayed = true;
}

void appendMsgWin(UiContext& context, vector<chtype> chTypeVec){
	if (!context.uiDisplayed){
		return;
	}

	Win& window = *(context.msgWin);
	WINDOW* win = window.textWin;

	// Mark beginning of line in mem
	window.screenBuf.push_back('\0');
	for (chtype& ch : chTypeVec){
		window.screenBuf.push_back(ch);
	}

	if (!context.alignedOnBottom){
		return;
	}

	int rows, cols;
	getmaxyx(win, rows, cols);

	curs_set(0);

	int visLines = calcLineCount(window.screenBuf, win);
	int addedLines = calcLineCount(chTypeVec, win);

	context.msgWin->lastVisChIdx = window.screenBuf.size() - 1 + chTypeVec.size();
	context.msgWin->firstVisChIdx = getTopCh(context.msgWin->lastVisChIdx, context.msgWin);


	// Adding to the bottom
	wscrl(win, 1);
	wmove(win, rows - 1, 0);

	for (chtype& ch : chTypeVec){
		waddch(win, ch);
	}

	wrefresh(win);
}

int calcLineCount(vector<chtype>& screenBuf, WINDOW* win){
	int lineCount = 0;
	int chCount = 0;

	int cols = getmaxx(win);
	
	for (chtype & ch : screenBuf){
		if (chCount >= cols || ch == '\0'){
			lineCount++;
			chCount = 0;
		} else {
			chCount++;
		}
	}

	return lineCount;
}

void scrollBottom(UiContext& context){
}

void restoreTextScrolled(UiContext& context){
}

void scrollUp(UiContext& context){
	WINDOW* win = context.msgWin->textWin;
	int firstLineIndex = context.msgWin->firstVisChIdx;

	int newLineIdx = prevLinesAway(firstLineIndex, context.msgWin, 1);

	if (newLineIdx < 0){
		return;
	}

	addLine(context, newLineIdx, -1);

	// Assign new first char
	context.msgWin->firstVisChIdx = newLineIdx;
	// Assign new last char
	context.msgWin->lastVisChIdx = getBotCh(newLineIdx, context.msgWin);

	wrefresh(win);
}

void scrollDown(UiContext& context){
	WINDOW* win = context.msgWin->textWin;
	int lastLineIndex = context.msgWin->lastVisChIdx;

	int newLineIdx = nextLinesAway(lastLineIndex, context.msgWin, 1);

	if (newLineIdx < 0){
		return;
	}

	addLine(context, newLineIdx, 1);

	// Assign new first char
	context.msgWin->firstVisChIdx = getTopCh(newLineIdx, context.msgWin);

	// Assign new last char
	context.msgWin->lastVisChIdx = newLineIdx;

	wrefresh(win);
}

int linesAbove(int pos, Win* win){
	// Gets line count above position
	int lineCount = -1;
	int chCount = 0;

	int cols = getmaxx(win->textWin);

	int begin = 0;
	for (int idx = begin; idx < pos; idx++){
		if (chCount >= cols || win->screenBuf[idx] == '\0'){
			lineCount++;
			chCount = 0;
		} else {
			chCount++;
		}
	}

	return lineCount;
}

int linesBelow(int pos, Win* win){
	// Gets line count below position
	int lineCount = 0;
	int chCount = 0;

	int cols = getmaxx(win->textWin);

	int end = win->screenBuf.size() - 1;
	for (int idx = end; idx > pos; idx--){
		if (chCount >= cols || win->screenBuf[idx] == '\0'){
			lineCount++;
			chCount = 0;
		} else {
			chCount++;
		}
	}

	return lineCount;
}


int prevLinesAway(int pos, Win* win, int lineOffset){
	// pos is always start of the line
	int lineCount = -1;
	int chCount = 0;

	int cols = getmaxx(win->textWin);

	int begin = 0;
	if (pos <= begin || lineOffset < 1){
		// No previous lines
		return -1;
	}

	for (int idx = pos - 1; idx > begin; idx--){
		if (chCount >= cols || win->screenBuf[idx] == '\0'){
			lineCount++;
			chCount = 0;
		} else {
			chCount++;
		}

		if (lineCount >= lineOffset){
			return idx;
		}
	}

	return -1;
}

int nextLinesAway(int pos, Win* win, int lineOffset){
	// pos is always start of the line
	int lineCount = -1;
	int chCount = 0;

	int cols = getmaxx(win->textWin);

	int end = win->screenBuf.size() - 1;
	if (pos >= end || lineOffset < 1){
		// No next lines
		return -1;
	}

	for (int idx = pos + 1; idx < (int)end; idx++){
		if (chCount >= cols || win->screenBuf[idx] == '\0'){
			lineCount++;
			chCount = 0;
		} else {
			chCount++;
		}

		if (lineCount >= lineOffset){
			return idx;
		}
	}

	return -1;
}

int getTopCh(int botChIdx, Win* win){
	int rows, cols;
	getmaxyx(win->textWin, rows, cols);

	int lineCount = 0;
	int chCount = 0;

	int begin = 0;
	int idx = botChIdx;

	for (; lineCount < rows && idx > begin; idx--){
		if (chCount >= cols || win->screenBuf[idx] == '\0'){
			lineCount++;
			chCount = 0;
		} else {
			chCount++;
		}
	}
	return idx;
}

int getBotCh(int topChIdx, Win* win){
	int rows, cols;
	getmaxyx(win->textWin, rows, cols);

	int lineCount = 0;
	int chCount = 0;

	int end = win->screenBuf.size() - 1;
	int idx = topChIdx;
	for (; lineCount <= rows && idx < end; idx++){
		if (chCount >= cols || win->screenBuf[idx] == '\0'){
			lineCount++;
			chCount = 0;
		} else {
			chCount++;
		}
	}

	if (lineCount == rows + 1 && idx != end){
		// If went past the screen, go back one
		--idx;
	}

	return idx;
}

void addLine(UiContext& context, int startPos, int dir){
	// Put string on the top from mem startPos
	// Returns end of the added line
	Win& window = *(context.msgWin);
	WINDOW* win = window.textWin;

	int end = window.screenBuf.size();

	int rows, cols;
	getmaxyx(win, rows, cols);

	curs_set(0);
	if (dir > 0){
		// Adding to the top of the screen
		wscrl(win, -1);
		wmove(win, 0, 0);
	} else {
		// Adding to the bottom of the screen
		wscrl(win, 1);
		wmove(win, rows - 1, 0);
	}

	if (window.screenBuf[startPos] == '\0'){
		++startPos;
	}

	int chCount = 0;
	for (int idx = startPos; idx < end; idx++){
		if (window.screenBuf[idx] == '\0' || chCount >= cols){
			return;
		}
		waddch(win, window.screenBuf[idx]);
		chCount++;
	}
	wrefresh(win);
}
