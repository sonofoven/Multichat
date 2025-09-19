#include "interface.hpp"

int menuSetup(vector<string> choices, string caption){

	string title = "| MultiChat |";

	int minHeight = MENU_HEIGHT;
	int minWidth = max(MENU_WIDTH, (int)caption.length() + 2*HALIGN);

	if (LINES < minHeight || COLS < minWidth){
		string errMsg = "Window too small";
		mvprintw(LINES / 2, (COLS - (int)errMsg.size())/2, errMsg.c_str());
		return -1;
	}

	WINDOW* locMenuWin = centerWin(NULL, 
								   title, caption, 
								   minHeight, minWidth);

	MenuContext context(locMenuWin, move(choices));
	MENU* locMenu = context.confMenu;

	menu_opts_off(locMenu, O_SHOWDESC);
	menu_opts_off(locMenu, O_NONCYCLIC);
	keypad(locMenuWin, TRUE);
	 
	set_menu_win(locMenu, locMenuWin);
	set_menu_sub(locMenu, context.subWin);
	set_menu_format(locMenu, 1, context.choices.size());
	set_menu_mark(locMenu, "");

	refresh();

	post_menu(locMenu);
	wrefresh(locMenuWin);
	
	int c;
	while((c = wgetch(locMenuWin)) != '\n'){	   
		switch(c){
			case '\t':
			case KEY_RIGHT:
			case KEY_LEFT:
			case 'h':
			case 'l':
				menu_driver(locMenu, REQ_NEXT_ITEM);
				break;

			wrefresh(locMenuWin);
		}	
	}

	int selection = item_index(current_item(locMenu));

	context.freeAll();

	return selection;
}

int configMenu(){
	string caption = "Existing config file detected";
	vector<string> choices {"Use Existing", "Configure New"}; 

	return menuSetup(move(choices), move(caption));
}

int reconnectMenu(){
	string caption = "Connection failed";
	vector<string> choices {"Reconnect", "Configure"}; 

	return menuSetup(move(choices), move(caption));
}

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


WINDOW* centerWin(WINDOW* parent, string& title, string& caption, int height, int width){ 
	int startY = (LINES - height)/2;
	int startX = (COLS - width)/2;

	if (parent == NULL){
		parent = stdscr;
	}

	WINDOW* localWin = newwin(height, width, startY, startX);

	wrefresh(localWin);


	int leftPad = (width - caption.length())/2;
	int topPad = VALIGN * 2;

	mvwprintw(localWin, topPad, leftPad, caption.c_str());

	box(localWin, 0,0);

	leftPad = (width - title.length())/2;
	mvwprintw(localWin, 0, leftPad, title.c_str());

	return localWin;
}

path FormContext::getConfDir(){
	const char* home = getenv("HOME");
	// Not a check but if you have this unset, you have bigger problems
	path configDir = path(home) / STORAGE_DIR;
	create_directories(configDir);
	path configFile = configDir / "config";

	return configFile;
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



