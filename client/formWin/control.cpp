#include "interface.hpp"

int configForm(){

	vector<string> fieldNames {"IP:", "Port:", "Name:"};
	
	int minHeight = MENU_HEIGHT + MENU_HEIGHT/3 * (int)fieldNames.size();
	int minWidth = MENU_WIDTH;

	if (LINES < minHeight || COLS < minWidth){
		string errMsg = "Window too small";
		mvprintw(LINES / 2, (COLS - (int)errMsg.size())/2, errMsg.c_str());
		return -1;
	}

	// Define ui context
	string title = "| MultiChat |";
	string caption = "Input connection information";

	WINDOW* locFormWin = centerWin(NULL, 
								   title, caption, 
								   minHeight, minWidth);
	
	FormContext context(locFormWin, move(fieldNames));

	// Set up form
	context.setForm();

	// Post and update screen
	post_form(context.confForm);
	context.refresh();

	context.handleInput();

	int status = context.updateFile();

	context.freeAll();

	return status;
}

void FormContext::refresh(){
	wrefresh(bordWin);

	int begY = getpary(formWin);
	int startY = 0;

	wrefresh(formWin);

	for (int i = 0; i < (int)fieldNames.size(); i++){
		startY = begY + i * FIELD_OFFSET;

		// Prepend names before fields
		mvwprintw(bordWin,
				  startY, HALIGN*3,
				  fieldNames[i].c_str());

		box(fieldBoxes[i], 0, 0);
		wrefresh(fieldBoxes[i]);
	}
}

void FormContext::freeAll(){
	if (confForm){
		unpost_form(confForm);
		free_form(confForm);
	}

	for (int i = 0; i < (int)formFields.size(); i++){
		free_field(formFields[i]);
	}

	for (int i = 0; i < (int)fieldBoxes.size(); i++){
		delwin(fieldBoxes[i]);
	}

	if (bordWin){
		delwin(bordWin);
	}

	if (formWin){
		delwin(formWin);
	}

	erase();
	refresh();
}

