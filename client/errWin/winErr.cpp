#include "interface.hpp"

int WinErrState::startUp(){
	erase();
	refresh();

	string errMsg = "Window too small";
	mvprintw(LINES / 2, (COLS - (int)errMsg.size())/2, errMsg.c_str());
	return 0;
}

int WinErrState::running(){
	// Busy loop I know, juice is not worth the squeeze
	while (!redrawQueued){
		sleep_for(chrono::milliseconds(REDRAW_WAIT_MS);
	}
	return 0;
}

int WinErrState::tearDown(){
	erase();
	refresh();
	return 0;
}
