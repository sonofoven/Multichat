#include "interface.hpp"

void appendMsgWin(UiContext& context, vector<chtype> chTypeVec){
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

int calcLineCount(vector<chtype>& screenBuf, WINDOW* win){
	int lineCount = 0;
	int chCount = 0;

	int cols = getmaxx(win);
	
	for (chtype & ch : screenBuf){
		if (chCount >= cols || ch == '\0'){
			lineCount++;
			chCount = 0;
		} else {
			chCount++;
		}
	}

	return lineCount;
}

void scrollBottom(UiContext& context){
}

void restoreTextScrolled(UiContext& context){
}

void scrollUp(UiContext& context){
	WINDOW* win = context.msgWin->textWin;
	int firstLineIndex = context.msgWin->firstVisChIdx;

	int newLineIdx = prevLinesAway(firstLineIndex, context.msgWin, 1);

	curs_set(0);
	if (newLineIdx < 0){
		return;
	}

	addLine(context, newLineIdx, 1);

	// Assign new first char
	context.msgWin->firstVisChIdx = newLineIdx;
	// Assign new last char
	context.msgWin->lastVisChIdx = getBotCh(newLineIdx, context.msgWin);

	wrefresh(win);
}

void scrollDown(UiContext& context){
	WINDOW* win = context.msgWin->textWin;
	int lastLineIndex = context.msgWin->lastVisChIdx;

	int newLineIdx = nextLinesAway(lastLineIndex, context.msgWin, 1);

	curs_set(0);
	if (newLineIdx < 0){
		return;
	}

	addLine(context, newLineIdx, -1);

	// Assign new first char
	context.msgWin->firstVisChIdx = getTopCh(newLineIdx, context.msgWin);

	// Assign new last char
	context.msgWin->lastVisChIdx = newLineIdx;

	wrefresh(win);
}

int linesAbove(int pos, Win* win){
	// Gets line count above position
	int lineCount = -1;
	int chCount = 0;

	int cols = getmaxx(win->textWin);

	int begin = 0;
	for (int idx = begin; idx < pos; idx++){
		if (chCount >= cols || win->screenBuf[idx] == '\0'){
			lineCount++;
			chCount = 0;
		} else {
			chCount++;
		}
	}

	return lineCount;
}

int linesBelow(int pos, Win* win){
	// Gets line count below position
	int lineCount = 0;
	int chCount = 0;

	int cols = getmaxx(win->textWin);

	int end = win->screenBuf.size() - 1;
	for (int idx = end; idx > pos; idx--){
		if (chCount >= cols || win->screenBuf[idx] == '\0'){
			lineCount++;
			chCount = 0;
		} else {
			chCount++;
		}
	}

	return lineCount;
}


int prevLinesAway(int pos, Win* win, int lineOffset){
	int lineCount = -1;
	int chCount = 0;

	int cols = getmaxx(win->textWin);

	int begin = 0;
	if (pos <= begin || lineOffset < 1){
		// No previous lines
		return -1;
	}

	for (int idx = pos ; idx >= begin; idx--){
		if (chCount >= cols || win->screenBuf[idx] == '\0'){
			lineCount++;
			chCount = 0;
		} else {
			chCount++;
		}

		if (lineCount >= lineOffset){
			return idx;
		}
	}

	return -1;
}

int nextLinesAway(int pos, Win* win, int lineOffset){
	int lineCount = -1;
	int chCount = 0;

	int cols = getmaxx(win->textWin);

	int end = win->screenBuf.size() - 1;
	if (pos >= end || lineOffset < 1){
		// No next lines
		return -1;
	}

	for (int idx = pos; idx <= end; idx++){
		if (chCount >= cols || win->screenBuf[idx] == '\0'){
			lineCount++;
			chCount = 0;
		} else {
			chCount++;
		}

		if (lineCount >= lineOffset){
			return idx;
		}
	}

	return -1;
}

int getTopCh(int botChIdx, Win* win) {
	int rows = getmaxy(win->textWin);
	int topIdx = prevLinesAway(botChIdx, win, rows - 1);
	return (topIdx >= 0 ? topIdx : 0);
}

int getBotCh(int topChIdx, Win* win) {
	int rows = getmaxy(win->textWin);
	int botIdx = nextLinesAway(topChIdx, win, rows - 1);
	return (botIdx >= 0 ? botIdx : 0);
}

void addLine(UiContext& context, int startPos, int dir){
	// Put string on the top from mem startPos
	// Returns end of the added line
	Win& window = *(context.msgWin);
	WINDOW* win = window.textWin;

	int end = window.screenBuf.size();

	int rows, cols;
	getmaxyx(win, rows, cols);

	curs_set(0);
	if (dir > 0){
		// Adding to the top of the screen
		wscrl(win, -1);
		wmove(win, 0, 0);
	} else {
		// Adding to the bottom of the screen
		wscrl(win, 1);
		wmove(win, rows - 1, 0);
	}

	if (window.screenBuf[startPos] == '\0'){
		startPos++;
	}

	int chCount = 0;
	for (int idx = startPos; idx < end; idx++){
		if (window.screenBuf[idx] == '\0' || chCount >= cols){
			return;
		}
		waddch(win, window.screenBuf[idx]);
		chCount++;
	}
	wrefresh(win);
}
