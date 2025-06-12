#include "client.hpp"


UiContext interfaceStart(){
	static Win users, input, messages;

	initscr();
	
	noecho();
	cbreak(); 
	curs_set(0);
	refresh();

	input = createInputWin();
	messages = createMsgWin();
	users = createUserWin();

	static UiContext uiContext = UiContext(&users, &messages, &input);

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


Win createUserWin() {
	int height = LINES;
	int width = COLS / 6;
	int startY = 0;
	int startX = 0;

	Win window;
	
	window.bordWin = createWindow(height, 
								  width, 
								  startY, 
								  startX, 
								  true, 
								  false);

	window.textWin = createWindow(height - (2*VALIGN), 
								  width - (2*HALIGN), 
								  startY + VALIGN, 
								  startX + HALIGN,
								  false,
								  true);

	return window;
}


Win createMsgWin(){
	int height = LINES - (LINES /6);
	int width = (COLS * 5) / 6;
	int startY = 0;
	int startX = COLS / 6;

	Win window;

	window.bordWin = createWindow(height, 
								  width, 
								  startY, 
								  startX,
								  true,
								  false);

	window.textWin = createWindow(height - (2*VALIGN), 
							width - (2*HALIGN), 
							startY + VALIGN, 
							startX + HALIGN,
							false,
							true);

	return window;
}

Win createInputWin(){
	int height = LINES / 6;
	int width = (COLS * 5) / 6;
	int startY = LINES - (LINES /6);
	int startX = COLS / 6;

	Win window;
	
	window.bordWin = createWindow(height, 
								  width, 
								  startY, 
								  startX,
								  true,
								  false);

	window.textWin = createWindow(height - (2*VALIGN), 
								  width - (2*HALIGN), 
								  startY + VALIGN, 
								  startX + HALIGN,
								  false,
								  true);

	nodelay(window.textWin, TRUE);
	keypad(window.textWin, TRUE);

	return window;
}

void handleCh(UiContext& context, int ch, int servFd){

	Win& inWin = *context.inputWin;
	WINDOW* inWindow = inWin.textWin;

	int row, col;
	getmaxyx(inWindow, row, col);

	curs_set(2); // Make the cursor visible

	size_t maxChar = min(MAXMSG, (row * col));

	int y, x; // Current y and x pos
	getyx(inWindow, y, x);

	// Handle Backspace
	if (ch == 127 || ch == KEY_DC || ch == 8 || ch == KEY_BACKSPACE){ 

		// Backspace
		if (inputBuf.size() <= 0){
			// If there is nothing, don't do anything
			return;
		}

		// Remove the last of the input
		inputBuf.pop_back();

		if (x <= 0){

			// If at left edge
			wmove(inWindow, y - 1, col - 1);
			waddch(inWindow, ' ');

			y--;
			wmove(inWindow, y, col - 1);
			x = col - 1;

		} else {
			// Normal deletion
			wmove(inWindow, y, x - 1);
			waddch(inWindow, ' ');
			wmove(inWindow, y, x - 1);
			x--;
		}
		wrefresh(inWindow);
	}

	// Handle enter
	if (ch == '\n'){
		if (inputBuf.empty()){
			return;
		}

		// Clear the window
		werase(inWindow);
		wmove(inWindow, 0, 0);
		wrefresh(inWindow);
		
		// Format string and output it
		vector<chtype> formatStr = formatMessage(inputBuf, clientInfo.username);
		appendToWindow(*context.msgWin, formatStr, 1);
		wrefresh(context.msgWin->textWin);

		// Append packet to writeBuf
		ClientBroadMsg pkt = ClientBroadMsg(inputBuf);
		pkt.serialize(writeBuf);

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

	if (inputBuf.size() >= maxChar){
		// Can't add no more
		return;

	} else if (x >= col - 1){

		// If hit the end of the line
		waddch(inWindow, ch);
		wmove(inWindow, y + 1, 0);
		x = 0;
		y++;

	} else {

		// Normal movement
		waddch(inWindow, ch);
		wmove(inWindow, y, x + 1);
		x++;
	}
	wrefresh(inWindow);

}

vector<chtype> formatMessage(string& message, string& username){
	vector<chtype> outBuf;

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

vector<chtype> formatDisMessage(string& username){
	vector<chtype> outBuf;

	pushBackStr("<--    ", outBuf, A_DIM);

	pushBackStr(username, outBuf, A_DIM);

	pushBackStr(" has disconnected    -->", outBuf, A_DIM);

	return outBuf;
}

vector<chtype> formatConMessage(string& username){
	vector<chtype> outBuf;

	pushBackStr("<--    ", outBuf, A_DIM);

	pushBackStr(username, outBuf, A_DIM);

	pushBackStr(" has connected    -->", outBuf, A_DIM);

	return outBuf;

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

	for (chtype& ch : chTypeVec){
        window.screenBuf.push_back(ch);
		waddch(win, ch);
    }

    wrefresh(win);
}

void updateUserWindow(UiContext& context){
	Win window = *context.userWin;

	WINDOW* win = window.textWin;

	werase(win);

	int row, col;
	getmaxyx(win, row, col);

	wmove(win, 0, 0); // Move to the top of the textWin
	curs_set(0);

	int y, x; // Current y and x pos

	getyx(win, y, x);


	// Scroll down
	wscrl(win, -1);

	for (string& str : userConns){
		for (char& ch : str){

			// If you get to the end of the screen
			if (x >= col - 1 || ch == '\0'){

				// Place char
				if (ch != '\0'){
					window.screenBuf.push_back(ch);
					waddch(win, ch);
				}

				// Go one row lower
				x = 0;
				y = row + 1;
				wmove(win, y, x);


			} else {

				// Normal placement
				window.screenBuf.push_back(ch);
				waddch(win, ch);
				wmove(win, y, x + 1);
				x++;
			}

		}
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
