#include "interface.hpp"

MenuContext::MenuContext(WINDOW* w, 
						 vector<string> c):
						 confWin(w),
						 choices(move(c)){

	myItems.reserve(choices.size());

	int menuWidth = (int)choices.size();

	for (const string& choice : choices){
		myItems.push_back(new_item(choice.c_str(), choice.c_str()));
		menuWidth += choice.length();
	}

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
}

void MenuContext::freeAll(){
	if (confMenu){
		unpost_menu(confMenu);
		free_menu(confMenu);
	}

	for (int i = 0; i < (int)myItems.size(); i++){
		if (myItems[i]){
			free_item(myItems[i]);
		}
	}

	if (confWin){
		delwin(confWin);
	}

	if (subWin){
		delwin(subWin);
	}
}
