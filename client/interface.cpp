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

	keypad(window.textWin, TRUE);
	nodelay(window.textWin, TRUE);

	return window;
}

vector<uint8_t> processOneChar(Win& window, UiContext& context, int ch, vector<uint8_t> outBuf){
	WINDOW* win = window.textWin;
	int row, col;
	getmaxyx(win, row, col);

	curs_set(2); // Make the cursor visible

	size_t maxChar = min(MAXMSG, (row * col));

	int y, x; // Current y and x pos
	getyx(win, y, x);

	// Hitting enter
	if (ch == '\n'){

		// Create packet
		ClientBroadMsg pkt = ClientBroadMsg(outBuf);

		// Lock write buffer and append message to the buffer
		unique_lock lock(writeMtx);

		// Write into the writeBuf
		pkt.serialize(writeBuf);

		// Unlock the writeBuf
		lock.unlock();

		writeCv.notify_one();

		// Create & append full message to message window
		Win* msgWin = context.msgWin;
		vector<chtype> message = formatMessage(outBuf, clientInfo.username.c_str());
		appendToWindow(*msgWin, message, 1);

		// Clear the buffer, window, and reset the cursor
		outBuf.clear();
		werase(win);
		wmove(win, 0, 0);

		// Refresh Windows
		wrefresh(win);


	// Backspace
	} else if (ch == 127 || ch == KEY_DC || ch == 8 || ch == KEY_BACKSPACE){ 

		// At the beginning
		if (outBuf.size() <= 0){
			return outBuf;
		}

		// Remove the last of the input
		outBuf.pop_back();

		if (x <= 0){
			// If at left edge
			wmove(win, y - 1, col - 1);
			waddch(win, ' ');

			y--;
			wmove(win, y, col - 1);

		} else {
			// Normal deletion
			wmove(win, y, x - 1);
			waddch(win, ' ');
			wmove(win, y, x - 1);
		}

	// Full
	} else if (outBuf.size() >= maxChar){
			return outBuf;

	// End of line
	} else if (x >= col - 1){

		// If hit the end of the line

		outBuf.push_back((uint8_t)ch);
		waddch(win, ch);
		wmove(win, y + 1, 0);

	// Normal placement
	} else {
		outBuf.push_back((uint8_t)ch);
		waddch(win, ch);
		wmove(win, y, x + 1);
	}

	wrefresh(win);
	return outBuf;
}

vector<chtype> formatMessage(vector<uint8_t> message, const char* username){
	vector<chtype> outBuf;

	outBuf.push_back((chtype)'[' | A_BOLD);

	// Append username to the beginning of the message in bold
	for (size_t i = 0; i < strlen(username); i++){
		outBuf.push_back((chtype)username[i] | A_BOLD);
	}

	outBuf.push_back((chtype)']' | A_BOLD);
	outBuf.push_back((chtype)' ');
	
	// Convert uint8_t message to chtype

	// No null byte
	for (size_t i = 0; i < message.size() - 1; i++){
		outBuf.push_back((chtype)username[i] | A_BOLD);
	}

	return outBuf;
}

void appendToWindow(Win& window, vector<chtype> inputVec, int prescroll){

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

	for (size_t i = 0; i < inputVec.size() - 1; i++){
		chtype ch = inputVec[i];
        window.screenBuf.push_back(ch);
		waddch(win, ch);
    }

    wrefresh(win);
}

void processRender(renderItem rItem){
	switch(rItem.rcode){
		case MESSAGE: {
			// Message recieved, updating message win
			updateMessageWindow(rItem);
			break;
		}

		case USERUPDATE: {
			// User list change recieved, updating userwin
			updateUserWindow(rItem);
			break;
		}

		case USERCONN: {
			// User list change recieved, informing message win
			updateUserConn(rItem);
			break;
		}

		case USERDISC: {
			// User list change recieved, informing message win
			updateUserDisc(rItem);
			break;
		}

		default:{
			cerr << "Unknown render code" << endl;
			break;
		}
	}
}

void updateUserWindow(renderItem rItem){
	// rItem should contain a sorted list of users

	Win window = *rItem.target;

	WINDOW* win = window.textWin;

	werase(win);

	int row, col;
	getmaxyx(win, row, col);

	wmove(win, 0, 0); // Move to the top of the textWin
	curs_set(0);

	int y, x; // Current y and x pos

	getyx(win, y, x);

	char bChar;
	for (size_t i = 0; i < rItem.data.size() - 1; i++){
		chtype ch = rItem.data[i];
		window.screenBuf.push_back(ch);

		// If you get to the end of the screen
		if (x >= col - 1 || (bChar = getBaseChar(ch)) == '\0'){

			// Place char
			if (bChar != '\0'){
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

	wrefresh(win);
}

// These all update the message window

void updateMessageWindow(renderItem rItem){
	appendToWindow(*rItem.target, rItem.data, 1);
}

void updateUserConn(renderItem rItem){
	appendToWindow(*rItem.target, rItem.data, 1);
}

void updateUserDisc(renderItem rItem){
	appendToWindow(*rItem.target, rItem.data, 1);
}

inline char getBaseChar(chtype ch){
	return (char)(ch & A_CHARTEXT);
}
