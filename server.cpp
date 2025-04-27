#include "server.hpp"

int main(){
	int listenFd;
	sockaddr_in address;

	int epollFd = -1;
	epoll_event events[MAX_EVENTS];
	int eventCount;

	unordered_map<int, clientConn> clientMap = {};

	// Create a listening socket
	listenFd = makeListenSocket(address);

	cout << "Server is listening" << endl;

	// Create our epoll instance
	epollFd = epoll_create1(0);

	if (epollFd == -1){
		perror("Epoll creation failure");
		exit(1);
	}

	// Create a thread to constantly accept new conns
	thread listenT(acceptLoop, listenFd, epollFd, ref(clientMap));
	listenT.detach();

	while (1){
		// Get # of events captured instantaneously
		eventCount = epoll_wait(epollFd, events, MAX_EVENTS, -1);

		for (int i = 0; i < eventCount; i++){

			int fd = events[i].data.fd;
			uint32_t event = events[i].events;

			if (event & (EPOLLHUP | EPOLLERR)){
				// Client has disconnected

				// Close autoderegisters fd from epoll
				clientMap.erase(fd);
				close(fd);
			}

			if (event & EPOLLIN){
				// We are able to read
				// Attempt read
				handleRead(epollFd, clientMap[fd]);
				handleWrite(epollFd, clientMap[fd]);
			}

			if (event & EPOLLOUT){
				// Finish writing what was started
				handleWrite(epollFd, clientMap[fd]);
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

void modFdEpoll(int epollFd, int fd, int ops){
	
	// Set event to new ops, make sure EPOLLET is set regardless
	epoll_event event;
	event.events = ops | EPOLLET;
	event.data.fd = fd;

	// Add fd to epoll
	if (epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event)){
		perror("Failed to modify fd in epoll");
		close(epollFd);
		exit(1);
	}

	cout << "Modding " << fd  << endl;

}

void handleRead(int epollFd, clientConn& client){

	size_t headerSize = sizeof(PacketHeader);
	PacketHeader header;

	// Do all the reading here
	drainReadPipe(client.fd, client);

	while(1){
		// If we don't even have a full header
		if (client.readBuf.size() < headerSize){
			return;
		} else {
			// We have a header here
			header = extractHeader(client);
		}

		size_t fullPacketLen = headerSize + header.length;

		if (client.readBuf.size() >= fullPacketLen){
			// If the buffer is empty and we are adding to it
			if (client.readBuf.empty()){
				// mod epoll fd to watch for open writes
				modFdEpoll(epollFd, client.fd, EPOLLIN | EPOLLOUT | EPOLLET);
			}

			// Move packet (with header) from read to writeBuf
			// Later this wont be done b/c we'll have to make new packets using opcodes but for now this is what we'll do
			// We are just echoing back things to the client, we'll have to add an op handler
			client.writeBuf.insert(client.writeBuf.end(), client.readBuf.begin(), client.readBuf.begin() + fullPacketLen);
			client.readBuf.erase(client.readBuf.begin(), client.readBuf.begin() + fullPacketLen);
		} else {
			break;
		}

	}
	return;
}

void handleWrite(int epollFd, clientConn& client){
	// While there is still stuff in the buffer
	while (!client.writeBuf.empty()){
		// Only write what is needed
		size_t sizeWrite = min(client.writeBuf.size(), (size_t)CHUNK);
		vector<uint8_t> tmp;
		tmp.reserve(sizeWrite);

		// Copy over one chunk/block of info to vector
		copy_n(client.writeBuf.begin(), sizeWrite, back_inserter(tmp));

		ssize_t n = write(client.fd, tmp.data(), tmp.size());

		if (n > 0){
			// Erase what has been written from the write buffer
			client.writeBuf.erase(client.writeBuf.begin(), client.writeBuf.begin() + (size_t)n);
			continue;
		}

		// Can't write anymore
		if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)){
			break;
		}

		// Do client deregistration here
		perror("Writing");
		break;
	}

	// If we have cleared the buffer, unset write flag
	if (client.writeBuf.empty()){
		modFdEpoll(epollFd, client.fd, EPOLLIN | EPOLLET);
	}
}

PacketHeader extractHeader(clientConn& client){
	PacketHeader header;
	uint8_t* p = (uint8_t*)&header;

	for (size_t i = 0; i < sizeof(header); i++){
		p[i] = client.readBuf[i];
	}
	
	// Set header from little endian to host endian
	header.length = le16toh(header.length);
	header.opcode = le16toh(header.opcode);

	return header;
}

void drainReadPipe(int fd, clientConn& client){
	array<uint8_t, CHUNK> buf{};
	ssize_t n;

	// Read til pipe is empty
	while ((n = read(fd, buf.data(), buf.size())) > 0){
		client.readBuf.insert(client.readBuf.end(), buf.begin(), buf.begin() + n);
	}

	// EOF
	if (n == 0){
		return;
	}

	// Can't read any more
	if (n < 0){
		if (errno == EAGAIN || errno == EWOULDBLOCK){
			return;
		} else {
			perror("Reading");
			return;
		}
	}
}


void acceptLoop(int listenFd, int epollFd, unordered_map<int, clientConn>& clients){
	int clientFd;
	sockaddr_in clientAddress;
	socklen_t clientLen = sizeof(clientAddress);
	
	// Set thread to loop forever
	while(1){
		// While there are still connections to accept
		while((clientFd = accept(listenFd, (sockaddr *)&clientAddress, (socklen_t *)& clientLen)) > 0){

			// Add connection fd to epoll marking list
			addFdToEpoll(epollFd, clientFd);
			
			// Add client fd to list of connected clients
			clients.emplace(clientFd, clientConn(clientFd));

			cout << "Connection accepted" << endl;
		}

		if (clientFd == -1 && errno != EAGAIN && errno != EWOULDBLOCK){
			perror("Accept handling");
			close(epollFd);
			close(listenFd);
			exit(1);
		}
	}
}
