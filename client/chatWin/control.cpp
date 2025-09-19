#include "interface.hpp"

int ChatContext::startProcess(){
	// Setup network connection
	setupFd();
	if (servFd < 0){
		return -1;
	}

	// Setup logging
	startLog();

	// Setup Epoll
	setupEpoll();
	if (epollFd == -1){
		return -1;
	}

	// Setup windows
	setupWindows();

	restoreHistory();

	modFds();
}

int ChatContext::runProcess(){
	while (1){
        int eventCount = epoll_wait(epollFd, events, MAX_EVENTS, -1);


        if (eventCount == -1) {
            if (errno == EINTR) {
				continue;
            } else {
				return -1;
            }
            continue;
        }


		for (int i = 0; i < eventCount; i++){

			int fd = events[i].data.fd;
			uint32_t event = events[i].events;

			if (fd == servFd){
				// If something happened w/ serverfd
				if (event & (EPOLLHUP | EPOLLERR)){
					// Server dying event
					return -1;
				}

				if (event & EPOLLOUT){
					// Write event
					handleWrite(servFd);
				}

				if (event & EPOLLIN){
					// Read event
					handleRead(servFd, uiContext);
				}

			} else {
				// If something happened w/ keyboard input
				int ch;

				curs_set(2); // Make the cursor visible
				while ((ch = wgetch(uiContext.inputWin->textWin)) != ERR){
					handleCh(uiContext, ch, servFd);
				}
			}
		}
	}
}

int ChatContext::termProcess(){
	stopLog();
	close(epollFd);
	close(servFd);
	freeAll();
	endwin();
}

void ChatContext::freeAll(){
	if (userWin){
		userWin.freeWin();
	}

	if (msgWin){
		msgWin.freeWin();
	}

	if (inputWin){
		inputWin.freeWin();
	}

	delete userWin;
	delete msgWin;
	delete inputWin;

	erase();
	refresh();
}
