#include "interface.hpp"

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

int lineCount(const unique_ptr<formMsg>& formStr, int maxCols){
	int lines = 0;
	int headLen = formStr->header.size();
	int msgLen = formStr->message.size();

	if (headLen >= maxCols){
		lines = headLen / maxCols;
		headLen %= maxCols;
	}

	int lineWidth = maxCols - headLen;
	if (lineWidth <= 0){
		lineWidth = 1;
	}

	lines += (msgLen + lineWidth - 1) / lineWidth;

	return lines;
}

void ChatContext::scrollBottom(){//
	msgWin->cursOffset = msgWin->occLines;
	refreshFromCurs();
}

void ChatContext::scrollUp(){//
	int maxRows = getmaxy(msgWin->bordWin) - 2 * VALIGN;

	if (msgWin->cursOffset <= maxRows){
		return;

	} else {
		msgWin->cursOffset--;
		refreshFromCurs();
	}
}

void ChatContext::scrollDown(){//
	if (msgWin->occLines <= msgWin->cursOffset){
		return;
	}

	msgWin->cursOffset++;
	refreshFromCurs();
}

void refreshFromCurs(){//
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


