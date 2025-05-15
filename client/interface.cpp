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

	mvwprintw(users, 1, 2, "Users go here");
	wrefresh(users);

	mvwprintw(input, 1, 2, "You type here");
	wrefresh(input);

	mvwprintw(messages, 1, 2, "Messages go here");
	wrefresh(messages);


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

	WINDOW* localWin;

	localWin = newwin(height, width, startY, startX);
	box(localWin, 0, 0);

	wrefresh(localWin);

	return localWin;
}


WINDOW* createTopWin(){
	int height = LINES - (LINES /6);
	int width = COLS * (5/6);
	int startY = 0;
	int startX = COLS / 6;

	WINDOW* localWin;

	localWin = newwin(height, width, startY, startX);
	box(localWin, 0, 0);

	wrefresh(localWin);

	return localWin;

}

WINDOW* createBotWin(){
	int height = LINES / 6;
	int width = COLS * (5/6);
	int startY = LINES - (LINES /6);
	int startX = COLS / 6;

	WINDOW* localWin;

	localWin = newwin(height, width, startY, startX);
	box(localWin, 0, 0);

	wrefresh(localWin);

	return localWin;
}
