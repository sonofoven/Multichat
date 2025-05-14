#include "server.hpp"

unordered_map<int, clientConn> clientMap = {};
mutex clientMapMtx;

int main(){
	signal(SIGINT, killServer); 

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

	// Create a thread to constantly accept new conns
	thread listenT(acceptLoop, listenFd, epollFd);
	listenT.detach();

	while (1){
		// Get # of events captured instantaneously
		eventCount = epoll_wait(epollFd, events, MAX_EVENTS, -1);

		// If there is an error and it is NOT an interrupt
		if (eventCount == -1 && !(errno == EINTR)){
			cerr << "Epoll Wait failed: " << strerror(errno) << endl;
			killServer(errno);
		}

		for (int i = 0; i < eventCount; i++){

			int fd = events[i].data.fd;
			uint32_t event = events[i].events;

			if (event & (EPOLLHUP | EPOLLERR)){
				killClient(fd);
				continue;
			}

			clientConn* clientPtr = NULL;

			{
				// Lock the map to look for client
				lock_guard lock(clientMapMtx);
				auto it = clientMap.find(fd);
				if (it == clientMap.end()){
					// client not found
					continue;
				}

				clientPtr = &it->second;
			}

			bool fatal = false;

			if (event & EPOLLIN){
				// We are able to read
				// Then try to immediately write
				if(!(handleRead(epollFd, *clientPtr) && handleWrite(epollFd, *clientPtr))){
					fatal = true;
				}
			}

			if (event & EPOLLOUT){
				// Finish writing what was started
				if(!(handleWrite(epollFd, *clientPtr))){
					fatal = true;
				}
			}

			if (fatal){
				killClient(fd);
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

	clientMap[fd].epollMask = event.events;

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

	clientMap[fd].epollMask = event.events;

	cout << "Modding " << fd  << endl;

}

int handleRead(int epollFd, clientConn& client){
	cout<<"Reading stuff"<<endl;

	size_t headerSize = sizeof(PacketHeader);
	PacketHeader* header;

	vector<uint8_t>& readBuf = client.readBuf;
	vector<uint8_t>& writeBuf = client.writeBuf;

	// Do all the reading here
	if(drainReadPipe(client.fd, client) != 0){
		return -1;
	}

	while(1){
		// If we don't even have a full header
		if (readBuf.size() < headerSize){
			return 0;
		} else {
			// We have a header here
			header = (PacketHeader*)readBuf.data();
		}

		// Header length is size of the the full packet, including header
		size_t fullPacketLen = header->length;


		if (readBuf.size() >= fullPacketLen){

			// Create a new packet
			Packet packet;

			// Take the header
			packet.header = header;
			packet.data = readBuf.data() + sizeof(PacketHeader);
			packet.dataSize = fullPacketLen - sizeof(PacketHeader);


			protocolParser(packet, client);

			// If the buffer is empty and we are adding to it
			// Only arm if writebuf was empty before we insert
			if (writeBuf.empty() && !(client.epollMask & EPOLLOUT)){
				// mod epoll fd to watch for open writes
				modFdEpoll(epollFd, client.fd, EPOLLIN | EPOLLOUT | EPOLLET);
			}

			// Move packet (with header) from read to writeBuf
			// Later this wont be done b/c we'll have to make new packets using opcodes but for now this is what we'll do
			// We are just echoing back things to the client, we'll have to add an op handler
			writeBuf.insert(writeBuf.end(), readBuf.begin(), readBuf.begin() + fullPacketLen);

			readBuf.erase(readBuf.begin(), readBuf.begin() + fullPacketLen);
		} else {
			return 0;
		}
	}
	return 0;
}

int handleWrite(int epollFd, clientConn& client){
	// While there is still stuff in the buffer
	cout << "Writing stuff" << endl;

	//vector<uint8_t>& readBuf = client.readBuf;
	vector<uint8_t>& writeBuf = client.writeBuf;

	while (!writeBuf.empty()){

		ssize_t n = write(client.fd, writeBuf.data(), writeBuf.size());

		if (n > 0){
			// Erase what has been written from the write buffer
			writeBuf.erase(writeBuf.begin(), writeBuf.begin() + n);

			continue;
		}

		// Can't write anymore
		if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)){
			return 0;
		}

		// Do client deregistration here
		perror("Writing");
		return -1;
	}

	// If we have cleared the buffer, unset write flag
	if (writeBuf.empty() && (client.epollMask & EPOLLOUT)){
		modFdEpoll(epollFd, client.fd, EPOLLIN | EPOLLET);
	}
	return 0;
}


int drainReadPipe(int fd, clientConn& client){
	uint8_t buf[CHUNK];
	ssize_t n;

	// Read til pipe is empty
	while ((n = read(fd, buf, CHUNK)) > 0){
		// Insert onto end of read buffer
		client.readBuf.insert(client.readBuf.end(), buf, buf + n);
	}

	// Can't read any more
	if (n < 0){
		if (errno == EAGAIN || errno == EWOULDBLOCK){
			return 0;
		} else {
			perror("Read pipe drain");
			return -1;
		}
	}

	return 0;
}

int protocolParser(Packet& packet, clientConn& sender){
	int exitCode = 0;
	switch(packet.header->opcode){
		case CMG_CONNECT:
			cout << "OPCODE: CONNECTION" << endl;
			break;

		case CMG_BROADMSG:
			cout << "OPCODE: BROAD MESSAGE" << endl;
			break;

		case CMG_SERVMSG:
			cout << "OPCODE: SERVER MESSAGE" << endl;

			// Right now, if someone wants to send a packet without the null byte
			// they could read other memory, maybe make sure that the packet
			// sends a length and cap that length too

			// will get back to this

			clientServerMessage(packet, sender);
			break;

		case CMG_DISCONNECT:
			cout << "OPCODE: DISCONNECTION" << endl;
			break;

		default:
			cout << "Unknown opcode" << endl;
	}
	return exitCode;
}


void acceptLoop(int listenFd, int epollFd){
	int clientFd;
	sockaddr_in clientAddress;
	socklen_t clientLen = sizeof(clientAddress);
	
	// Set thread to loop forever
	while(1){
		// While there are still connections to accept
		while((clientFd = accept(listenFd, (sockaddr *)&clientAddress, (socklen_t *)& clientLen)) > 0){

			// Lock client map from other threads to prevent race conds
			lock_guard lock(clientMapMtx);

			// Add client fd to list of connected clients
			clientMap.emplace(clientFd, clientConn(clientFd));

			// Add connection fd to epoll marking list
			addFdToEpoll(epollFd, clientFd);

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

void killClient(int fd){

	cout << "Killing client: " << fd << endl;

	// Wipe the socket
	close(fd);

	// Lock the map
	lock_guard lock(clientMapMtx);

	// Wipe from the map
	clientMap.erase(fd);

}

void killServer(int code){
	// Inform
	cout << "Killing server over signal: " << code << endl;

	// Lock mutex
	lock_guard lock(clientMapMtx);

	// Kill every 
	for (auto i = clientMap.begin(); i != clientMap.end(); i++){
		killClient(i->first);
	}

	exit(1);
}
