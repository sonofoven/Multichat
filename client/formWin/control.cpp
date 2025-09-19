#include "interface.hpp"


FormContext::FormContext(WINDOW* w, 
						 vector<string> f):
						 bordWin(w), 
						 fieldNames(move(f)){

	int fieldNum = (int)fieldNames.size();
	formFields.reserve(fieldNum + 1);
	fieldBoxes.reserve(fieldNum);

	int rows, cols;
	getmaxyx(bordWin, rows, cols);

	int nlines = (rows*3)/4 - VALIGN*2;
	int ncols = NAMELEN + 3;
	int begY = VALIGN*2 + FIELD_OFFSET;
	int begX = HALIGN + NAMELEN;

	// Create underlying formWin
	formWin = derwin(bordWin,
					nlines, ncols,
					begY, begX);

	for (int i = 0; i < (int)fieldNames.size(); i++){
		
		int desiredLen = NAMELEN;
		switch(i){
			case 0:
				desiredLen = NAMELEN - 1;
				break;

			case 1:
				desiredLen = 5;
				break;

			case 2:
				desiredLen = NAMELEN;
				break;
		}


		// Create box windows
		int relY = getpary(formWin);
		int relX = getparx(formWin);

		int height = 3;
		int width = desiredLen + 3;
		int startY = relY - 1 + (i * FIELD_OFFSET);
		int startX = relX - 1;

		WINDOW* boxWin = derwin(bordWin,
								height, width,
								startY, startX);

		fieldBoxes.push_back(boxWin);

		// Create new fields
		height = 1;
		width = desiredLen + 1;
		startY = i * FIELD_OFFSET;
		startX = 0;
		
		FIELD* newField = new_field(height, width,
									startY, startX,
									0, 0);

		switch(i){
			case 0: // IP
				break;

			case 1: // Port
				set_field_type(newField,
							   TYPE_INTEGER,
							   0,
							   0, PORT_MAX);
				break;

			case 2: // Name
				set_field_type(newField,
							   TYPE_ALNUM,
							   NAMELEN);
				break;
		}

		formFields.push_back(newField);
	}

	formFields.push_back(NULL);
}

void FormContext::setForm(){
	// Set field options
	for (FIELD* f : formFields) {
		if (f){
			field_opts_off(f, O_AUTOSKIP); 
		}
	}

	// New form
	confForm = new_form(formFields.data());

	form_opts_off(confForm, O_BS_OVERLOAD);

	// Set main window and sub window
	set_form_win(confForm, bordWin);
	set_form_sub(confForm, formWin);
}

bool FormContext::validIpCh(int idx, int ch){
	// Checks to make sure that character is a num or decimal
	return idx == 0 && (ch == '.' || (ch > 47 && ch < 58));
}

void FormContext::handleInput(){
	// Set special keys
	keypad(bordWin, TRUE);

	int ch;
	while ((ch = wgetch(bordWin)) != '\n' && ch != KEY_ENTER && ch != '\r'){
		curs_set(1);
		switch(ch){
			case KEY_DOWN:
				form_driver(confForm, REQ_NEXT_FIELD);
				form_driver(confForm, REQ_END_LINE);
				break;

			case KEY_UP:
				form_driver(confForm, REQ_PREV_FIELD);
				form_driver(confForm, REQ_END_LINE);
				break;

			case '\t':
				form_driver(confForm, REQ_NEXT_FIELD);
				form_driver(confForm, REQ_END_LINE);
				break;

			case KEY_BACKSPACE:
			case 127:
			case '\b': 
				form_driver(confForm, REQ_DEL_PREV);
				break;

			default: {
					int rc = form_driver(confForm, REQ_NEXT_CHAR);
					if (rc != E_REQUEST_DENIED){
						form_driver(confForm, REQ_END_LINE);

						int idx = field_index(current_field(confForm)); 

						if (idx != 0 || validIpCh(idx, ch)){
							form_driver(confForm, ch);
						}
					}
				}
				break;
		}
	}
	form_driver(confForm, REQ_VALIDATION);
}

bool FormContext::updateFile(){
	// Updates and checks file
	clientInfo.addr = getFieldValue(formFields[0]); 
	clientInfo.port = (uint16_t)stoi(getFieldValue(formFields[1])); 
	clientInfo.username = getFieldValue(formFields[2]); 

	if(!fileCreate()){
		return false;
	} else {
		return fileVerify();
	}
}

string FormContext::getFieldValue(FIELD* field){
	// Trims off all the padded spaces and gets the plain value
	char* raw = field_buffer(field, 0);

	if (!raw){ 
		return "";
	}

	int idx = strlen(raw) - 1;

	while (idx >= 0 && 
		   isspace((int)raw[idx])){
		idx--;
	}

	if (idx < 0){
		return "";
	}

	return string(raw, idx + 1);
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

