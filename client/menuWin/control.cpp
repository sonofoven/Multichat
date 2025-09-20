#include "interface.hpp"

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
