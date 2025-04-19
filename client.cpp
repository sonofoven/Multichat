#include "client.hpp"


int main() {
	int sockFd = 0;
	struct sockaddr_in serverAddr;
	const char *hello = "Hello from client";

	// Create socket
	sockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockFd < 0) {
		perror("socket failed");
		exit(1);
	}

	cout << "Socket created" << endl;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);


	// Convert IPv4 and IPv6 addresses from text to binary
	if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
		std::cerr << "Invalid address/ Address not supported" << std::endl;
		return -1;
	}

	cout << "Options set" << endl;

	// Connect to server
	if (connect(sockFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
		perror("Connection failed");
		exit(1);
	}

	cout << "Connected to server" << endl;

	// Send data
	send(sockFd, hello, strlen(hello), 0);
	std::cout << "Message sent" << std::endl;

	// Close socket
	close(sockFd);
	return 0;
}
