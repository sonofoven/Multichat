#include "interface.hpp"

void appendMsgWin(UiContext& context, unique_ptr<formMsg> formStr){
	MsgWin& window = *(context.msgWin);
	WINDOW* pad = window.textWin;

	if (!context.uiDisplayed){
		window.msgBuf.push_back(move(formStr));
		return;
	}

	// Not focused on bottom
	if (!context.alignedOnBottom){
		window.msgBuf.push_back(move(formStr));
		return;
	}

	int maxCols, maxRows;
	int row = getcury(pad);
	getmaxyx(context.msgWin->bordWin, maxRows, maxCols);
	maxCols -= 1; // Account for border thickness
	maxRows -= 2;

	curs_set(0);

	// Adding to the bottom
	wmove(pad, row + 1, 0);

	int lineShift = lineCount(formStr, maxCols);
	int chCount = 0;
	int msgSpace = maxCols - (int)formStr->header.size() - (HALIGN * 2);

	//TRIM/SHIFT HERE

	// Print out header
	for (const chtype& ch : formStr->header){
		waddch(pad, ch);
	}

	// Print out message justified w/ header
	for (const chtype& ch : formStr->message){
		if (chCount > msgSpace){
			for (int i = 0; i < (int)formStr->header.size(); i++){
				waddch(pad, ' ');
			}
			chCount = 0;
		}
		waddch(pad, ch);
		chCount++;
	}

	// Set new bottom line
	//window.botIdx += 1;
	//window.cursIdx += 1;
	window.occLines += lineShift;

	// refresh the right amount
	// Get position of bordwin
	int starty, startx;
	getbegyx(context.msgWin->bordWin, starty, startx);

	int padTopOffset = (window.occLines - maxRows < 0 ? 0 : window.occLines - maxRows); // The top of the text in the pad
	int winTopOffset = (maxRows - window.occLines < 0 ? 0 : maxRows - window.occLines); // Where the pad should show up from the top of win

	prefresh(pad, 
			 padTopOffset + 1, // Top Left Pad Y
			 0, // Top Left Pad X
			 starty + VALIGN + winTopOffset, // TLW Y
			 startx + HALIGN, // TLW X
			 starty + maxRows, //BRW Y
			 startx + maxCols); //BRW X

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
	WINDOW* win = context.msgWin->textWin;
}

void scrollDown(UiContext& context){
	WINDOW* win = context.msgWin->textWin;
}
