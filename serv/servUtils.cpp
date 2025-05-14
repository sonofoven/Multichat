#include "server.hpp"

void clientServerMessage(Packet& packet, clientConn& sender){
	// Convert it to a string and print it out
	clientServMsg* srvMsg = (clientServMsg*)packet.data;
	string sendData((char*)srvMsg->msg);
	cout << sendData << endl;

}
