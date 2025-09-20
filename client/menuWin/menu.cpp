#include "interface.hpp"

int MenuContext::menuSetup(vector<string> choices, string caption){

	string title = "| MultiChat |";

	int minHeight = MENU_HEIGHT;
	int minWidth = max(MENU_WIDTH, (int)caption.length() + 2*HALIGN);

	if (LINES < minHeight || COLS < minWidth){
		string errMsg = "Window too small";
		mvprintw(LINES / 2, (COLS - (int)errMsg.size())/2, errMsg.c_str());
		return -1;
	}

	myItems.reserve(choices.size());

	int menuWidth = (int)choices.size();

	for (const string& choice : choices){
		myItems.push_back(new_item(choice.c_str(), choice.c_str()));
		menuWidth += choice.length();
	}

	WINDOW* confWin = centerWin(NULL, 
								title, caption, 
								minHeight, minWidth);

	confMenu = new_menu(myItems.data());

	int rows, cols;
	getmaxyx(confWin, rows, cols);

	int nlines = rows/2 - VALIGN;
	int ncols = menuWidth;
	int begY = VALIGN + rows/2;
	int begX = (cols - HALIGN)/2 - menuWidth/2;

	subWin = derwin(confWin,
					nlines, ncols,
					begY, begX);


	menu_opts_off(confMenu, O_SHOWDESC);
	menu_opts_off(confMenu, O_NONCYCLIC);
	keypad(confWin, TRUE);
	 
	set_menu_win(confMenu, confWin);
	set_menu_sub(confMenu, subWin);
	set_menu_format(confMenu, 1, choices.size());
	set_menu_mark(confMenu, "");

	refresh();

	post_menu(confMenu);
	wrefresh(confWin);

	return 0;
}

int MenuContext::getSelection(){
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

	return selection;
}


