#include "client.hpp"


UiContext interfaceStart(){
	static WIN users, input, messages;
	static mutex ncursesMtx;

	initscr();
	cbreak(); // Disable line buffering
			  // input is handled by me, not interrupt keys tho
	
	noecho();
	curs_set(0);
	refresh();

	input = createBotWin();
	messages = createTopWin();
	users = createLeftWin();

	static UiContext uiContext = UiContext(&users, &messages, &input, ncursesMtx);

	return uiContext;
}

WINDOW* createWindow(int height, int width, int starty, int startx){
	WINDOW* localWin;
	localWin = newwin(height, width, starty, startx);

	box(localWin, 0, 0);
	wrefresh(localWin);

	return localWin;
}


WIN createLeftWin() {
	int height = LINES;
	int width = COLS / 6;
	int startY = 0;
	int startX = 0;

	WIN window;
	
	window.bordWin = createWindow(height, width, startY, startX);
	window.textWin = newwin(height - (2*VALIGN), width - (2*HALIGN), startY + VALIGN, startX + HALIGN);

	scrollok(window.textWin, TRUE);
	idlok(window.textWin, TRUE);

	wrefresh(window.bordWin);
	wrefresh(window.textWin);

	return window;
}


WIN createTopWin(){
	int height = LINES - (LINES /6);
	int width = (COLS * 5) / 6;
	int startY = 0;
	int startX = COLS / 6;

	WIN window;

	window.bordWin = createWindow(height, width, startY, startX);
	window.textWin = newwin(height - (2*VALIGN), width - (2*HALIGN), startY + VALIGN, startX + HALIGN);

	scrollok(window.textWin, TRUE);
	idlok(window.textWin, TRUE);

	wrefresh(window.bordWin);
	wrefresh(window.textWin);

	return window;
}

WIN createBotWin(){
	int height = LINES / 6;
	int width = (COLS * 5) / 6;
	int startY = LINES - (LINES /6);
	int startX = COLS / 6;

	WIN window;
	
	window.bordWin = createWindow(height, width, startY, startX);
	window.textWin = newwin(height - (2*VALIGN), width - (2*HALIGN), startY + VALIGN, startX + HALIGN);

	keypad(window.textWin, TRUE);

	wrefresh(window.bordWin);
	wrefresh(window.textWin);

	return window;
}

vector<uint8_t> getWindowInput(WIN& window, UiContext& context){
	// Only work with the textWin (we don't care about no borders)
	WINDOW* win = window.textWin;
	int row, col;
	getmaxyx(win, row, col);

	vector<uint8_t> outBuf;

	wmove(win, 0, 0); // Move to the beginning of the window
	curs_set(2); // Make the cursor visible

	size_t maxChar = min(MAXMSG, (row * col));

	int ch; // Hold input one at a time
	int y, x; // Current y and x pos
	getyx(win, y, x);


	while((ch = wgetch(win)) != '\n'){

		if (ch == 127 || ch == KEY_DC || ch == 8 || ch == KEY_BACKSPACE){ 

			// Backspace
			if (outBuf.size() <= 0){
				// If there is nothing, don't do anything
				continue;
			}

			// Remove the last of the input
			outBuf.pop_back();

			if (x <= 0){

				// If at left edge
				wmove(win, y - 1, col - 1);
				waddch(win, ' ');

				y--;
				wmove(win, y, col - 1);
				x = col - 1;

			} else {
				// Normal deletion
				wmove(win, y, x - 1);
				waddch(win, ' ');
				wmove(win, y, x - 1);
				x--;
			}

			continue;
		}



		if (outBuf.size() >= maxChar){
			// Can't add no more
			continue;

		} else if (x >= col - 1){

			// If hit the end of the line
			waddch(win, ch);
			wmove(win, y + 1, 0);
			x = 0;
			y++;

		} else {

			// Normal movement
			waddch(win, ch);
			wmove(win, y, x + 1);
			x++;
		}

		outBuf.push_back((uint8_t)ch);
	}

	outBuf.push_back((uint8_t)'\0');

	// Clear window and reset
	werase(win);
	wmove(win, 0, 0);
	wrefresh(win);

	return outBuf;
}

void appendToWindow(WIN& window, string& inputStr, attr_t attributes, int prescroll){

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

    for (char ch: inputStr) {
        chtype attrCh = (chtype)ch | attributes;
        window.screenBuf.push_back(attrCh);
        waddch(win, attrCh);
    }

    wrefresh(win);
}

void printToWindow(WIN& window, vector<uint8_t> inputData){
	string inputStr(inputData.begin(), inputData.end());
	appendToWindow(window, inputStr, 0, 1);
}


void updateUserWindow(WIN& window){

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

	for (string & str : userConns){
		for (char & ch : str){
			if (ch == '\0'){
				break;
			}

			// If you get to the end of the screen
			if (x >= col - 1){

				// Place char
				window.screenBuf.push_back(ch);
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
