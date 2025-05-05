#include <endian.h>

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

#define PORT 8080
#define CHUNK 4096
