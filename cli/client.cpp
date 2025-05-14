#include "client.hpp"


int main() {
	int sockFd = 0;
	struct sockaddr_in serverAddr;

	// Create socket
	sockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockFd < 0) {
		perror("socket failed");
		exit(1);
	}

	cout << "Socket created" << endl;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);


	// Convert IPv4 and IPv6 addresses from text to binary
	if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
		cerr << "Invalid address/ Address not supported" << endl;
		return -1;
	}

	cout << "Options set" << endl;

	// Connect to server
	if (connect(sockFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
		perror("Connection failed");
		exit(1);
	}

	cout << "Connected to server" << endl;

	// Get text input	
	vector<uint8_t> data = getTextInput();

	// Make packet
	Packet packet;
	packet.data = data.data();
	packet.dataSize = data.size();
	PacketHeader header = makeHeader(CMG_SERVMSG, packet.size());
	packet.header = &header;

	// Send packet
	send(sockFd, packet.exportData().data(), packet.size(), 0);
	//// Add err checking

	cout << "Sent packet to server" << endl;

	// Close socket
	close(sockFd);

 return 0;

 //interfaceStart();
}

vector<uint8_t> getTextInput(){
	vector<uint8_t> outBuf;
	string text;
	cout << "Enter message: ";
	cin >> text;

	outBuf.insert(outBuf.begin(), text.begin(), text.end());

	// Add null terminators
	//while (outBuf.size() < sizeof(clientServMsg)){
	outBuf.push_back('\0');
	//}
	
	return outBuf;
}

PacketHeader makeHeader(opcode code, size_t length){
	PacketHeader newHeader;
	newHeader.length = htole16((uint16_t)(length));
	newHeader.opcode = htole16((uint16_t)code);
	return newHeader;
}
