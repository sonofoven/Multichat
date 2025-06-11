#include "client.hpp"

// The multiple threads
void dealThreads(int servFd, UiContext& context){

	thread readT(readThread, servFd, ref(context));
	thread writeT(writeThread, servFd);

	readT.detach();
	writeT.detach();
	updateUserWindow(context);
}

void userInput(UiContext& context){
	string message = getWindowInput(*context.inputWin, context);
	ClientBroadMsg pkt = ClientBroadMsg(message);
	
	{
		unique_lock lock(writeMtx);
		pkt.serialize(writeBuf);
	}
	writeCv.notify_one();

	vector<chtype> formattedStr = formatMessage(message, clientInfo.username.c_str());
	
	{
		unique_lock lock(msgMtx);
		appendToWindow(*context.msgWin, formattedStr, 1);
	}
}

int startUp(){
	int servFd = networkStart();
	if(servFd < 0){
		// Can't connect
		return -1;
	}

	// send validation
	sendOneConn(servFd);

	// wait for validation
	if (!recvOneVal(servFd)){
		return -1;
	}

	return servFd;
}

int networkStart(){
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


void sendOneConn(int servFd){
	ClientConnect pkt = ClientConnect(clientInfo.username.c_str());
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


bool recvOneVal(int servFd){
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
	ServerValidate* serverVal = static_cast<ServerValidate*>(pkt);

	bool retVal = serverVal->able;

	// Remove the packet from the read queue
	readBuf.clear();

	return retVal;
}

void readThread(int servFd, UiContext& context){
	// Sleep controlled by the read/write of the socket

	// Read what there is to the global readBuf (change global to pass in)

	size_t fullPacketLen;

	while(1){
		uint8_t buf[CHUNK];
		ssize_t n;
		while ((n = read(servFd, buf, CHUNK)) > 0){
			// Insert onto end of read buffer
			readBuf.insert(readBuf.end(), buf, buf + n);
		}

		// ERROR handling later
		if (n < 0){
			continue;
		}


		if (readBuf.size() < Packet::headerLen){
			continue;
		}
		
		fullPacketLen = parsePacketLen(readBuf.data());

		// Wait for full packet
		if (readBuf.size() >= fullPacketLen){
			Packet* pkt = instancePacketFromData(readBuf.data());

			// Switch depending on opcode
			protocolParser(pkt, context);

			// Remove the packet from the read queue
			readBuf.erase(readBuf.begin(), readBuf.begin() + fullPacketLen);

		}
	}
}

void writeThread(int servFd){
	while(1){
		vector<uint8_t> writeData;

		{
			// Lock the thread
			unique_lock lock(writeMtx);
			// Unlock til notified and writeBuf aint empty
			writeCv.wait(lock, []{return !writeBuf.empty(); });
			// Move writeBuf data to writeData and visa versa
			writeBuf.swap(writeData);
			// ^ This clears the writeBuf btw
		}

		// Write to fd
		ssize_t total = writeData.size();
		ssize_t sent = 0;
		while (total > sent){
			ssize_t n = write(servFd, writeData.data() + sent, total - sent);

			if (n < 0){
				// Write err
				cerr << "Error writing" << endl;
				break;
			}
			sent += n;
		}
	}
}


int protocolParser(Packet* pkt, UiContext& context){
	int exitCode = 0;
	uint16_t opcode = pkt->opcode;

	switch(opcode){
		case SMG_VALIDATE: {
			cerr << "OPCODE: VALIDATION" << endl;

			ServerValidate* serverPacket = static_cast<ServerValidate*>(pkt);

			serverValidate(*serverPacket, context);
			break;
		}

		case SMG_CONNECT: {
			cerr << "OPCODE: CONNECTION" << endl;

			ServerConnect* serverPacket = static_cast<ServerConnect*>(pkt);

			serverConnect(*serverPacket, context);

			break;
		}

		case SMG_BROADMSG: {
			cerr << "OPCODE: BROAD MESSAGE" << endl;

			ServerBroadMsg* serverPacket = static_cast<ServerBroadMsg*>(pkt);

			serverBroadMsg(*serverPacket, context);

			break;
		}


		case SMG_DISCONNECT: {
			cerr << "OPCODE: DISCONNECTION" << endl;

			ServerDisconnect* serverPacket = static_cast<ServerDisconnect*>(pkt);

			serverDisconnect(*serverPacket, context);

			break;
		}

		default: {
			cerr << "Unknown opcode" << endl;
			exitCode = -1;
		}
	}
	return exitCode;
}

void serverValidate(ServerValidate& pkt, UiContext& context){
	// YOU SHOULD NEVER GET THIS WHILE RUNNING
	// You can use this in the future to occasionally send out sync's

	// Check if got back good boy mark
	if (!pkt.able){
		// Push back to set up form here
		exit(0);
	}

	// Get and update userconn list

	// This coult be an issue with the move
	userConns = move(pkt.userList);
	updateUserWindow(context);
}

void serverConnect(ServerConnect& pkt, UiContext& context){
	// Informs the user about client disconnect
	string username(pkt.username);

	userConns.remove(username);

	// Sort in inverse alphabetical order (this is because how its printed)
	userConns.sort(greater<string>());

	vector<chtype> formattedStr = formatDisMessage(pkt.username);
	
	{
		unique_lock lock(msgMtx);
		appendToWindow(*context.msgWin, formattedStr, 1);
	}

	// Update the users window
	updateUserWindow(context);

}

void serverBroadMsg(ServerBroadMsg& pkt, UiContext& context){
	vector<chtype> formattedStr = formatPktMessage(pkt);

	{
		unique_lock lock(msgMtx);
		appendToWindow(*context.msgWin, formattedStr, 1);
	}
}

void serverDisconnect(ServerDisconnect& pkt, UiContext& context){
	// Informs the user about client disconnect
	string username(pkt.username);

	userConns.remove(username);

	// Sort in inverse alphabetical order (this is because how its printed)
	userConns.sort(greater<string>());

	vector<chtype> formattedStr = formatDisMessage(pkt.username);

	{
		unique_lock lock(msgMtx);
		appendToWindow(*context.msgWin, formattedStr, 1);
	}

	// Update the users window
	updateUserWindow(context);

}
