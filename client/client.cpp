#include "client.hpp"
#include "interface.hpp"

connInfo clientInfo;
shared_mutex fileMtx;
int epollFd = -1;

// Contexts

int main() {
	// Add sigwinch to epoll

	cout << "Multichat v" << VERSION << endl; // U

	interfaceStart();
	ContextController controller;

	controller.setupEpoll();
	if (controller.curState->startUp() == 2){
		// Skip to form if no valid setup
		controller.curState = make_unique<FormState>();
		controller.curState->startUp();
	}

	controller.controlEpoll();
}
