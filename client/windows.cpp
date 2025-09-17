#include "interface.hpp"


void Win::freeWin(){
	if (textWin){
		delwin(textWin);
	}

	if (bordWin){
		delwin(textWin);
	}
}

void MsgWin::addMsg(unique_ptr<formMsg> formStr){
	if (writeIdx >= (int)msgBuf.size()){
		msgBuf.push_back(move(formStr));
	} else {
		msgBuf[writeIdx] = move(formStr);
	}

	writeIdx = (writeIdx + 1) % MAX_MSG_BUF;
}

void MsgWin::replayMessages(ChatContext& context){
	cursOffset = 0;
	occLines = 0;

	int n = (int)msgBuf.size(); int start = (n == MAX_MSG_BUF) ? writeIdx : 0;

	for (int i = 0; i < n; i++){
		int idx = (start + i) % MAX_MSG_BUF;
		context.appendMsgWin(msgBuf[idx], true);
	}

	// Set to bottom
	cursOffset = occLines;
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

void shiftPad(ChatContext& context){
	// Clears pad and shifts everything to the top
	werase(textWin);
	wmove(textWin, 0, 0);
	wrefresh(textWin);
	replayMessages(context);
}
