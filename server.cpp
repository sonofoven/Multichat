#include "server.hpp"

int main(){
	int serverFd, newSocket;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = {0};
	
	// Open socket
		// domain: ipv4, 
		// type: sequenced, reliable 
		// protocol: whatever the OS decides (TCP)
	serverFd = socket(AF_INET, SOCK_STREAM, 0);


	if (serverFd == 0){
		perror("socket failed");
		exit(1);
	}

	cout << "Socket created" << endl;

	// Set Options
		// SOL_SOCKET: set generic socket-level option
		// SO_REUSEADDR: lets you bind to an addr even if its in TIME_WAIT from a recent close
		// Pass in an opt integer to turn it on
	setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
		// Convert port number to network byte order
	address.sin_port = htons(PORT);

	cout << "Options set" << endl;

	// Bind
		// Associates a socket to an address & port num
	if (bind(serverFd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	cout << "Address bound to socket" << endl;

	// Listen
		// Specifies the max amount of amount of connections 
		// that can be queued while server is busy
	if (listen(serverFd, BACKLOG_MAX) < 0) {
		perror("listen");
		exit(1);
	}


	cout << "Server is listening" << endl;

	// Accept a connection
		// Accepts the first pending connection of the current socket
		// Creates a new socket not in a listening state for the client
		// Makes a new strut that holds address info
		// Holds length of the struct
		// Original socket is unaffected
	newSocket = accept(serverFd, (struct sockaddr *)&address, (socklen_t *)&addrlen);

	if (newSocket < 0) {
		perror("accept");
		exit(1);
	}

	cout << "Connection accepted" << endl;

	// Read data
	read(newSocket, buffer, 1024);
	std::cout << "Message from client: " << buffer << std::endl;

	// Close socket
	close(newSocket);
	close(serverFd);
	return 0;
}

