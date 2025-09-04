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
	MsgWin* messages = createMsgWin(serverName, rows, cols);
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


MsgWin* createMsgWin(string title, int lines, int cols){
	int height = lines - (lines /6);
	int userWidth = cols/6;
	int width = cols - userWidth;
	int startY = 0;
	int startX = cols / 6;
	bool boxOn = true;
	bool scrollOn = true;

	MsgWin* window = new MsgWin();

	window->bordWin = createWindow(height, 
								  width, 
								  startY, 
								  startX,
								  boxOn, 
								  !scrollOn);

	window->textWin = newpad(INIT_PAD_HEIGHT, 
							 width - (2*HALIGN));

	title = "| "+ title + " |";
	int leftPadding = (width - title.length())/2;
	mvwprintw(window->bordWin, 0, leftPadding, title.c_str());

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
	werase(window->textWin);
	for (char & ch : inputBuf){
		waddch(window->textWin, (chtype)ch);
	}
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


void redrawMsgWin(UiContext& context, int lines, int cols){
	MsgWin& window = *(context.msgWin);

	delwin(window.bordWin);
	delwin(window.textWin);

	int height = lines - (lines /6);
	int userWidth = cols/6;
	int width = cols - userWidth;
	int startY = 0;
	int startX = cols / 6;
	bool boxOn = true;
	bool scrollOn = true;
	int padWidth = width - (2*HALIGN);

	window.bordWin = createWindow(height, 
								  width, 
								  startY, 
								  startX,
								  boxOn, 
								  !scrollOn);

	window.textWin = newpad(INIT_PAD_HEIGHT, 
							 padWidth);

	string title = "| "+ serverName + " |";
	int leftPadding = (width - title.length())/2;
	mvwprintw(window.bordWin, 0, leftPadding, title.c_str());
	wrefresh(window.bordWin);

	int tempCursIdx = window.cursIdx;

	window.cursOffset = 0;
	window.occLines = 0;

	for (int i = 0; i < (int)window.msgBuf.size(); i++){
		// Append
		appendMsgWin(context, window.msgBuf[i], true);
		if (i == tempCursIdx){
			window.cursOffset = window.occLines;
		}
	}

	// Refresh
	refreshFromCurs(context);
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
		unique_ptr<formMsg> msgPtr = formatMessage(time(NULL), inputBuf, clientInfo.username);
		appendMsgWin(context, msgPtr, false);

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

		scrollBottom(context);

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

unique_ptr<formMsg> formatMessage(time_t time, string& message, string& username){
	unique_ptr<formMsg> outMsg = make_unique<formMsg>();

	vector<chtype>& hdr = outMsg->header;
	vector<chtype>& msg = outMsg->message;

	pushBackStr(formatTime(time), hdr, A_DIM);

	hdr.push_back((chtype)' ' | A_BOLD);
	hdr.push_back((chtype)'[' | A_BOLD);

	// Append username to the beginning of the message in bold
	for (char& ch : username){
		hdr.push_back((chtype)ch | A_BOLD);
	}

	hdr.push_back((chtype)']' | A_BOLD);
	hdr.push_back((chtype)' ');

	// Convert uint8_t message to chtype
	for (char& ch : message){
		msg.push_back((chtype)ch);
	}

	return outMsg;
}

unique_ptr<formMsg> formatDisMessage(time_t time, string& username){
	unique_ptr<formMsg> outMsg = make_unique<formMsg>();

	vector<chtype>& hdr = outMsg->header;
	vector<chtype>& msg = outMsg->message;

	pushBackStr(formatTime(time), hdr, A_DIM);

	hdr.push_back((chtype)' ' | A_BOLD);

	pushBackStr("<---  ", msg, A_BOLD);

	pushBackStr(username, msg, A_BOLD);

	pushBackStr(" has disconnected  --->", msg, A_BOLD);

	return outMsg;
}

unique_ptr<formMsg> formatConMessage(time_t time, string& username){
	unique_ptr<formMsg> outMsg = make_unique<formMsg>();

	vector<chtype>& hdr = outMsg->header;
	vector<chtype>& msg = outMsg->message;
	pushBackStr(formatTime(time), hdr, A_DIM);

	hdr.push_back((chtype)' ' | A_BOLD);

	pushBackStr("<---  ", msg, A_BOLD);

	pushBackStr(username, msg, A_BOLD);

	pushBackStr(" has connected  --->", msg, A_BOLD);

	return outMsg;
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


inline char getBaseChar(chtype ch){
	return (char)(ch & A_CHARTEXT);
}

void pushBackStr(string str, vector<chtype>& outBuf, attr_t attr){
	for (char& ch : str){
		outBuf.push_back((chtype)ch | attr);
	}
}

void redrawUi(UiContext& context, int lines, int cols){
	erase();
	refresh();

	redrawInputWin(context.inputWin, lines, cols);
	redrawUserWin(context.userWin, lines, cols);
	redrawMsgWin(context, lines, cols);
	updateUserWindow(context);
	context.uiDisplayed = true;
}
