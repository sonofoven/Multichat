#include "server.hpp"

unordered_map<int, clientConn> clientMap = {};
mutex clientMapMtx;

unordered_map<string, int> userMap = {};

int main(){

	//signal(SIGINT, killServer); 
	registerPackets();

	signal(SIGINT, killServer);

	int listenFd;
	sockaddr_in address;

	int epollFd = -1;
	epoll_event events[MAX_EVENTS];


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

        int eventCount = epoll_wait(epollFd, events, MAX_EVENTS, -1);
        if (eventCount == -1) {
            if (errno == EINTR) {
                // our handler set g_quit ⇒ break out
                break;
            } else if (errno != EINTR) {
				cerr << "epoll wait" << endl;
                break;
            }
            // if EINTR but gquit==0
            continue;
        }


		for (int i = 0; i < eventCount; i++){

			int fd = events[i].data.fd;
			uint32_t event = events[i].events;

			// Client disconnected/connection drop
			if (event & (EPOLLHUP | EPOLLERR)){
				dropClient(fd);
				continue;
			}

			clientConn* clientPtr = lockFindCli(fd);

			bool fatal = false;

			if (clientPtr == NULL){
				fatal = true;
			}


			if (event & EPOLLIN && !fatal && !clientPtr->markToDie){
				// We are able to read
				// Then try to immediately write
				if ((handleRead(epollFd, *clientPtr) < 0)){// || (handleWrite(epollFd, *clientPtr) < 0))){
					cout << "Fatal read" << endl;
					fatal = true;
				}
			}

			if (event & EPOLLOUT && !fatal && !clientPtr->markToDie){
				// Finish writing what was started
				if (handleWrite(epollFd, *clientPtr) < 0){
					cout << "Fatal write" << endl;
					fatal = true;
				}
			}

			if (fatal || clientPtr->markToDie){
				dropClient(fd);
			}
		}
	}

	close(listenFd);
	killServer(0);
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

	//cout << "Socket created" << endl;

	// Set Options
	setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	address.sin_family = AF_INET; // ipv4
	address.sin_addr.s_addr = INADDR_ANY;
		// Convert port number to network byte order (big endian)
	address.sin_port = htons(PORT);

	//cout << "Options set" << endl;

	// Bind socket fd to address
	if (bind(sockFd, (sockaddr *)&address, sizeof(address)) < 0) {
		perror("bind failed");
		exit(1);
	}

	//cout << "Address bound to socket" << endl;

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
	event.events = EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR;
	event.data.fd = fd;

	// Add fd to epoll
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event)){
		perror("Failed to add fd to epoll");
		close(epollFd);
		exit(1);
	}

	clientMap[fd].epollMask = event.events;

	//cout << "Adding " << fd << " to epoll fd" << endl;

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

	//cout << "Modding " << fd  << endl;

}

int handleRead(int epollFd, clientConn& client){
	cout << "Reading packet from "<< client.username << endl;

	size_t headerSize = 2 * sizeof(uint16_t);

	vector<uint8_t>& readBuf = client.readBuf;
	vector<uint8_t>& writeBuf = client.writeBuf;
	size_t fullPacketLen;
	// Do all the reading here
	if(drainReadPipe(client.fd, client) != 0){
		return -1;
	}

	// If we don't even have a full header
	while(1){

		if (readBuf.size() < headerSize){
			return 0;
		}
		
		fullPacketLen = parsePacketLen(readBuf.data());
		cout << "Packet length is: " << fullPacketLen << endl;

		if (readBuf.size() >= fullPacketLen){
			bool wasEmpty = writeBuf.empty();

			// Create a new packet
			Packet* pkt = instancePacketFromData(readBuf.data());

			// Parse it into the right packet type and handle it
			protocolParser(pkt, client);

			// If the buffer is empty and we are adding to it
			// Only arm if writebuf was empty before we insert
			if (wasEmpty && writeBuf.size() > 0){
				// mod epoll fd to watch for open writes
				modFdEpoll(epollFd, client.fd, EPOLLIN | EPOLLOUT | EPOLLET);
			}

			// Remove the packet from the read queue
			readBuf.erase(readBuf.begin(), readBuf.begin() + fullPacketLen);
			return 0;

		} else {
			return 0;
		}
	}
}

int handleWrite(int epollFd, clientConn& client){
	// While there is still stuff in the buffer
	cout << "Sending packet to "<< client.username << endl;

	//vector<uint8_t>& readBuf = client.readBuf;
	vector<uint8_t>& writeBuf = client.writeBuf;
	cout << "Bytes to be written is: " << writeBuf.size() << endl;

	while (!writeBuf.empty()){

		ssize_t n = write(client.fd, writeBuf.data(), writeBuf.size());

		if (n > 0){
			// Erase what has been written from the write buffer
			writeBuf.erase(writeBuf.begin(), writeBuf.begin() + n);
			cout << "Written bytes is: " << n << endl;

			continue;
		}

		// Can't write anymore
		if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)){
			return 0;
		}

		// Do client deregistration here
		cerr << "Error in write" << endl;
		perror("Writing");
		return -1;
	}


	// If we have cleared the buffer, unset write flag
	if (writeBuf.empty() && (client.epollMask & EPOLLOUT)){
		cout << "CLEARING THIS" << endl;
		modFdEpoll(epollFd, client.fd, EPOLLIN | EPOLLET);
	}
	//cout << "Unset write mod" << endl;
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
	if (n == 0){
		// Conn closed
		return -1;
	} else if (n < 0){
		if (errno == EAGAIN || errno == EWOULDBLOCK){
			return 0;
		} else {
			perror("Read pipe drain");
			return -1;
		}
	}

	return 0;
}

int protocolParser(Packet* pkt, clientConn& sender){
	int exitCode = 0;
	uint16_t opcode = pkt->opcode;

	switch(opcode){
		case CMG_CONNECT: {
			cout << "OPCODE: CONNECTION" << endl;

			ClientConnect* clientPacket = static_cast<ClientConnect*>(pkt);

			clientConnect(*clientPacket, sender);

			break;
		}

		case CMG_BROADMSG: {
			cout << "OPCODE: BROAD MESSAGE" << endl;

			ClientBroadMsg* clientPacket = static_cast<ClientBroadMsg*>(pkt);

			clientBroadMsg(*clientPacket, sender);

			break;
		}

		case CMG_SERVMSG: {
			cout << "OPCODE: SERVER MESSAGE" << endl;

			ClientServMsg* clientPacket = static_cast<ClientServMsg*>(pkt);

			clientServerMessage(*clientPacket, sender);
			break;
		}

		case CMG_DISCONNECT: {
			cout << "OPCODE: DISCONNECTION" << endl;

			ClientDisconnect* clientPacket = static_cast<ClientDisconnect*>(pkt);

			clientDisconnect(*clientPacket, sender);

			break;
		}

		default: {
			cout << "Unknown opcode" << endl;
			exitCode = -1;
		}
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
