#include "server.hpp"

string validateUser(const char* username){
	// If username is too long or too short, reject
	size_t usrLen = strlen(username);

	if (usrLen > NAMELEN || usrLen <= 0){
		return "";
	}

	// If username is taken, reject
	return usernameExists(username);
}


void clientConnect(ClientConnect& pkt, clientConn& sender){
	// Grab username
	string username = validateUser(pkt.username);

	if (username == ""){
		// Rejection

		// Create the rejection response to client
		ServerValidate response = ServerValidate(false);

		// Send rejection response to client
		response.serialize(sender.writeBuf);

		// Mark them to die next polling loop
		sender.markToDie = true;
	} else {
		// Accept
		////Might have to copy over the string because packets gets killed but it could auto copy over 

		// Register the username to the map
		userMap[username] = sender.fd;

		// Add the username to the sender's clientConn
		sender.username = username;

		// Create the accept response to client
		ServerValidate response = ServerValidate(true);

		// Send server val with true val to sender
		response.serialize(sender.writeBuf);

		// Inform other clients that sender has joined

		// Construct packet
		ServerConnect responseAll = ServerConnect(pkt.username);

		// Send server connected msg to every other client
		serializeToAllButSender(responseAll, sender);
	}

}

void clientBroadMsg(ClientBroadMsg& pkt, clientConn& sender){
	// Send server broad msg to every other client
}

void clientServerMessage(ClientServMsg& pkt, clientConn& sender){

	// Get the length of the message because it is not null terminated
	size_t msgLength = pkt.length - pkt.headerLen;

	cout.write(pkt.msg, msgLength);
}

void clientDisconnect(ClientDisconnect& pkt, clientConn& sender) {
	// Announce to everyone but sender that this client has left
}

clientConn* lockFindCli(int fd){
		clientConn* clientPtr = NULL;

		// Lock the map to look for client
		lock_guard lock(clientMapMtx);
		auto it = clientMap.find(fd);
		if (it == clientMap.end()){
			// client not found
			return NULL;
		}

		clientPtr = &it->second;

		return clientPtr;
}

string usernameExists(const char* username){
	string userStr = username;
	auto it = userMap.find(userStr);
	if (it == userMap.end()){
		// username not taken
		return userStr;
	}

	// username taken
	return "";
}

void serializeToAllButSender(Packet& pkt, clientConn& sender){
	lock_guard lock(clientMapMtx);
	
	// Loop over clients connected in the map
	for (auto i : clientMap){
		clientConn client = i.second;
		int fd = i.first;

		// If the sender id is not the current fd
		if (sender.fd != fd){
			pkt.serialize(client.writeBuf);
		}
	}
}

