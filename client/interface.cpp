#include "client.hpp"


UiContext interfaceStart(){
	static Win users, input, messages;

	initscr();
	cbreak(); 
	
	noecho();
	curs_set(0);
	refresh();
	timeout(33);

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

	return window;
}

vector<uint8_t> processOneChar(Win& window, UiContext& context, int ch, vector<uint8_t> outBuf){
	// Only work with the textWin (we don't care about no borders)
	WINDOW* win = window.textWin;
	int row, col;
	getmaxyx(win, row, col);

	curs_set(2); // Make the cursor visible

	size_t maxChar = min(MAXMSG, (row * col));

	int y, x; // Current y and x pos
	getyx(win, y, x);

	// Hitting enter
	if (ch == '\n'){

		outBuf.push_back((uint8_t)'\0');

		// Lock write buffer and append message to the buffer
		lock_guard lock(writeMtx);

		// Create packet
		ClientBroadMsg pkt = ClientBroadMsg(outBuf);

		// Write into the writeBuf
		pkt.serialize(writeBuf);

		// Create & append full message to message window
		WINDOW* msgWin = uiContext.msgWin
		string msgWinStr(output.begin(), output.end());
		appendToWindow(*msgWin, str, 0, 1);

		// Clear the buffer, window, and reset the cursor
		outBuf.clear();
		werase(win);
		wmove(win, 0, 0);

		// Refresh Windows
		wrefresh(win);

		// Unlock the writeBuf with the scope

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

void appendToWindow(Win& window, vector<uint8_t> inputVec, attr_t attributes, int prescroll){

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

	for (size_t i = 0, i < inputVec.size() - 1, i++){

        chtype attrCh = (chtype)inputVec[i] | attributes;
        window.screenBuf.push_back(attrCh);
		waddch(win, attrCh);
    }

    wrefresh(win);
}

void processRender(renderItem rItem, UiContext context){
	switch(rItem.rcode){
		case MESSAGE: {


			break;
		}

		case USERUPDATE: {
			updateUserWindow(rItem, context);

			break;
		}

		default:{
			cerr << "Unknown err code" << endl;
		}
	}
}



void updateUserWindow(WIN& window, renderItem rItem){
	lock_guard lock(userMtx);

	WINDOW* win = window.textWin;

	werase(win);

	int row, col;
	getmaxyx(win, row, col);

	wmove(win, 0, 0); // Move to the top of the textWin
	curs_set(0);

	int y, x; // Current y and x pos

	getyx(win, y, x);

	// Scroll down
	//wscrl(win, -1);

	for (size_t i = 0; i < rItem.data.size() - 1; i++){
		chtype ch = rItem.data[i];
		window.screenBuf.push_back(ch);
		//////HERE


	}


	for (string & str : userConns){

		for (char & ch : str){
			if (ch == '\0'){
				break;
			}

			// If you get to the end of the screen
			if (x >= col - 1){

				// Place char
				waddch(win, ch);

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
