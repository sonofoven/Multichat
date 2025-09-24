#include "../interface.hpp"

int WinErrState::startUp(){
	erase();
	refresh();

	string errMsg = "Window too small";
	mvprintw(LINES / 2, (COLS - (int)errMsg.size())/2, errMsg.c_str());
	return 0;
}

int handleInput(int ch){
	// Do nothing
	return 0;
}

int WinErrState::tearDown(){
	erase();
	refresh();
	return 0;
}
