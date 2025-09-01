#include "interface.hpp"

void appendMsgWin(UiContext& context, unique_ptr<formMsg> formStr){
	MsgWin& window = *(context.msgWin);
	WINDOW* pad = window.textWin;

	if (!context.uiDisplayed){
		window.msgBuf.push_back(move(formStr));
		return;
	}
	int maxCols;
	int row = getcury(pad);
	maxCols = getmaxx(context.msgWin->textWin) + 1;



	curs_set(0);

	// Adding to the bottom
	if (window.occLines > 0){
		wmove(pad, row + 1, 0);
	} else {
		wmove(pad, 0, 0);
	}

	int chCount = 0;
	int msgSpace = maxCols - ((int)formStr->header.size() + 1);
	int lineShift = lineCount(formStr, maxCols);

	//TRIM/SHIFT HERE along with ringbuf

	// Print out header
	for (const chtype& ch : formStr->header){
		waddch(pad, ch);
	}

	// Print out message justified w/ header
	for (const chtype& ch : formStr->message){
		if (chCount >= msgSpace){
			for (int i = 0; i < (int)formStr->header.size(); i++){
				waddch(pad, ' ');
			}
			chCount = 0;
		}
		waddch(pad, ch);
		chCount++;
	}


	// If cursor is aligned show bottom
	if (window.occLines == window.cursOffset){
		// Shift the cursor
		window.cursIdx += 1;
		window.cursOffset += lineShift;
		window.occLines += lineShift;

		refreshFromCurs(context);
	} else {
		window.occLines += lineShift;
	}

	// push message into history
	window.msgBuf.push_back(move(formStr));
}

int lineCount(const unique_ptr<formMsg>& formStr, int maxCols){
	int lineWidth = (maxCols - formStr->header.size());
	return (formStr->message.size() / lineWidth) + 1;
}

void scrollBottom(UiContext& context){
}

void scrollUp(UiContext& context){
	MsgWin& window = *(context.msgWin);
	WINDOW* pad = window.textWin;

	int maxRows = getmaxy(context.msgWin->bordWin) - 2 * VALIGN;

	if (window.cursOffset <= maxRows){
		return;
	}

	int maxCols = getmaxx(context.msgWin->textWin) + 1;
	int lineCnt = lineCount(window.msgBuf[window.cursIdx], maxCols);
	int lineShift = 0;

	if ((window.cursOffset - lineCnt) <= maxRows){
		lineShift = window.cursOffset - maxRows;
		window.shiftRemainder = lineCnt - lineShift;
	} else {
		lineShift = lineCnt;
	}

	window.cursIdx -= 1;
	window.cursOffset -= lineShift;

	refreshFromCurs(context);
}

void scrollDown(UiContext& context){
	MsgWin& window = *(context.msgWin);
	WINDOW* pad = window.textWin;

	if (window.occLines <= window.cursOffset){
		return;
	}

	int maxCols = getmaxx(context.msgWin->textWin) + 1;
	int lineCnt = lineCount(window.msgBuf[++window.cursIdx], maxCols);

	//window.cursIdx += 1;
	window.cursOffset += lineCnt + window.shiftRemainder;

	window.shiftRemainder = 0;

	if (window.cursOffset > window.occLines) {
		window.cursOffset = window.occLines;
	}

	refreshFromCurs(context);
}

void refreshFromCurs(UiContext& context){
	MsgWin& window = *(context.msgWin);
	WINDOW* pad = window.textWin;
	
	// refresh the right amount
	// Get position of bordwin
	int maxRows, maxCols, starty, startx, winTopOffset, padTopOffset;

	maxRows = getmaxy(context.msgWin->bordWin) - 2 * VALIGN;
	maxCols = getmaxx(context.msgWin->textWin) + 1;
	getbegyx(context.msgWin->bordWin, starty, startx);

	winTopOffset = (maxRows - window.occLines < 0 ? 0 : maxRows - window.occLines); // Offset from top of win
	padTopOffset = window.cursOffset - maxRows < 0 ? 0 : window.cursOffset - maxRows;

	prefresh(pad, 
			 padTopOffset, // Top Left Pad Y
			 0, // Top Left Pad X
			 starty + VALIGN + winTopOffset, // TLW Y
			 startx + HALIGN, // TLW X
			 starty + maxRows, //BRW Y
			 startx + maxCols); //BRW X
}
