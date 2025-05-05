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
		std::cerr << "Invalid address/ Address not supported" << std::endl;
		return -1;
	}

	cout << "Options set" << endl;

	// Connect to server
	if (connect(sockFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
		perror("Connection failed");
		exit(1);
	}

	cout << "Connected to server" << endl;

	
	string line;

	// Get input
	cout << "Enter message: ";
	getline(cin, line);

	// Make packet
	vector<uint8_t> packet = makePacket(CMG_SERVMSG, line);

	// Send data
	sendPacket(sockFd, packet.data(), packet.size());

	// Close socket
	close(sockFd);
	return 0;
}

void sendPacket(int servFd, uint8_t* packet, size_t packetLen){
	send(servFd, packet, packetLen, 0);
	std::cout << "Packet sent" << std::endl;
}

vector<uint8_t> makePacket(opcode code, const string& line){
	// Make header
	PacketHeader header = makeHeader(code, line.size()+1);

	// Cast to bytes
	const uint8_t* byteHeader = reinterpret_cast<const uint8_t*>(&header);

	//make an output buffer 
	vector<uint8_t> outBuf;
	outBuf.reserve(header.length);

	// Insert header and string into output buffer
	const char* strBuf = line.data();
	outBuf.insert(outBuf.end(), byteHeader, byteHeader + sizeof(PacketHeader)); 
	outBuf.insert(outBuf.end(), strBuf, strBuf + line.size());	

	//Add null terminator
	outBuf.push_back(0);


	return outBuf;
}

PacketHeader makeHeader(opcode code, size_t stringLen){
	PacketHeader newHeader;
	newHeader.length = htole16((uint16_t)(stringLen + sizeof(PacketHeader)));
	newHeader.opcode = htole16((uint16_t)code);
	return newHeader;
}
