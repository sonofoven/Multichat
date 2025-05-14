#include <endian.h>
#include <vector>

#define PORT 8080
#define CHUNK 4096


enum opcode {
	CMG_CONNECT, // Connect and auth
	CMG_MSG, // Message from one client to one client
	CMG_GROUPMSG, // Message from one client to every client in a group
	CMG_BROADMSG, // Message from one client to every client
	CMG_SERVMSG, // Message directly to the server
	CMG_MSGBULK, // Message long enough that it needs to be fragmented
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

struct clientMsg {
	uint8_t recipient[256];
	uint8_t msg[256];
};

struct clientBroadMsg {
	uint8_t msg[256];
};

struct clientServMsg {
	uint8_t msg[256];
};


struct clientGroupMsg {
	uint8_t group[16];
	uint8_t msg[256];
};


