#include "client.hpp"

// The multiple threads
void dealThreads(int servFd, UiContext& context){

	thread inputT(inputThread, ref(context));
    thread readT(readThread, servFd, ref(context));
    thread writeT(writeThread, servFd);

	readT.detach();
	writeT.detach();
	inputT.join();
}


void inputThread(UiContext& context){
	vector<uint8_t> outBuf;
	while(1){
		
		// Get input
		int ch = getch();

		if (ch != ERR){
			// If we get input
			outBuf = processOneChar(*context.inputWin, context, ch, outBuf);
		}

		if (renderQueue.size() > 0){
			// Render stuff here

		}

	}
}

//void readThread(int servFd, UiContext& context){
//	// Sleep controlled by the read/write of the socket
//
//	// Read what there is to the global readBuf (change global to pass in)
//
//	size_t fullPacketLen;
//
//	while(1){
//		uint8_t buf[CHUNK];
//		ssize_t n;
//		while ((n = read(servFd, buf, CHUNK)) > 0){
//			// Insert onto end of read buffer
//			readBuf.insert(readBuf.end(), buf, buf + n);
//		}
//
//		// ERROR handling later
//
//
//		if (readBuf.size() < Packet::headerLen){
//			continue;
//		}
//		
//		fullPacketLen = parsePacketLen(readBuf.data());
//
//		// Wait for full packet
//		if (readBuf.size() >= fullPacketLen){
//			Packet* pkt = instancePacketFromData(readBuf.data());
//
//			// Switch depending on opcode
//			protocolParser(pkt, context);
//
//			// Remove the packet from the read queue
//			readBuf.erase(readBuf.begin(), readBuf.begin() + fullPacketLen);
//
//		} else {
//			continue;
//		}
//	}
//}
//
//void writeThread(int servFd, bool& ready, condition_variable& writeCv, mutex& writeMtx){
//	while(1){
//		// Create a unique lock for the write thread
//		unique_lock lock(writeMtx);
//
//		// Wait while the buffer is empty and not ready
//		// Guard against spurious wakeups (wakeups that you have not set)
//		while (writeBuf.empty() && !ready){
//			writeCv.wait(lock);
//		}
//
//		while (!writeBuf.empty()){
//			vector<uint8_t> writeData;
//
//			// Move the entirety of the buffer to a temp buf
//			writeData.insert(writeData.end(), writeBuf.begin(), writeBuf.end());
//
//			// Erase everything in the write buffer
//			writeBuf.clear();
//
//
//			// Unlock the lock while doing a blocking write
//			lock.unlock();
//			
//			// Write to fd
//			write(servFd, writeData.data(), writeData.size());
//
//			// Return the lock
//			lock.lock();
//		}
//		ready = false;
//	}
//
//}
//
//int protocolParser(Packet* pkt, UiContext& context){
//	int exitCode = 0;
//	uint16_t opcode = pkt->opcode;
//
//	switch(opcode){
//		case SMG_VALIDATE: {
//			cout << "OPCODE: VALIDATION" << endl;
//
//			ServerValidate* serverPacket = static_cast<ServerValidate*>(pkt);
//
//			serverValidate(*serverPacket, context);
//			break;
//		}
//
//		case SMG_CONNECT: {
//			cout << "OPCODE: CONNECTION" << endl;
//
//			ServerConnect* serverPacket = static_cast<ServerConnect*>(pkt);
//
//			serverConnect(*serverPacket, context);
//
//			break;
//		}
//
//		case SMG_BROADMSG: {
//			cout << "OPCODE: BROAD MESSAGE" << endl;
//
//			ServerBroadMsg* serverPacket = static_cast<ServerBroadMsg*>(pkt);
//
//			serverBroadMsg(*serverPacket, context);
//
//			break;
//		}
//
//
//		case SMG_DISCONNECT: {
//			cout << "OPCODE: DISCONNECTION" << endl;
//
//			ServerDisconnect* serverPacket = static_cast<ServerDisconnect*>(pkt);
//
//			serverDisconnect(*serverPacket, context);
//
//			break;
//		}
//
//		default: {
//			cout << "Unknown opcode" << endl;
//			exitCode = -1;
//		}
//	}
//	return exitCode;
//}
//
//void serverValidate(ServerValidate& pkt, UiContext& context){
//
//
//}
//
//void serverConnect(ServerConnect& pkt, UiContext& context){
//	// Informs the user about the server connection
//	string username(pkt.username);
//	userConns.push_back(username);
//
//	// Sort in inverse alphabetical order (this is because how its printed)
//	userConns.sort(greater<string>());
//
//	{
//		lock_guard lock(context.ncursesMtx);
//
//		// Update the users window
//		updateUserWindow(*context.userWin);
//	}
//}
//
//void serverBroadMsg(ServerBroadMsg& pkt, UiContext& context){
//
//}
//
//void serverDisconnect(ServerDisconnect& pkt, UiContext& context){
//	// Informs the user about client disconnect
//	string username(pkt.username);
//
//	userConns.remove(username);
//
//	// Sort in inverse alphabetical order (this is because how its printed)
//	userConns.sort(greater<string>());
//
//	{
//		lock_guard lock(context.ncursesMtx);
//
//		// Update the users window
//		updateUserWindow(*context.userWin);
//	}
//}
