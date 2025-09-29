#include "client.hpp"
#include "interface.hpp"

void ContextController::setupEpoll(){

	// Create our epoll instance
	epollFd = epoll_create1(0);

	// ADD SIGWINCH FD RIGHT HERE
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGWINCH);

	pthread_sigmask(SIG_BLOCK, &mask, NULL);

	winchFd = signalfd(-1, &mask, SFD_NONBLOCK);
	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

	epollModify(epollFd, winchFd, EPOLLIN | EPOLLET, EPOLL_CTL_ADD);
	epollModify(epollFd, STDIN_FILENO, EPOLLIN | EPOLLET, EPOLL_CTL_ADD);
}

void ContextController::addServFd(int newServFd){
	if (servFd != -1){
		rmServFd(servFd);
	}

	fcntl(newServFd, F_SETFL, O_NONBLOCK);
	epollModify(epollFd, newServFd, EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR, EPOLL_CTL_ADD);
	servFd = newServFd;
}

void ContextController::rmServFd(int oldServFd){
	if (servFd == -1){
		return;
	}

	epollModify(epollFd, oldServFd, 0, EPOLL_CTL_DEL);
	close(oldServFd);
	servFd = -1;
}

void ContextController::handleWinch(){
	for (;;){
		signalfd_siginfo si;

		ssize_t readBytes = read(winchFd, &si, sizeof(si));

		if (readBytes == -1){
			if (errno == EAGAIN){
				break;
			}

			if (errno == EINTR){
				break;
			}
		}
		
		redrawUi();
	}
}

int ContextController::handleChar(){
	// If something happened w/ keyboard input
	int ch;
	curs_set(2); // Make the cursor visible
	int stateDec = -1;

	while ((ch = getch()) != ERR){
		// Adjust so gives input to right state
		stateDec = curState->handleInput(ch);
		if (stateDec != -1){
			break;
		}
	}
	return stateDec;
}

void ContextController::stateChange(int status){
	// MAKE THIS FUNCTION A FUNC THAT INTERPRETS THE OUTPUT OF HANDLE CH
	// 0 means op 1
	// 1 means op 2

	switch (curState->state){
		case FILE_DETECT:
			switch(status){
				case 0:
					//endwin();
					//cout << clientInfo.addr << endl;
					//for(;;);
					curState->tearDown();
					switchIntoChat();
					return;
					break;

				default:
					curState->tearDown();
					curState = make_unique<FormState>();
					break;
			}
			break;


		case RECONNECT:
			switch(status){
				case 0:
					curState->tearDown();
					switchIntoChat();
					return;
					break;

				default:
					curState->tearDown();
					curState = make_unique<FormState>();
					break;
			}
			break;

		default:
			// Form fill
			switch(curState->tearDown()){
				case 0:
					switchIntoChat();
					return;
					break;

				default:
					// Error filling out data
					curState = make_unique<FormState>();
					break;
			}
			break;

		curState->startUp();
	}
}

void ContextController::switchIntoChat(){
	curState = make_unique<ChatState>();
	curState->startUp();

	ChatState* chatState = (ChatState*)(curState.get());
	chatState->Chat->servFdStart();
	if (servFd < 0){
		servFd = -1;
		curState = make_unique<ReconnectState>();
	} else {
		addServFd(servFd);
	}
}


int ContextController::handleServFd(uint32_t event){


	// If we have connection we know this is ChatState
	// If something happened w/ serverfd
	if (event & (EPOLLHUP | EPOLLERR)){
		// Server dying event
		// Change to reconnect state
		curState->tearDown();

		rmServFd(servFd);
		curState = make_unique<ReconnectState>();
		curState->startUp();
	}

	if (event & EPOLLOUT){
		// Write event
		ChatState* chatState = (ChatState*)(curState.get());
		chatState->Chat->handleWrite();
	}

	if (event & EPOLLIN){
		// Read event
		ChatState* chatState = (ChatState*)(curState.get());
		chatState->Chat->handleRead();
	}

	return 0;
}


void ContextController::controlEpoll(){
	while (1){
        int eventCount = epoll_wait(epollFd, events, MAX_EVENTS, -1);


        if (eventCount == -1) {
            if (errno == EINTR) {
				continue;
            } else {
				return;
            }
            continue;
        }


		for (int i = 0; i < eventCount; i++){

			int fd = events[i].data.fd;
			uint32_t event = events[i].events;

			if (fd == servFd && curState->state == MESSENGING){
				handleServFd(event);

			} else if (fd == STDIN_FILENO){
				int status = handleChar();
				if (status != -1){
					stateChange(status);
				}

			} else {
				handleWinch();
			}
		}
	}
}
