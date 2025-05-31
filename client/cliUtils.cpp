#include "client.hpp"

// The multiple threads
void dealThreads(int servFd, UiContext& context){

    thread readT(readThread, servFd, ref(context));
    thread writeT(writeThread, servFd);

	readT.detach();
	writeT.detach();
}

void userInput(UiContext& context){
	vector<uint8_t> message = getWindowInput(*context.inputWin, context);
	ClientBroadMsg pkt = ClientBroadMsg(message);
	
	{
		unique_lock lock(writeMtx);
		pkt.serialize(writeBuf);
	}

	vector<chtype> formattedStr = formatMessage(message, clientInfo.username.c_str());
	
	{
		unique_lock lock(msgMtx);
		appendToWindow(*context.msgWin, formattedStr, 1);
	}
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

		} else {
			continue;
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
			cout << "OPCODE: VALIDATION" << endl;

			ServerValidate* serverPacket = static_cast<ServerValidate*>(pkt);

			serverValidate(*serverPacket, context);
			break;
		}

		case SMG_CONNECT: {
			cout << "OPCODE: CONNECTION" << endl;

			ServerConnect* serverPacket = static_cast<ServerConnect*>(pkt);

			serverConnect(*serverPacket, context);

			break;
		}

		case SMG_BROADMSG: {
			cout << "OPCODE: BROAD MESSAGE" << endl;

			ServerBroadMsg* serverPacket = static_cast<ServerBroadMsg*>(pkt);

			serverBroadMsg(*serverPacket, context);

			break;
		}


		case SMG_DISCONNECT: {
			cout << "OPCODE: DISCONNECTION" << endl;

			ServerDisconnect* serverPacket = static_cast<ServerDisconnect*>(pkt);

			serverDisconnect(*serverPacket, context);

			break;
		}

		default: {
			cout << "Unknown opcode" << endl;
			exitCode = -1;
		}
	}
	return exitCode;
}

void serverValidate(ServerValidate& pkt, UiContext& context){
	// Check if got back good boy mark

	// Get and update userconn list

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
