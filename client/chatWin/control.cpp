#include "../interface.hpp"

int ChatState::startUp(){

	return Chat->startProcess();
}

int ChatState::handleInput(int ch){
	Chat->handleCh(ch);
	return -1;
}

int ChatState::tearDown(){
	
	Chat->termProcess();
	Chat.reset();
	return 0;
}

int ChatContext::startProcess(){
	// Store connection info on successful connection
	FormContext::fileCreate();

	// Setup logging
	startLog();

	// Setup windows
	setupWindows();

	restoreHistory();

	return 0;
}

int ChatContext::termProcess(){
	stopLog();
	// These will eventually be handled in epoll loop
	freeAll();
	//endwin();
	return 0;
}

void ChatContext::freeAll(){
	if (userWin){
		userWin->freeWin();
	}

	if (msgWin){
		msgWin->freeWin();
	}

	if (inputWin){
		inputWin->freeWin();
	}

	delete userWin;
	delete msgWin;
	delete inputWin;

	erase();
	refresh();
}
