#include "interface.hpp"

void ChatContext::setupWindows(){
	int rows, cols;
	getmaxyx(stdscr, rows, cols);

	inputWin = createInputWin(rows, cols);
	msgWin = createMsgWin(serverName, rows, cols);
	userWin = createUserWin(rows, cols);

	updateUserWindow();
}

void ChatContext::appendMsgWin(unique_ptr<formMsg>& formStr, bool redraw){
	int maxCols = getmaxx(msgWin->textWin);
	int lineShift = lineCount(formStr, maxCols);

	curs_set(0);

	// If one message away from edge of pad
	if (msgWin->occLines + lineShift > getmaxy(msgWin->textWin) && !redraw){
		// Shift pad to save memory
		msgWin->shiftPad();
	}

	int row = getcury(msgWin->textWin-textWin);
	int col = getcurx(msgWin->textWin);

	// Adding to the bottom
	if (msgWin->occLines > 0 && col != 0){
		wmove(msgWin->textWin, row + 1, 0);
	} else {
		wmove(msgWin->textWin, row, 0);
	}

	int chCount = 0;
	int remainder = (int)formStr->header.size() % maxCols;
	int msgSpace = maxCols - remainder - 1;

	// Print out header
	for (const chtype& ch : formStr->header){
		waddch(msgWin->textWin, ch);
	}


	// Print out message justified w/ header
	for (const chtype& ch : formStr->message){
		if (chCount > msgSpace){
			for (int i = 0; i < ((int)formStr->header.size()); i++){
				waddch(msgWin->textWin, ' ');
			}
			chCount = 0;
		}
		waddch(msgWin->textWin, ch);
		chCount++;
	}

	bool atBottom = false;
	if (msgWin->occLines <= msgWin->cursOffset){
		atBottom = true;
	}

	msgWin->occLines += lineShift;

	// push message into history
	if (!redraw){
		msgWin->addMsg(move(formStr));
	}

	if (atBottom){
		msgWin->cursOffset = msgWin->occLines;
	}

	refreshFromCurs();
}

void ChatContext::handleCh(int ch){
	refresh();

	Win& inWin = *inputWin;
	WINDOW* inWindow = inWin.textWin;


	int row, col;
	getmaxyx(inWindow, row, col);


	size_t maxChar = min(MAXMSG, (row * col) - 1);

	int y, x; // Current y and x pos
	getyx(inWindow, y, x);

	if (ch == KEY_UP){
		scrollUp();
	}

	if (ch == KEY_DOWN){
		scrollDown();
	}

	// Handle Backspace
	if (ch == KEY_BACKSPACE){ 

		// Backspace
		if (inputBuf.empty()){
			// If there is nothing, don't do anything
			return;
		}

		// Remove the last of the input
		inputBuf.pop_back();

		if (x == 0 && y > 0){
			// If at left edge
			y--;
			x = col - 1;

		} else {
			x--;
		}

		wmove(inWindow, y, x);

		wdelch(inWindow);
		wrefresh(inWindow);
		return;
	}

	// Handle enter
	if (ch == '\n' || ch == KEY_ENTER){
		if (inputBuf.empty()){
			return;
		}

		// Clear the window
		werase(inWindow);
		wmove(inWindow, 0, 0);
		wrefresh(inWindow);
		
		// Format string and output it
		unique_ptr<formMsg> msgPtr = formatMessage(time(NULL), inputBuf, clientInfo.username);
		appendMsgWin(msgPtr, false);

		// Append packet to writeBuf
		ClientBroadMsg pkt = ClientBroadMsg(inputBuf);
		pkt.serialize(writeBuf);

		// Send a modded packet to the log
		ServerBroadMsg logPkt = ServerBroadMsg(clientInfo.username, inputBuf);
		unique_ptr<ServerBroadMsg> upkt = make_unique<ServerBroadMsg>(logPkt);
		appendToLog(move(upkt));


		// If initial send didn't work, mark fd for output checking
		ssize_t sent = write(servFd, writeBuf.data(), writeBuf.size());
		if (sent > 0){
			writeBuf.erase(writeBuf.begin(), writeBuf.begin() + sent);	

		} else if (sent < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
			//Big error, handle
			perror("Init write");
			exit(1);
		}

		if (!writeBuf.empty()) {
			epollModify(epollFd, servFd, EPOLLIN|EPOLLOUT|EPOLLET|EPOLLHUP|EPOLLERR, EPOLL_CTL_MOD);
		}
		
		inputBuf.clear();

		scrollBottom();

		return;
		
	}

	if (inputBuf.size() >= maxChar || (ch < 32 || ch >= 127)){
		// Can't add no more or not a real char
		return;

	} 

	if (x >= col - 1){

		// If hit the end of the line
		waddch(inWindow, ch);
		wmove(inWindow, y + 1, 0);
		inputBuf.push_back(ch);

	} else {

		// Normal movement
		waddch(inWindow, ch);
		wmove(inWindow, y, x + 1);
		inputBuf.push_back(ch);
	}

	wrefresh(inWindow);
	return;
}

void ChatContext::scrollBottom(){
	msgWin->cursOffset = msgWin->occLines;
	refreshFromCurs();
}

void ChatContext::scrollUp(){
	int maxRows = getmaxy(msgWin->bordWin) - 2 * VALIGN;

	if (msgWin->cursOffset <= maxRows){
		return;

	} else {
		msgWin->cursOffset--;
		refreshFromCurs();
	}
}

void ChatContext::scrollDown(){
	if (msgWin->occLines <= msgWin->cursOffset){
		return;
	}

	msgWin->cursOffset++;
	refreshFromCurs();
}

void ChatContext::replayMessages(){
	msgWin->cursOffset = 0;
	msgWin->occLines = 0;

	int n = (int)msgWin->msgBuf.size(); 
	int start = (n == MAX_MSG_BUF) ? msgWin->writeIdx : 0;

	for (int i = 0; i < n; i++){
		int idx = (start + i) % MAX_MSG_BUF;
		appendMsgWin(msgBuf[idx], true);
	}

	// Set to bottom
	msgWin->cursOffset = msgWin->occLines;
}

void ChatContext::refreshFromCurs(){
	// refresh the right amount
	// Get position of bordwin
	int maxRows, maxCols, starty, startx, winTopOffset, padTopOffset;

	maxRows = getmaxy(msgWin->bordWin) - 2 * VALIGN;
	maxCols = getmaxx(msgWin->bordWin) - 2 * HALIGN;
	getbegyx(msgWin->bordWin, starty, startx);

	winTopOffset = (maxRows - msgWin->occLines < 0 ? 0 : maxRows - msgWin->occLines); // Offset from top of win
	padTopOffset = msgWin->cursOffset - maxRows < 0 ? 0 : msgWin->cursOffset - maxRows;

	int pminrow = padTopOffset;
	int pmincol = 0;
	int sminrow = starty + VALIGN + winTopOffset;
	int smincol = startx + HALIGN;
	int smaxrow = starty + maxRows;
	int smaxcol = startx + maxCols + 1;

	prefresh(msgWin->textWin, 
			 pminrow, // Top Left Pad Y
			 pmincol, // Top Left Pad X
			 sminrow, // TLW Y
			 smincol, // TLW X
			 smaxrow, //BRW Y
			 smaxcol); //BRW X

}


void ChatContext::shiftPad(){
	// Clears pad and shifts everything to the top
	werase(msgWin->textWin);
	wmove(msgWin->textWin, 0, 0);
	wrefresh(msgWin->textWin);
	replayMessages();
}

void MsgWin::addMsg(unique_ptr<formMsg> formStr){
	if (writeIdx >= (int)msgBuf.size()){
		msgBuf.push_back(move(formStr));
	} else {
		msgBuf[writeIdx] = move(formStr);
	}

	writeIdx = (writeIdx + 1) % MAX_MSG_BUF;
}


int MsgWin::maxMsgLine(int maxCols){
	// Max lines a single message can be
	int lines = 0;
	int maxHeadLen = NAMELEN * 2;
	int maxMsgLen = MAXMSG;

	if (maxHeadLen >= maxCols){
		lines = maxHeadLen / maxCols;
		maxHeadLen %= maxCols;
	}

	int lineWidth = maxCols - maxHeadLen;
	if (lineWidth <= 0){
		lineWidth = 1;
	}

	lines += (maxMsgLen + lineWidth - 1) / lineWidth;

	return lines;
}

void ChatContext::updateUserWindow(){

	WINDOW* win = userWin->textWin;

	werase(win);

	int row, col;
	getmaxyx(win, row, col);

	curs_set(0);

	int y = 0;

	for (const string& str : userConns){
		if (y >= row){
			break;
		}

		wattron(win, A_BOLD);
		mvwprintw(win, y++, 0, "[%s]", str.c_str());
		wattroff(win, A_BOLD);
	}
	wrefresh(win);
}

