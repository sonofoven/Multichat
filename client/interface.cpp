#include "client.hpp"


void interfaceStart(){
	WINDOW *users, *input, *messages;
	int ch;

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

	mvwprintw(users, VALIGN, HALIGN, "Users go here");
	wrefresh(users);

	mvwprintw(messages, VALIGN, HALIGN, "Messages go here");
	wrefresh(messages);

	getWindowInput(input);

	getWindowInput(input);

	getch();
	endwin();
}

WINDOW* createWindow(int height, int width, int starty, int startx){
	WINDOW* localWin;
	localWin = newwin(height, width, starty, startx);

	box(localWin, 0, 0);
	wrefresh(localWin);

	return localWin;
}


WINDOW* createLeftWin() {
	int height = LINES;
	int width = COLS / 6;
	int startY = 0;
	int startX = 0;
	
	return createWindow(height, width, startY, startX);
}


WINDOW* createTopWin(){
	int height = LINES - (LINES /6);
	int width = COLS * (5/6);
	int startY = 0;
	int startX = COLS / 6;

	return createWindow(height, width, startY, startX);
}

WINDOW* createBotWin(){
	int height = LINES / 6;
	int width = COLS * (5/6);
	int startY = LINES - (LINES /6);
	int startX = COLS / 6;

	return createWindow(height, width, startY, startX);
}

vector<uint8_t> getWindowInput(WINDOW* win){
	int row, col;
	getmaxyx(win, row, col);

	vector<uint8_t> outBuf;

	wmove(win, VALIGN, HALIGN); // Move to the beginning of the window
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

			outBuf.pop_back();

			if (x <= HALIGN){

				// If at left edge
				wmove(win, y - 1, col - HALIGN - 1);
				waddch(win, ' ');

				y--;
				wmove(win, y, col - HALIGN - 1);
				x = col - HALIGN - 1;

			} else {
				// Normal deletion
				wmove(win, y, x - 1);
				waddch(win, ' ');
				wmove(win, y, x - 1);
				x--;
			}

			continue;
		}

		if (x >= col - HALIGN - 1){

			// If hit the end of the line
			waddch(win, ch);
			wmove(win, y + 1, HALIGN);
			x = HALIGN;
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

	werase(win);
	box(win, 0, 0);
	wmove(win, VALIGN, HALIGN);

	return outBuf;
}
