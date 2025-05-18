#include "protocol.hpp"
#include <endian.h>
#include <vector>
#include <cstdint>

void ClientConnect::parse(const uint8_t* data) {
	length = le16toh(*(uint16_t*)data);
	opcode = le16toh(*(uint16_t*)(data + sizeof(length)));

	// Get pointer to message
	constexpr size_t header = sizeof(length) + sizeof(opcode);
	username = (const char*)(data + header);
}

void ClientBroadMsg::parse(const uint8_t* data) {
	length = le16toh(*(uint16_t*)data);
	opcode = le16toh(*(uint16_t*)(data + sizeof(length)));
	
	constexpr size_t header = sizeof(length) + sizeof(opcode);
	msg = (const char*)(data + header);
}

void ClientServMsg::parse(const uint8_t* data) {
	length = le16toh(*(uint16_t*)data);
	opcode = le16toh(*(uint16_t*)(data + sizeof(length)));

	constexpr size_t header = sizeof(length) + sizeof(opcode);
	msg = (const char*)(data + header);
}

void ClientDisconnect::parse(const uint8_t* data) {
	length = le16toh(*(uint16_t*)data);
	opcode = le16toh(*(uint16_t*)(data + sizeof(length)));
}

void ServerValidate::parse(const uint8_t* data){
	length = le16toh(*(uint16_t*)data);
	opcode = le16toh(*(uint16_t*)(data + sizeof(length)));
	able = *(bool*)(data + sizeof(length) + sizeof(opcode));
}

void ServerConnect::parse(const uint8_t* data) {
	length = le16toh(*(uint16_t*)data);
	opcode = le16toh(*(uint16_t*)(data + sizeof(length)));

	constexpr size_t header = sizeof(length) + sizeof(opcode);
	username = (const char*)(data + header);
}

void ServerBroadMsg::parse(const uint8_t* data) {
	length = le16toh(*(uint16_t*)data);
	opcode = le16toh(*(uint16_t*)(data + sizeof(length)));
	
	constexpr size_t header = sizeof(length) + sizeof(opcode);
	username = (const char*)(data + header);
	msg = (const char*)(username + strlen(username) + 1);
}

void ServerDisconnect::parse(const uint8_t* data) {
	length = le16toh(*(uint16_t*)data);
	opcode = le16toh(*(uint16_t*)(data + sizeof(length)));

	constexpr size_t header = sizeof(length) + sizeof(opcode);
	username = (const char*)(data + header);
}

using PacketFactory = Packet* (*)();  // Pointer to a func
									 // That takes no args
									 // and return type is Packet*

Packet* createCConnPacket() { return new ClientConnect(); }
Packet* createCBroadPacket() { return new ClientBroadMsg(); }
Packet* createCServPacket() { return new ClientServMsg(); }
Packet* createCDisPacket() { return new ClientDisconnect(); }

Packet* createSValPacket() { return new ServerValidate(); }
Packet* createSConnPacket() { return new ServerConnect(); }
Packet* createSBroadPacket() { return new ServerBroadMsg(); }
Packet* createSDisPacket() { return new ServerDisconnect(); }

PacketFactory packetFactories[NUM_OF_OPCODES] = { 0 };

void registerPackets() {
	packetFactories[CMG_CONNECT] = createCConnPacket;
	packetFactories[CMG_BROADMSG] = createCBroadPacket;
	packetFactories[CMG_SERVMSG] = createCServPacket;
	packetFactories[CMG_DISCONNECT] = createCDisPacket;

	packetFactories[SMG_VALIDATE] = createSValPacket;
	packetFactories[SMG_CONNECT] = createSConnPacket;
	packetFactories[SMG_BROADMSG] = createSBroadPacket;
	packetFactories[SMG_DISCONNECT] = createSDisPacket;

}


Packet* instancePacketFromData(const uint8_t* data){
	uint16_t opcode = *(uint16_t*)(data + sizeof(uint16_t));

	PacketFactory factory = packetFactories[opcode]; // Set up factory depending on opcode;

	Packet* pkt = factory(); // Make new packet based on opcode set factory

	pkt->parse(data);

	return pkt;
}
