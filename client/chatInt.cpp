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

void ChatContext::modFds(){
	fcntl(servFd, F_SETFL, O_NONBLOCK);
	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
	epollModify(epollFd, servFd, EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR, EPOLL_CTL_ADD);
	epollModify(epollFd, STDIN_FILENO, EPOLLIN | EPOLLET, EPOLL_CTL_ADD);
}

void ChatContext::setupWindows(){
	int rows, cols;
	getmaxyx(stdscr, rows, cols);

	inputWin = createInputWin(rows, cols);
	msgWin = createMsgWin(serverName, rows, cols);
	userWin = createUserWin(rows, cols);

	updateUserWindow();
}

void ChatContext::setupEpoll(){
	// Create our epoll instance
	epollFd = epoll_create1(0);
}


void ChatContext::startLog(){
	stopLog = false;
	logT(logLoop);
	logT.detach();
}

void ChatContext::stopLog(){
	stopLog = true;
	logT.join();
}

void ChatContext::setupFd(){
	int servFd = networkStart();
	if(servFd < 0){
		// Can't connect
		return;
	}

	// send validation
	sendOneConn(servFd);

	// wait for validation
	if (!recvOneVal(servFd)){
		servFd = -1;
	}
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
	erase();
	refresh();
}

void ChatContext::restoreHistory(){ //logging
	// Find logs
	list<path> logFiles = detectLogFiles();

	if (logFiles.empty()){
		return;
	}

	// Sort them (reverse order)
	logFiles.sort(less<path>());

	shared_lock<shared_mutex> lock(fileMtx);
	// Iterate log file
	for (const path & logFile : logFiles){
		if (!exists(logFile)){
			continue;
		}

		ifstream log(logFile);
		if (!log.is_open()){
			continue;
		}

		string line;

		while (getline(log, line)){
			uint8_t* data = (uint8_t*)line.c_str();
			Packet* linePtr = instancePacketFromData(data);

			ServerBroadMsg servPacket = *(static_cast<ServerBroadMsg*>(linePtr));

			unique_ptr<formMsg> formattedStr = formatMessage(servPacket.timestamp, servPacket.msg, servPacket.username);
			appendMsgWin(formattedStr, false);
		}

		log.close();
	}
}

void ChatContext::appendMsgWin(unique_ptr<formMsg>& formStr, bool redraw){ // scrolling
	MsgWin& window = *msgWin;
	WINDOW* pad = window.textWin;

	int maxCols = getmaxx(context.msgWin->textWin);
	int lineShift = lineCount(formStr, maxCols);

	curs_set(0);

	// If one message away from edge of pad
	if (window.occLines + lineShift > getmaxy(window.textWin) && !redraw){
		// Shift pad to save memory
		window.shiftPad(context);
	}

	int row = getcury(pad);
	int col = getcurx(pad);

	// Adding to the bottom
	if (window.occLines > 0 && col != 0){
		wmove(pad, row + 1, 0);
	} else {
		wmove(pad, row, 0);
	}

	int chCount = 0;
	int remainder = (int)formStr->header.size() % maxCols;
	int msgSpace = maxCols - remainder - 1;

	// Print out header
	for (const chtype& ch : formStr->header){
		waddch(pad, ch);
	}


	// Print out message justified w/ header
	for (const chtype& ch : formStr->message){
		if (chCount > msgSpace){
			for (int i = 0; i < ((int)formStr->header.size()); i++){
				waddch(pad, ' ');
			}
			chCount = 0;
		}
		waddch(pad, ch);
		chCount++;
	}

	bool atBottom = false;
	if (window.occLines <= window.cursOffset){
		atBottom = true;
	}

	window.occLines += lineShift;

	// push message into history
	if (!redraw){
		window.addMsg(move(formStr));
	}

	if (atBottom){
		window.cursOffset = window.occLines;
	}

	refreshFromCurs(context);
}
