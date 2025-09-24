#include "../interface.hpp"

int ChatState::startUp(){

	Chat = make_unique<ChatContext>();

	return Chat->startProcess();
}

int ChatState::handleInput(int ch){
	Chat->handleCh(ch);
	return 0;
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



//int ChatContext::runProcess(){
//	while (1){
//        int eventCount = epoll_wait(epollFd, events, MAX_EVENTS, -1);
//
//
//        if (eventCount == -1) {
//            if (errno == EINTR) {
//				continue;
//            } else {
//				return -2;
//            }
//            continue;
//        }
//
//
//		for (int i = 0; i < eventCount; i++){
//
//			int fd = events[i].data.fd;
//			uint32_t event = events[i].events;
//
//			if (fd == servFd){
//				// If something happened w/ serverfd
//				if (event & (EPOLLHUP | EPOLLERR)){
//					// Server dying event
//					return -2;
//				}
//
//				if (event & EPOLLOUT){
//					// Write event
//					handleWrite();
//				}
//
//				if (event & EPOLLIN){
//					// Read event
//					handleRead();
//				}
//
//			} else {
//				// If something happened w/ keyboard input
//				int ch;
//
//				curs_set(2); // Make the cursor visible
//				while ((ch = wgetch(inputWin->textWin)) != ERR){
//					handleCh(ch);
//				}
//			}
//		}
//	}
//}

int ChatContext::termProcess(){
	stopLog();
	// These will eventually be handled in epoll loop
	close(epollFd);
	close(servFd);
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
