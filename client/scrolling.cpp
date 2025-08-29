#include "interface.hpp"

void appendMsgWin(UiContext& context, unique_ptr<formMsg> formStr){
	if (!context.uiDisplayed){
		return;
	}

	Win& window = *(context.msgWin);
	WINDOW* win = window.textWin;

	// Mark beginning of line in mem
	window.screenBuf.push_back('\0');
	for (chtype& ch : chTypeVec){
		window.screenBuf.push_back(ch);
	}

	if (!context.alignedOnBottom){
		return;
	}

	int rows, cols;
	getmaxyx(win, rows, cols);

	curs_set(0);

	context.msgWin->lastVisChIdx = window.screenBuf.size() - 1;
	context.msgWin->firstVisChIdx = getTopCh(context.msgWin->lastVisChIdx, context.msgWin);


	// Adding to the bottom
	wscrl(win, 1);
	wmove(win, rows - 1, 0);

	for (chtype& ch : chTypeVec){
		waddch(win, ch);
	}

	wrefresh(win);
}

void scrollBottom(UiContext& context){
}

void restoreTextScrolled(UiContext& context){
}

void scrollUp(UiContext& context){
	WINDOW* win = context.msgWin->textWin;
}

void scrollDown(UiContext& context){
	WINDOW* win = context.msgWin->textWin;
}
