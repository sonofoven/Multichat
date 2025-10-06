#include "../interface.hpp"

void ChatContext::redrawInputWin(int lines, int cols){
	Win* window = inputWin;
	delwin(window->bordWin);
	delwin(window->textWin);

	int userWidth = cols/6;
	int boxHeight = lines / 6;
	int boxWidth = cols-userWidth;
	int boxStartY = lines - boxHeight;
	int boxStartX = cols  / 6;

	int textHeight = boxHeight - (2 * VALIGN);
	int textWidth = boxWidth  - (2 * HALIGN);
	int textStartY = boxStartY + VALIGN;
	int textStartX = boxStartX + HALIGN;

	window->bordWin = createWindow(
		boxHeight, boxWidth, boxStartY, boxStartX,
		true, false
	);
	window->textWin = createWindow(
		textHeight, textWidth, textStartY, textStartX,
		false, true
	);


	nodelay(window->textWin, TRUE);
	keypad(window->textWin, TRUE);

	wrefresh(window->bordWin);
	// Append text to textWin
	werase(window->textWin);
	for (char & ch : inputBuf){
		waddch(window->textWin, (chtype)ch);
	}
	wrefresh(window->textWin);
}

void ChatContext::redrawUserWin(int lines, int cols){
	Win* window = userWin;
	delwin(window->bordWin);
	delwin(window->textWin);

	int boxHeight = lines;
	int boxWidth = cols / 6;
	int boxStartY = 0;
	int boxStartX = 0;

	int textHeight = boxHeight - (2 * VALIGN);
	int textWidth = boxWidth  - (2 * HALIGN);
	int textStartY = VALIGN;
	int textStartX = HALIGN;

	window->bordWin = createWindow(
		boxHeight, boxWidth, boxStartY, boxStartX,
		true, false
	);
	window->textWin = createWindow(
		textHeight, textWidth, textStartY, textStartX,
		false, true
	);


	string title = "| Users |";
	int leftPad = (boxWidth - title.length()) / 2;
	mvwprintw(window->bordWin, 0, leftPad, "%s", title.c_str());

	wrefresh(window->bordWin);
	wrefresh(window->textWin);
}


void ChatContext::redrawMsgWin(int lines, int cols){
	MsgWin& window = *msgWin;

	delwin(window.bordWin);
	delwin(window.textWin);

	int height = lines - (lines /6);
	int userWidth = cols/6;
	int width = cols - userWidth;
	int startY = 0;
	int startX = cols / 6;
	bool boxOn = true;
	bool scrollOn = true;

	window.bordWin = createWindow(height, 
								  width, 
								  startY, 
								  startX,
								  boxOn, 
								  !scrollOn);

	int padWidth = width - (2*HALIGN);
	int padHeight = PAD_BUF_MULTI * MAX_MSG_BUF;
	window.textWin = newpad(padHeight, 
							 padWidth);

	string title = "| "+ serverName + " |";
	int leftPadding = (width - title.length())/2;
	mvwprintw(window.bordWin, 0, leftPadding, title.c_str());
	wrefresh(window.bordWin);

	replayMessages();

	// Refresh
	refreshFromCurs();
}


void ChatContext::redrawChat(int lines, int cols){
	erase();
	refresh();

	redrawInputWin(lines, cols);
	redrawUserWin(lines, cols);
	redrawMsgWin(lines, cols);
	updateUserWindow();
}
