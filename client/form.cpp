#include "interface.hpp"

bool configMenu(){
	
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	vector<string> choices {"Use Existing", "Configure New"};
	string caption = "Existing config file detected";
	string title = "| MultiChat |";
	WINDOW* locMenuWin = centerWin(NULL, title, caption, 4, 4);

	ConfMenuContext context(locMenuWin, move(choices));
	MENU* locMenu = context.confMenu;

	menu_opts_off(locMenu, O_SHOWDESC);
	menu_opts_off(locMenu, O_NONCYCLIC);
	keypad(locMenuWin, TRUE);
	 
	set_menu_win(locMenu, locMenuWin);
	set_menu_sub(locMenu, context.subWin);
	set_menu_format(locMenu, 2, 1);
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

			//item_index();
			//current_index();
			wrefresh(locMenuWin);
		}	
	}

	context.freeAll();
	endwin();
}

bool reconnectMenu(){
}

void configForm(){
	FIELD *field[3];
	FORM  *my_form;
	int ch;
	
	/* Initialize curses */
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	/* Initialize the fields */
	field[0] = new_field(1, 10, 4, 18, 0, 0);
	field[1] = new_field(1, 10, 6, 18, 0, 0);
	field[2] = NULL;

	/* Set field options */
	set_field_back(field[0], 0); 	/* Print a line for the option 	*/
	field_opts_off(field[0], O_AUTOSKIP);  	/* Don't go to next field when this */
						/* Field is filled up 		*/
	set_field_back(field[1], 0); 
	field_opts_off(field[1], O_AUTOSKIP);

	/* Create the form and post it */
	my_form = new_form(field);
	post_form(my_form);
	refresh();
	
	mvprintw(4, 10, "Value 1:");
	mvprintw(6, 10, "Value 2:");
	refresh();

	/* Loop through to get user requests */
	while((ch = getch()) != KEY_F(1))
	{	switch(ch)
		{	case KEY_DOWN:
				/* Go to next field */
				form_driver(my_form, REQ_NEXT_FIELD);
				/* Go to the end of the present buffer */
				/* Leaves nicely at the last character */
				form_driver(my_form, REQ_END_LINE);
				break;
			case KEY_UP:
				/* Go to previous field */
				form_driver(my_form, REQ_PREV_FIELD);
				form_driver(my_form, REQ_END_LINE);
				break;
			default:
				/* If this is a normal character, it gets */
				/* Printed				  */	
				form_driver(my_form, ch);
				break;
		}
	}
	// field_buffer

	/* Un post form and free the memory */
	unpost_form(my_form);
	free_form(my_form);
	free_field(field[0]);
	free_field(field[1]); 

	endwin();
}


WINDOW* centerWin(WINDOW* parent, string& title, string& caption, int rowDiv, int colDiv){
	int height = LINES / rowDiv;
	int width = COLS / colDiv;
	int startY = (LINES - height)/2;
	int startX = (COLS - width)/2;

	if (parent == NULL){
		parent = stdscr;
	}

	WINDOW* localWin = newwin(height, width, startY, startX);
	box(localWin, 0, 0);
	wrefresh(localWin);

	int leftPad = (width - title.length())/2;
	mvwprintw(localWin, 0, leftPad, title.c_str());

	leftPad = (width - caption.length())/2;
	int topPad = (height - HALIGN)/4;

	mvwprintw(localWin, topPad, leftPad, caption.c_str());

	box(localWin, 0,0);

	return localWin;
}
