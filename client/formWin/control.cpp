#include "../interface.hpp"


int FormState::startUp(){
	erase();
	vector<string> fieldNames {"IP:", "Port:", "Name:"};
	
	int minHeight = MENU_HEIGHT + MENU_HEIGHT/3 * (int)fieldNames.size();
	int minWidth = MENU_WIDTH;

	// Define ui context
	string title = "| MultiChat |";
	string caption = "Input connection information";

	WINDOW* locFormWin = centerWin(NULL, 
								   title, caption, 
								   minHeight, minWidth);
	
	Form = make_unique<FormContext>(locFormWin, move(fieldNames));


	refresh();

	// Set up form
	Form->setForm();
	keypad(stdscr, TRUE);

	// Post and update screen
	post_form(Form->confForm);

	Form->refreshForm();
	return 0;
}

int FormState::handleInput(int ch){
	// Get selection or action
	int retCh = Form->handleCh(ch);
	pos_form_cursor(Form->confForm);
	Form->refreshForm();
	return retCh;
}

int FormState::tearDown(){
	
	// Clear everything and revert back to nullptr
	Form->updateConnInfo();
	bool goodInfo = Form->checkCliInfo();

	Form->freeAll();
	Form.reset();

	if (goodInfo){
		return 0;
	} else {
		return -1;
	}
}



void FormContext::refreshForm(){

	int begY = getpary(formWin);
	int startY = 0;


	for (int i = 0; i < (int)fieldNames.size(); i++){
		startY = begY + i * FIELD_OFFSET;

		// Prepend names before fields
		mvwprintw(bordWin,
				  startY, HALIGN*3,
				  fieldNames[i].c_str());

		box(fieldBoxes[i], 0, 0);
		wrefresh(fieldBoxes[i]);
	}
	wrefresh(bordWin);
	pos_form_cursor(confForm);
	wrefresh(formWin);
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

