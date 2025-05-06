#include <endian.h>
#include <vector>

#define PORT 8080
#define CHUNK 4096


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

struct Packet {
	PacketHeader header;
	std::vector<uint8_t> data;

	size_t size(){
		return sizeof(PacketHeader) + data.size();
	}

	std::vector<uint8_t> exportData(){
		std::vector<uint8_t> outBuf;
		outBuf.reserve(size());
		
		uint8_t* byteHeader = reinterpret_cast<uint8_t*>(&header);

		outBuf.insert(outBuf.end(), byteHeader, byteHeader + sizeof(header));
		outBuf.insert(outBuf.end(), data.begin(), data.end());

		return outBuf;
	}

	void sendPacket(int fd){
		send(fd, exportData().data(), size(), 0);
		std::cout << "Packet sent" << std::endl;
	}
};
