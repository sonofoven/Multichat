#include "client.hpp"
#include "interface.hpp"

int prevState = SIZE_ERR; //G
int state = SIZE_ERR; //G


atomic<bool> redrawQueued = false;
connInfo clientInfo;
shared_mutex fileMtx;

// Contexts

int main() {
	// Add sigwinch to epoll
	
	struct sigaction sa {}; // U
	sa.sa_handler = sigwinchHandler;
	sa.sa_flags = SA_RESTART;
	sigaction(SIGWINCH, &sa, nullptr);

	cout << "Multichat v" << VERSION << endl; // U

	interfaceStart(); // U

	ReconnectState reconState;
	reconState.startUp();
	reconState.running();
	reconState.tearDown();

	FormState formState;
	formState.startUp();
	formState.running();
	formState.tearDown();

	FileState fileState;
	fileState.startUp();
	fileState.running();
	fileState.tearDown();
	endwin();


	//if (redrawQueued.exchange(false)){
	//	endwin();
	//	clearok(stdscr, TRUE);
	//	refresh();

	//	int rows, cols;
	//	getmaxyx(stdscr, rows, cols);
	//	resizeterm(rows, cols);
	//	redrawUi(uiContext, rows, cols);
	//	refresh();
	//	continue; // Remove this later
	//}


}

void sigwinchHandler(int sig){
	redrawQueued = true;
}

