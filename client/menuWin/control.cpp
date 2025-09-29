#include "../interface.hpp"

int FileState::startUp(){
	if (!exists(getConfDir()) && !FormContext::fileVerify()){
		// If conf file doesn't exist go
		// or if its not valid
		// straight to form
		return 2;
	}

	string caption = "Existing config file detected";
	vector<string> choices {"Use Existing", "Configure New"}; 

	return Menu->menuSetup(move(choices), move(caption));
}

int FileState::handleInput(int ch){
	// Get selection or action
	
	return Menu->handleCh(ch);
}

int FileState::tearDown(){
	
	// Clear everything and revert back to nullptr
	Menu->freeAll();
	Menu.reset();

	return 0;
}


int ReconnectState::startUp(){

	string caption = "Connection failed";
	vector<string> choices {"Reconnect", "Configure"}; 

	Menu = make_unique<MenuContext>();

	return Menu->menuSetup(move(choices), move(caption));
}


int ReconnectState::handleInput(int ch){
	// Get selection or action
	
	return Menu->handleCh(ch);
}

int ReconnectState::tearDown(){
	
	// Clear everything and revert back to nullptr
	Menu->freeAll();
	Menu.reset();

	return 0;
}



void MenuContext::freeAll(){
	if (confMenu){
		unpost_menu(confMenu);
		free_menu(confMenu);
	}

	for (int i = 0; i < (int)myItems.size()-1; i++){
		if (myItems[i]){
			free_item(myItems[i]);
		}
	}

	if (subWin){
		delwin(subWin);
	}

	if (confWin){
		delwin(confWin);
	}
}
