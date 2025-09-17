#include "interface.hpp"

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

void scrollBottom(UiContext& context){
	MsgWin& window = *(context.msgWin);
	window.cursOffset = window.occLines;
	refreshFromCurs(context);
}

void scrollUp(UiContext& context){
	MsgWin& window = *(context.msgWin);

	int maxRows = getmaxy(context.msgWin->bordWin) - 2 * VALIGN;

	if (window.cursOffset <= maxRows){
		return;

	} else {
		window.cursOffset--;
		refreshFromCurs(context);
	}
}

void scrollDown(UiContext& context){
	MsgWin& window = *(context.msgWin);

	if (window.occLines <= window.cursOffset){
		return;
	}

	window.cursOffset++;
	refreshFromCurs(context);
}

void refreshFromCurs(UiContext& context){
	MsgWin& window = *(context.msgWin);
	WINDOW* pad = window.textWin;
	
	// refresh the right amount
	// Get position of bordwin
	int maxRows, maxCols, starty, startx, winTopOffset, padTopOffset;

	maxRows = getmaxy(context.msgWin->bordWin) - 2 * VALIGN;
	maxCols = getmaxx(context.msgWin->bordWin) - 2 * HALIGN;
	getbegyx(context.msgWin->bordWin, starty, startx);

	winTopOffset = (maxRows - window.occLines < 0 ? 0 : maxRows - window.occLines); // Offset from top of win
	padTopOffset = window.cursOffset - maxRows < 0 ? 0 : window.cursOffset - maxRows;

	int pminrow = padTopOffset;
	int pmincol = 0;
	int sminrow = starty + VALIGN + winTopOffset;
	int smincol = startx + HALIGN;
	int smaxrow = starty + maxRows;
	int smaxcol = startx + maxCols + 1;

	prefresh(pad, 
			 pminrow, // Top Left Pad Y
			 pmincol, // Top Left Pad X
			 sminrow, // TLW Y
			 smincol, // TLW X
			 smaxrow, //BRW Y
			 smaxcol); //BRW X

}


