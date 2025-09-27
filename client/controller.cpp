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
}

void ContextController::rmServFd(int oldServFd){
	if (servFd == -1){
		return;
	}

	epollModify(epollFd, oldServFd, 0, EPOLL_CTL_DEL);
	close(oldServFd);
}

int ContextController::handleWinch(){
	for (;;){
		signalfd_siginfo si;

		ssize_t readBytes = read(sigFd, &si, sizeof(si));

		if (readBytes == -1){
			if (errno == EAGAIN){
				break;
			}

			if (errno == EINTR){
				continue;
			}
		}
		
		redrawUi();
	}
}

int ContextController::handleInput(int ch){
	int status;
	switch(state){

		case FILE_DETECT:
			status = fileState.handleInput(ch);
			
			break;

		case FORM_FILL:
			status = formState.handleInput(ch);
			
			break;

		case RECONNECT:
			status = reconState.handleInput(ch);
			
			break;

		case MESSENGING:
			status = chatState.handleInput(ch);
			
			break;

		default:
			winErrState.handleInput(ch);
	}

}

int ContextController::handleChar(){
	// If something happened w/ keyboard input
	int ch;
	curs_set(2); // Make the cursor visible
	int stateDec = -1;

	while ((ch = getch(stdscr)) != ERR){
		// Adjust so gives input to right state
		stateDec = handleInput(ch);
		if (stateDec != -1){
			break;
		}
	}
	return stateDec;
}

int ContextController::stateChange(uiEnum oldState, uiEnum newState){
	int status;
	switch(oldState){

		case FILE_DETECT:
			status = fileState.tearDown();
			
			break;

		case FORM_FILL:
			status = formState.tearDown();
			
			break;

		case RECONNECT:
			status = reconState.tearDown();
			
			break;

		case MESSENGING:
			status = chatState.tearDown();
			
			break;

		default:
			// WinErr
			status = winErrState.tearDown();
	}

	if (status < 0){
		// Default back to what we know is good
		stateChange(-1, oldState);
		return -1;
	}

	erase();
	refresh();

	switch(newState){

		case FILE_DETECT:
			status = fileState.startUp();
			
			break;

		case FORM_FILL:
			status = formState.startUp();
			
			break;

		case RECONNECT:
			status = reconState.startUp();
			
			break;

		case MESSENGING:
			// Make sure to get/reassign servFd

			chatState.Chat->servFdStart();
			servFd = chatState.Chat->servFd;

			if (servFd < 0){
				status = -1;
				break;
			}

			status = chatState.startUp();
			
			break;

		default:
			// WinErr
			winErrState.startUp();

	}

	if (status < 0){
		// Default back to what we know is good
		stateChange(-1, oldState);
		return -1;
	}

	prevState = oldState;
	state = newState;
	return 0;
}


int ContextController::handleServFd(uint32_t event){
	// If something happened w/ serverfd
	if (event & (EPOLLHUP | EPOLLERR)){
		// Server dying event
		return;
	}

	if (event & EPOLLOUT){
		// Write event
		handleWrite();
	}

	if (event & EPOLLIN){
		// Read event
		handleRead();
	}
}

void ContextController::switchStates(int status){
	// Switches states based on the return status of state

}

void ContextController::controlEpoll(){
	while (1){
        int eventCount = epoll_wait(epollFd, events, MAX_EVENTS, -1);


        if (eventCount == -1) {
            if (errno == EINTR) {
				continue;
            } else {
				return -2;
            }
            continue;
        }


		for (int i = 0; i < eventCount; i++){

			int fd = events[i].data.fd;
			uint32_t event = events[i].events;

			if (fd == servFd && state == MESSENGING){
				handleServFd(event);

			} else if (fd == STDIN_FILENO){
				int status = handleChar();
				if (status != -1){
					switchStates(status);
				}
			} else {
				handleWinch();

			}
		}
	}
}
