#include <endian.h>
#include <vector>

#define PORT 8080
#define CHUNK 4096
#define MAXMSG 512

enum opcode {
	CMG_CONNECT, // Connect and auth
	CMG_BROADMSG, // Message from one client to every client
	CMG_SERVMSG, // Message directly to the server
	CMG_DISCONNECT, // Disconnect
	SMG_CONNVAL, // Connection validation, tells client if it can join
	SMG_MSG // Sent directly from server to client
};

struct PacketHeader {
	uint16_t length;
	uint16_t opcode;
};

struct Packet {
	PacketHeader* header;
	uint8_t* data;
	size_t dataSize;

	size_t size(){
		return dataSize + sizeof(PacketHeader);
	}

	// Change so a copy is not needed
	std::vector<uint8_t> exportData(){
		std::vector<uint8_t> outBuf;
		outBuf.reserve(size());
		
		outBuf.insert(outBuf.end(), (uint8_t*)header, (uint8_t*)header + sizeof(PacketHeader));
		outBuf.insert(outBuf.end(), data, data + dataSize);

		return outBuf;
	}

};


// Null character is included
struct clientConnect {
	uint8_t username[16];
	uint8_t password[16];
};

struct clientBroadMsg {
	uint8_t* msg;
};

struct clientServMsg {
	uint8_t* msg;
};
