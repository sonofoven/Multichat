#include "client.hpp"

list<string> userConns = {};
vector<uint8_t> readBuf;
connInfo clientInfo;

vector<uint8_t> writeBuf;
mutex writeMtx;
condition_variable writeCv;

mutex msgMtx;

int main() {
	registerPackets();

	// Check if setting file exists

	// if not -> form + generate one

	
	// Attempt connection

	// If no connection -> prompt reconnect

	// If connection -> check validation

	// If bad validation -> prompt message & kick out
	int servFd = startUp();

	if (servFd < 0){
		cout << "Connection Not Possible" << endl;
		exit(1);
	}

	cout << "Connection Validated" << endl;

	UiContext uiContext = interfaceStart();

	//Arbitrary thread #
	dealThreads(servFd, uiContext);

	while(1){
		userInput(uiContext);
	}


	endwin();

}
