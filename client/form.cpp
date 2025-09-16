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

	context.freeAll();

	return 0;
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
