#include "server.hpp"

void clientServerMessage(Packet* packet, clientConn& sender){
	// Get the length of the message because it is not null terminated
	size_t msgLength = packet->length - 2*sizeof(uint16_t);
	cout.write(packet->msg, msgLength);
}
