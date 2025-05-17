#include "client.hpp"


void interfaceStart(){
	WIN users, input, messages;

	initscr();
	cbreak(); // Disable line buffering
			  // input is handled by me, not interrupt keys tho
	
	keypad(stdscr, TRUE);
	noecho();
	curs_set(0);
	refresh();

	input = createBotWin();
	messages = createTopWin();
	users = createLeftWin();

	wprintw(messages.textWin, "Messages go here");
	wrefresh(messages.textWin);

	wprintw(users.textWin, "Users go here");
	wrefresh(users.textWin);


	getWindowInput(input.textWin);

	getWindowInput(input.textWin);

	getch();
	endwin();
}

// ADD A WINDOW WITHIN THE FIRST FOR TEXT

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

	wrefresh(window.textWin);

	return window;
}

vector<uint8_t> getWindowInput(WINDOW* win){
	int row, col;
	getmaxyx(win, row, col);

	vector<uint8_t> outBuf;

	wmove(win, 0, 0); // Move to the beginning of the window
	curs_set(2); // Make the cursor visible

	int ch; // Hold input one at a time
	int y, x; // Current y and x pos
	getyx(win, y, x);

	while((ch = wgetch(win)) != '\n'){

		if (ch == 127 || ch == KEY_DC){ 

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

		if (x >= col - 1){

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

void printToWindow(WINDOW* win, vector<uint8_t> inputData){
	int row, col;
	getmaxyx(win, row, col);


	wmove(win, VALIGN, HALIGN); // Move to the beginning of the window
	curs_set(0);

	int ch; // Hold input one at a time
	int y, x; // Current y and x pos

	getyx(win, y, x);

	for (uint8_t & ch : inputData){
		
	}

}
