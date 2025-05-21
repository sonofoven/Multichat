#include "server.hpp"

string usernameValid(const char* username){
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
	string username = usernameValid(pkt.username);

	if (username == ""){
		// Rejection
		// Send server val with false val
	}

	// Accept
	// Register the username to the map
	// Add the username to the sender's clientConn
	// Send server val with true val to sender
	// Send server connected msg to every other client

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

clientConn* lockAndFind(int fd){
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
