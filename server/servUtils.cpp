#include "server.hpp"

void clientConnect(ClientConnect& pkt, clientConn& sender){
	// Grab username
	string username = validateUser(pkt.username);

	if (username == ""){
		// Rejection

		// Create the rejection response to client
		ServerValidate response = ServerValidate(false, userMap);

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
		cout << "Adding username: " << username << endl;

		// Create the accept response to client
		ServerValidate response = ServerValidate(true, userMap);

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


	// If the client is trying to impersonate someone or 
	// try to send a message w/out connecting first kill them

	if(!valConnMsg(sender.username, sender)){
		cout << "Sabatoure detected" << endl;
		sender.markToDie = true;
		return;
	}
	
	// Construct the packet
	ServerBroadMsg responseAll = ServerBroadMsg(sender.username.c_str(), pkt.msgLen, pkt.msg);

	// Serialize it to everyone but sender
	serializeToAllButSender(responseAll, sender);
}

void clientServerMessage(ClientServMsg& pkt, clientConn& sender){
	// Output the message from the client to the console

	cout.write(pkt.msg, pkt.msgLen);
}

void clientDisconnect(ClientDisconnect& pkt, clientConn& sender) {
	// You pretty much should never recieve this, actually im gonna remove this
	// Announce to everyone but sender that this client has left

	// Set the client to be culled
	sender.markToDie = true;

	// Create the disconnect response to all other clients
	ServerDisconnect responseAll = ServerDisconnect(sender.username.c_str());

	// Send server connected msg to every other client
	serializeToAllButSender(responseAll, sender);
}



void dropClient(int fd){
	// Find client conn from fd
	clientConn* cliPtr = lockFindCli(fd);

	if (!cliPtr){
		killClient(fd);
	}
	
	// If client registered w/ username
	if (usernameExists(cliPtr->username.c_str()) != ""){

		// Inform log
		cout << "Dropping user: " << cliPtr->username << endl;

		// Create disconnect packet
		ServerDisconnect responseAll = ServerDisconnect(cliPtr->username.c_str());
		// Serialize to all
		serializeToAllButSender(responseAll, *cliPtr);
	}

	// Kill client
	killClient(fd);
}

clientConn* lockFindCli(int fd){
	clientConn* clientPtr = nullptr;

	// Lock the map to look for client
	lock_guard lock(clientMapMtx);
	auto it = clientMap.find(fd);
	if (it == clientMap.end()){
		// client not found
		return nullptr;
	}

	clientPtr = &it->second;

	return clientPtr;
}

string validateUser(const char* username){
	// If username is too long or too short, reject
	size_t usrLen = strlen(username);

	if (usrLen > NAMELEN || usrLen <= 0){
		return "";
	}

	// If username is taken, reject
	return usernameExists(username);
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

bool valConnMsg(string username, clientConn& sender){

	// Rudimentary check to prevent impersonation
	auto it = userMap.find(username);
	if (it == userMap.end()){
		// username not registered
		return false;
	}

	// check that username given is matching the one registered
	return it->second == sender.fd;
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


void killClient(int fd){

	cout << "Killing client: " << fd << endl;

	// Find client
	clientConn* clientPtr = lockFindCli(fd);

	if (!clientPtr){
		close(fd);
		return;
	}

	// Deregister the username
	killUser(clientPtr->username);

	// Wipe the socket
	close(fd);

	// Lock the map
	lock_guard lock(clientMapMtx);

	// Wipe from the map
	clientMap.erase(fd);
}

void killUser(string username){
	userMap.erase(username);
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
