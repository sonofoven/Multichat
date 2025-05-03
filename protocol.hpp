
enum opcode {
	CMG_CONNECT,
	CMG_MSG,
	CMG_SERVMSG,
	CMG_MSGBULK,
	CMG_DISCONNECT
};

struct PacketHeader {
	uint16_t length;
	uint16_t opcode;
};

/*
struct Packet {
	PacketHeader header;
	vector<uint16_t> data;
};
*/

#define PORT 8080
#define CHUNK 4096


