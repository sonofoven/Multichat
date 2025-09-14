#include "interface.hpp"

int menuSetup(vector<string> choices, string caption){

	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

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

	ConfMenuContext context(locMenuWin, move(choices));
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
	endwin();

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

void configForm(){
}


WINDOW* centerWin(WINDOW* parent, string& title, string& caption, int height, int width){
	int startY = (LINES - height)/2;
	int startX = (COLS - width)/2;

	if (parent == NULL){
		parent = stdscr;
	}

	WINDOW* localWin = newwin(height, width, startY, startX);
	wrefresh(localWin);

	int leftPad = (width - title.length())/2;
	mvwprintw(localWin, 0, leftPad, title.c_str());

	leftPad = (width - caption.length())/2;
	int topPad = (height - HALIGN)/4;

	mvwprintw(localWin, topPad, leftPad, caption.c_str());

	box(localWin, 0,0);

	return localWin;
}
