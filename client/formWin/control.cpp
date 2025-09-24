#include "../interface.hpp"


int FormState::startUp(){

	vector<string> fieldNames {"IP:", "Port:", "Name:"};
	
	int minHeight = MENU_HEIGHT + MENU_HEIGHT/3 * (int)fieldNames.size();
	int minWidth = MENU_WIDTH;

	if (LINES < minHeight || COLS < minWidth){
		return -1;
	}

	// Define ui context
	string title = "| MultiChat |";
	string caption = "Input connection information";

	WINDOW* locFormWin = centerWin(NULL, 
								   title, caption, 
								   minHeight, minWidth);
	
	Form = make_unique<FormContext>(locFormWin, move(fieldNames));

	// Set up form
	Form->setForm();

	// Post and update screen
	post_form(Form->confForm);
	Form->refreshForm();

	return 0;
}

int FormState::running(){
	// Get selection or action
	Form->handleInput();
	return 0;
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
		return -2;
	}
}



void FormContext::refreshForm(){
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

