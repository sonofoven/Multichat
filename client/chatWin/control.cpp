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

void ChatState::redraw(){
	Chat->redrawChat();
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
	// These will eventually be handled in epoll loop
	freeAll();
	//endwin();
	return 0;
}




void ChatContext::freeAll(){
	if (userWin){
		userWin->freeWin();
		delete userWin;
	}

	if (msgWin){
		msgWin->freeWin();
		delete msgWin;
	}

	if (inputWin){
		inputWin->freeWin();
		delete inputWin;
	}


	erase();
	refresh();
}
