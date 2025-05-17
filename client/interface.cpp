#include "client.hpp"


void interfaceStart(){
	WIN users, input, messages;

	initscr();
	cbreak(); // Disable line buffering
			  // input is handled by me, not interrupt keys tho
	
	noecho();
	curs_set(0);
	refresh();

	input = createBotWin();
	messages = createTopWin();
	users = createLeftWin();

	wprintw(users.textWin, "Users go here");
	wrefresh(users.textWin);

	vector<uint8_t> grabbedText = getWindowInput(input.textWin);
	
	printToWindow(messages, grabbedText);	

	grabbedText = getWindowInput(input.textWin);
	
	printToWindow(messages, grabbedText);	

	grabbedText = getWindowInput(input.textWin);
	
	printToWindow(messages, grabbedText);	


	endwin();
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

void printToWindow(WIN window, vector<uint8_t> inputData){
	int row, col;
	getmaxyx(window.textWin, row, col);

	wmove(window.textWin, row - 1, 0); // Move to the bottom of the text window
	curs_set(0);

	int y, x; // Current y and x pos

	getyx(window.textWin, y, x);

	scrollok(window.textWin, TRUE);
	idlok(window.textWin, TRUE);

	// Scroll up
	wscrl(window.textWin, 1);

	for (uint8_t & ch : inputData){
		if (ch == '\0'){
			break;
		}

		// If you get to the end of the screen
		if (ch == '\n' || x >= col - 1){

			// Place char
			window.screenBuf.push_back(ch);
			if (ch != '\n'){
				waddch(window.textWin, (char)ch);
			}

			// Go to the bottom
			x = 0;
			y = row - 1;
			wmove(window.textWin, y, x);


		} else {

			// Normal placement
			window.screenBuf.push_back(ch);
			waddch(window.textWin, (char)ch);
			wmove(window.textWin, y, x + 1);
			x++;
		}
	}

	wrefresh(window.textWin);
}
