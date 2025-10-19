#include "../interface.hpp"

FormContext::FormContext(WINDOW* w, vector<string> f):
						 bordWin(w), fieldNames(move(f)){

	int fieldNum = (int)fieldNames.size();
	formFields.reserve(fieldNum + 1);
	fieldBoxes.reserve(fieldNum);

	leaveok(bordWin, TRUE);	
	leaveok(formWin, FALSE);   

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
	int count = 0;
	for (FIELD* f : formFields) {
		
		if (f){
			count++;
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

int FormContext::handleCh(int ch){
	// Set special keys

	if (ch  != '\n' && ch != KEY_ENTER && ch != '\r'){
		curs_set(1);
		switch(ch){
			case KEY_DOWN:
				form_driver(confForm, REQ_NEXT_FIELD);
				form_driver(confForm, REQ_END_LINE);
				return -1;

			case KEY_UP:
				form_driver(confForm, REQ_PREV_FIELD);
				form_driver(confForm, REQ_END_LINE);
				return -1;

			case '\t':
				form_driver(confForm, REQ_NEXT_FIELD);
				form_driver(confForm, REQ_END_LINE);
				return -1;

			case KEY_BACKSPACE:
			case 127:
			case '\b': 
				form_driver(confForm, REQ_DEL_PREV);
				return -1;

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
			return -1;
		}
	} else {
		form_driver(confForm, REQ_VALIDATION);
		return 0;
	}
}

void FormContext::updateConnInfo(){
	// Updates and checks file
	clientInfo.addr = getFieldValue(formFields[0]); 
	string intStr = getFieldValue(formFields[1]);
	int portNum;

	try {
		portNum = stoi(intStr);
	} catch (...) {
		portNum = 0;
	}

	clientInfo.port = (uint16_t)portNum; 
	clientInfo.username = getFieldValue(formFields[2]); 
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


bool FormContext::fileCreate(){
	path configFile = getConfDir();

	ofstream config(configFile, ios::trunc);

	if (!config){
		return false;
	}

	config << "address:=" << clientInfo.addr << '\n';
	config << "port:=" << clientInfo.port << '\n';
	config << "username:=" << clientInfo.username << '\n';
	config.close();
	return true;
}

bool FormContext::fileVerify(){
	path configFile = getConfDir();

	if (!exists(configFile)){
		clientInfo = {};
		return false;
	}

	ifstream config(configFile);
	if (!config){
		return false;
	}

	string line, addrP, portP, usernameP;
	addrP = "address:=";
	portP = "port:=";
	usernameP = "username:=";

	while (getline(config, line)){
		if (line.rfind(addrP, 0) != string::npos){
			clientInfo.addr = line.substr(addrP.length());

		} else if (line.rfind(portP, 0) != string::npos){

			string subStr = line.substr(portP.length());
			if (subStr.length() > 5){
				config.close();
				return false;
			}

			try {
				int portInt = stoi(line.substr(portP.length()));
				if (portInt > 65536 || portInt < 0){
					throw 1;
				}
				clientInfo.port = (uint16_t)portInt;
			}

			catch (...){
				config.close();
				return false;
			}
				
		} else if (line.rfind(usernameP, 0) != string::npos){
			clientInfo.username = line.substr(usernameP.length());

		} else {
			continue;
		}
	}
	config.close();
	
	if (checkCliInfo()){
		return true;
	} else {
		clientInfo = {};
		return false;
	}
}

bool FormContext::checkCliInfo(){
	if (clientInfo.addr.empty() || clientInfo.port == 0 || clientInfo.username.empty()){
		return false;
	}

	if (clientInfo.username.length() > NAMELEN){
		return false;
	}

	if (clientInfo.port < 1 || clientInfo.port > 65535){
		return false;
	}

	return validateIpv4(clientInfo.addr);
}


bool FormContext::validateIpv4(string str){

	// Validate whole structure
	if (str.length() < 7 || str.length() > 15){
		return false;
	}

	optional<vector<string>> octetsOpt = octetTokenize(str);
	if (!octetsOpt){
		return false;
	}

	vector<string> octets = *octetsOpt;
	
	// Validate the octets of the address
	for (string & octet : octets){
		if (octet.length() > 3 || octet.empty()){
			return false;
		}

		for (char & ch : octet){
			if (!isdigit(ch)){
				return false;
			}
		}

		int octVal = stoi(octet);
		if (octVal > 255 || octVal < 0){
			return false;
		}
	}

	return true;
}



optional<vector<string>> FormContext::octetTokenize(string str){
	vector<string> outBuf;
	
	char* tok = strtok(str.data(), ".");
	size_t count = 0;

	while (tok){
		// Stop if its for some reason longer
		if (count > 4){
			return {};
		}

		string tokenStr(tok);
		outBuf.push_back(tokenStr);
		tok = strtok(NULL, ".");
		count++;
	}

	// Break if its too short
	if (count < 4){
		return {};
	}

	return outBuf;
}


