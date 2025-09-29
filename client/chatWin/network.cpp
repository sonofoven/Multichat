#include "../interface.hpp"

void ChatContext::servFdStart(){
	servFd = networkStart();
	if (servFd < 0){
		// Can't connect
		return;
	}

	// send validation
	sendOneConn();

	// wait for validation
	if (!recvOneVal()){
		servFd = -1;
	}
}

int ChatContext::networkStart(){
	int sockFd = 0;
	struct sockaddr_in serverAddr;

	// Create socket
	sockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockFd < 0) {
		cerr << "Socket failed" << endl;
		return -1;
	}

	//cout << "Socket created" << endl;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(clientInfo.port);


	// Convert IPv4 and IPv6 addresses from text to binary
	if (inet_pton(AF_INET, clientInfo.addr.c_str(), &serverAddr.sin_addr) <= 0) {
		cerr << "Invalid address/ Address not supported" << endl;
		cerr << clientInfo.addr.c_str() << endl;
		return -1;
	}

	//cout << "Options set" << endl;

	// Connect to server
	if (connect(sockFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
		perror("Connection failed");
		return -1;
	} 

	cout << "Connected to server" << endl; 

	return sockFd;
}

void ChatContext::sendOneConn(){
	ClientConnect pkt = ClientConnect(clientInfo.username, getLatestLoggedMsgTime());
	pkt.serialize(writeBuf);

	// Write to fd
	ssize_t total = writeBuf.size();
	ssize_t sent = 0;
	while (total > sent){
		ssize_t n = write(servFd, writeBuf.data() + sent, total - sent);

		if (n < 0){
			// Write err
			cerr << "Error writing" << endl;
			break;
		}
		sent += n;
	}

	writeBuf.clear();
}


bool ChatContext::recvOneVal(){
	size_t fullLen;

	size_t headerLen = Packet::headerLen;

	size_t got = 0;
	readBuf.resize(headerLen);
	while (got < headerLen){
		ssize_t n = read(servFd, readBuf.data() + got, headerLen - got);

		if (n <= 0) {
			cerr << "Error reading header" << endl;
			return false;
		}

		got += n;
	}

	fullLen = parsePacketLen(readBuf.data());

	// Read rest of the packet
	readBuf.resize(fullLen);
	while (got < fullLen){
		ssize_t n = read(servFd, readBuf.data() + got, fullLen - got);

		if (n <= 0) {
			cerr << "Error reading rest of packet" << endl;
			return false;
		}

		got += n;
	}

	Packet* pkt = instancePacketFromData(readBuf.data());
	ServerValidate* serverVal = static_cast<ServerValidate*>(pkt); // It was static cast btw if it messes up

	bool retVal = serverVal->able;
	serverName = move(serverVal->servName);
	userConns = move(serverVal->userList);

	// Remove the packet from the read queue
	readBuf.clear();
	delete pkt;

	return retVal;
}

void ChatContext::handleRead(){
	if (drainReadFd() != 0){
		return;
	}

	while(readBuf.size() >= Packet::headerLen){

		size_t fullPacketLen = parsePacketLen(readBuf.data());

		if (fullPacketLen < Packet::headerLen){
			//cerr << "Lying runt" << endl;
			readBuf.erase(readBuf.begin());
			continue;
		}


		// Wait for full packet
		if (readBuf.size() >= fullPacketLen){
			Packet* pkt = instancePacketFromData(readBuf.data());

			if (!pkt){
				//cerr << "Packet cannot be instanced" << endl;
				readBuf.erase(readBuf.begin(), readBuf.begin() + fullPacketLen);
				continue;
			}

			// Switch depending on opcode
			protocolParser(pkt);
			delete pkt;

			// Remove the packet from the read queue
			readBuf.erase(readBuf.begin(), readBuf.begin() + fullPacketLen);
		} else {
			break;
		}
	}
}

void ChatContext::handleWrite(){
	ssize_t sent = write(servFd, writeBuf.data(), writeBuf.size());
	if (sent > 0){
		writeBuf.erase(writeBuf.begin(), writeBuf.begin() + sent);	
	} else if (sent < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
		//Big error, handle
		//perror("Backupwrite write");
		exit(1);
	}

	if (!writeBuf.empty()) {
		epollModify(epollFd, servFd, EPOLLIN|EPOLLOUT|EPOLLET|EPOLLHUP|EPOLLERR, EPOLL_CTL_MOD);
	} else {
		epollModify(epollFd, servFd, EPOLLIN|EPOLLET|EPOLLHUP|EPOLLERR, EPOLL_CTL_MOD);
	}
}

int ChatContext::drainReadFd(){
	uint8_t buf[CHUNK];
	ssize_t n;

	// Read til pipe is empty
	while ((n = read(servFd, buf, CHUNK)) > 0){
		// Insert onto end of read buffer
		readBuf.insert(readBuf.end(), buf, buf + n);
	}

	// Can't read any more
	if (n == 0){
		// Conn closed
		return -1;
	} else if (n < 0){
		if (errno == EAGAIN || errno == EWOULDBLOCK){
			return 0;
		} else {
			//perror("Read pipe drain");
			return -1;
		}
	}

	return 0;
}

