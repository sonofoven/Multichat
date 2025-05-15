#include "server.hpp"

void clientServerMessage(Packet& packet, clientConn& sender){
	// Create a struct for the type of message
	clientServMsg msgStruct;

	// Set message at the starting point of the buffer (offset 0)
	msgStruct.msg = packet.data;


	string sendData((char*)msgStruct.msg);
	cout << sendData << endl;
}
