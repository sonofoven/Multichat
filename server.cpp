#include "server.hpp"

int main(){
	int listenFd;
	sockaddr_in address;

	int epollFd = -1;
	epoll_event events[MAX_EVENTS];
	int eventCount;

	// Create a listening socket
	listenFd = makeListenSocket(address);

	cout << "Server is listening" << endl;

	// Create our epoll instance
	epollFd = epoll_create1(0);

	if (epollFd == -1){
		perror("Epoll creation failure");
		exit(1);
	}

	// Add listening fd to the socket
	addFdToEpoll(epollFd, listenFd);

	while (1){
		// Get # of events captured instantaneously
		eventCount = epoll_wait(epollFd, events, MAX_EVENTS, -1);
		for (int i = 0; i < eventCount; i++){

			// If event is listenFd
			if (events[i].data.fd == listenFd){
				
				// Accept connection
				handleAccept(epollFd, listenFd);
			} else {

				// Handle connection
				handleReadOrWrite(events[i].data.fd);
			}
		}
	}

	// Close socket
	close(listenFd);
	return 0;
}


int makeListenSocket(sockaddr_in address){
	int sockFd = -1;
	int opt = 1;

	// Open socket
	sockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockFd == 0){
		perror("socket failed");
		exit(1);
	}

	cout << "Socket created" << endl;

	// Set Options
	setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	address.sin_family = AF_INET; // ipv4
	address.sin_addr.s_addr = INADDR_ANY;
		// Convert port number to network byte order (big endian)
	address.sin_port = htons(PORT);

	cout << "Options set" << endl;

	// Bind socket fd to address
	if (bind(sockFd, (sockaddr *)&address, sizeof(address)) < 0) {
		perror("bind failed");
		exit(1);
	}

	cout << "Address bound to socket" << endl;

	// Listen
	if (listen(sockFd, BACKLOG_MAX) < 0) {
		perror("listen");
		exit(1);
	}

	return sockFd;
}
	
void addFdToEpoll(int epollFd, int fd){
	
	// Set the file descriptor to nonblocking
	fcntl(fd, F_SETFL, O_NONBLOCK);

	// Set event to check input and set as edge triggered
	epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = fd;
 	

	// Add fd to epoll
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event)){
		perror("Failed to add fd to epoll");
		close(epollFd);
		exit(1);
	}

	cout << "Adding " << fd << " to epoll fd" << endl;

}

void handleAccept(int epollFd, int listenFd){
	int clientFd;
	sockaddr_in clientAddress;
	socklen_t clientLen = sizeof(clientAddress);

	// While there are still connections to accept
	while((clientFd = accept(listenFd, (sockaddr *)&clientAddress, (socklen_t *)&clientLen))>0){

		cout << "Connection accepted" << endl;
		
		// Add connection fd to epoll marking list
		addFdToEpoll(epollFd, clientFd);
	}

	if (clientFd == -1 && errno != EAGAIN && errno != EWOULDBLOCK){
		perror("Accept handling");
		close(epollFd);
		exit(1);
	}
}

void handleReadOrWrite(int fd){
	char buffer[1024] = {0};
	// Read data

	read(fd, buffer, 1024);
	std::cout << "Message from client: " << buffer << std::endl;
}

