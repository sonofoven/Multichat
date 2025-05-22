#include "protocol.hpp"
#include <endian.h>
#include <vector>
#include <cstdint>

Packet::~Packet() = default;
template <typename T>

//// Helper functions for buffer transfers ////

void pushBack(vector<uint8_t>& buffer, T additive){
	const uint8_t* addPtr = (const uint8_t*)&additive;

	buffer.insert(buffer.end(), addPtr, addPtr + sizeof(T));
}

void pushUsernameBack(vector<uint8_t>& buffer, const char* username){
	const uint8_t* usrPtr = (const uint8_t*)username;
	
	buffer.insert(buffer.end(), usrPtr, usrPtr + strlen(username) + 1);
}

void pushLenBack(vector<uint8_t>& buffer, const char* message, size_t messageLength){
	const uint8_t* msgPtr = (const uint8_t*)message;
	
	buffer.insert(buffer.end(), msgPtr, msgPtr + messageLength);
}


//// Client -> Server ////

// Client Connection

void ClientConnect::parse(const uint8_t* data) {
	// This is done so the uint16_t is aligned (for portability)
	uint16_t leLen;
	memcpy(&leLen, data, sizeof(leLen));
	length = le16toh(leLen);
	
	uint16_t leOp;
	memcpy(&leOp, data + sizeof(leLen), sizeof(leOp));
	opcode = le16toh(leOp);

	username = (const char*)(data + headerLen);
}

void ClientConnect::serialize(vector<uint8_t>& buffer) {
	pushBack(buffer, htole16(length));
	pushBack(buffer, htole16(opcode));
	pushUsernameBack(buffer, username);
}

ClientConnect::ClientConnect(char* usr){
	// + 1 for null byte
	length = headerLen + strlen(usr) + 1;
	opcode = CMG_CONNECT;

	username = (const char*) usr;
}

ClientConnect::ClientConnect(){}

// Client Broadcast Message

void ClientBroadMsg::parse(const uint8_t* data) {
	uint16_t leLen;
	memcpy(&leLen, data, sizeof(leLen));
	length = le16toh(leLen);
	
	uint16_t leOp;
	memcpy(&leOp, data + sizeof(leLen), sizeof(leOp));
	opcode = le16toh(leOp);
	
	msg = (const char*)(data + headerLen);
	msgLen = length - headerLen;
}

void ClientBroadMsg::serialize(vector<uint8_t>& buffer) {
	pushBack(buffer, htole16(length));
	pushBack(buffer, htole16(opcode));

	size_t messageLength = (size_t)length - headerLen;
	pushLenBack(buffer, msg, messageLength);
}

ClientBroadMsg::ClientBroadMsg(size_t messageLen, char* message){
	length = headerLen + messageLen;
	opcode = CMG_BROADMSG;

	msgLen = messageLen;
	msg = (const char*)message;
}

ClientBroadMsg::ClientBroadMsg(){}

// Client to Server message (for debug/logging)

void ClientServMsg::parse(const uint8_t* data) {
	uint16_t leLen;
	memcpy(&leLen, data, sizeof(leLen));
	length = le16toh(leLen);
	
	uint16_t leOp;
	memcpy(&leOp, data + sizeof(leLen), sizeof(leOp));
	opcode = le16toh(leOp);

	msg = (const char*)(data + headerLen);
	msgLen = length - headerLen;
}

void ClientServMsg::serialize(vector<uint8_t>& buffer) {
	pushBack(buffer, htole16(length));
	pushBack(buffer, htole16(opcode));

	size_t messageLength = (size_t)length - headerLen;
	pushLenBack(buffer, msg, messageLength);
}

ClientServMsg::ClientServMsg(){}

// Client Disconnect

void ClientDisconnect::parse(const uint8_t* data) {
	uint16_t leLen;
	memcpy(&leLen, data, sizeof(leLen));
	length = le16toh(leLen);
	
	uint16_t leOp;
	memcpy(&leOp, data + sizeof(leLen), sizeof(leOp));
	opcode = le16toh(leOp);
}

void ClientDisconnect::serialize(vector<uint8_t>& buffer) {
	pushBack(buffer, htole16(length));
	pushBack(buffer, htole16(opcode));
}

ClientDisconnect::ClientDisconnect(){
	length = headerLen;
	opcode = CMG_DISCONNECT;
}


//// Server -> Client ////

// Server Validation

void ServerValidate::parse(const uint8_t* data){
	uint16_t leLen;
	memcpy(&leLen, data, sizeof(leLen));
	length = le16toh(leLen);
	
	uint16_t leOp;
	memcpy(&leOp, data + sizeof(leLen), sizeof(leOp));
	opcode = le16toh(leOp);

	able = *(bool*)(data + sizeof(length) + sizeof(opcode));
}

void ServerValidate::serialize(vector<uint8_t>& buffer) {
	pushBack(buffer, htole16(length));
	pushBack(buffer, htole16(opcode));
	pushBack(buffer, able);
}

ServerValidate::ServerValidate(bool a){
	length = headerLen + sizeof(a);
	opcode = SMG_VALIDATE;

	able = a;
}

ServerValidate::ServerValidate(){}

// Server Connection

void ServerConnect::parse(const uint8_t* data) {
	uint16_t leLen;
	memcpy(&leLen, data, sizeof(leLen));
	length = le16toh(leLen);
	
	uint16_t leOp;
	memcpy(&leOp, data + sizeof(leLen), sizeof(leOp));
	opcode = le16toh(leOp);

	username = (const char*)(data + headerLen);
}

void ServerConnect::serialize(vector<uint8_t>& buffer) {
	pushBack(buffer, htole16(length));
	pushBack(buffer, htole16(opcode));
	pushUsernameBack(buffer, username);
}

ServerConnect::ServerConnect(char* usr){
	length = headerLen + strlen(usr) + 1;
	opcode = SMG_CONNECT;

	username = (const char*)usr;
}

ServerConnect::ServerConnect(){}

// Server Broadcast Message

void ServerBroadMsg::parse(const uint8_t* data) {

	uint16_t leLen;
	memcpy(&leLen, data, sizeof(leLen));
	length = le16toh(leLen);
	
	uint16_t leOp;
	memcpy(&leOp, data + sizeof(leLen), sizeof(leOp));
	opcode = le16toh(leOp);

	username = (const char*)(data + headerLen);
	size_t userLen = strlen(username) + 1;

	msg = (const char*)(username + userLen);
	msgLen = length - headerLen - userLen;
}

void ServerBroadMsg::serialize(vector<uint8_t>& buffer) {
	pushBack(buffer, htole16(length));
	pushBack(buffer, htole16(opcode));
	pushUsernameBack(buffer, username);

	pushLenBack(buffer, msg, msgLen);
}

ServerBroadMsg::ServerBroadMsg(char* usr, size_t messageLen, char* message){
	length = headerLen + strlen(usr) + 1 + messageLen;
	opcode = SMG_BROADMSG;

	username = (const char*)usr;
	
	msgLen = messageLen;
	msg = (const char*)message;
}

ServerBroadMsg::ServerBroadMsg(){}

// Server Disconnect

void ServerDisconnect::parse(const uint8_t* data) {
	uint16_t leLen;
	memcpy(&leLen, data, sizeof(leLen));
	length = le16toh(leLen);
	
	uint16_t leOp;
	memcpy(&leOp, data + sizeof(leLen), sizeof(leOp));
	opcode = le16toh(leOp);

	username = (const char*)(data + headerLen);
}

void ServerDisconnect::serialize(vector<uint8_t>& buffer) {
	pushBack(buffer, htole16(length));
	pushBack(buffer, htole16(opcode));
	pushUsernameBack(buffer, username);
}

ServerDisconnect::ServerDisconnect(char* usr){
	length = headerLen + strlen(usr) + 1;
	opcode = SMG_DISCONNECT;

	username = (const char*)usr;
}

ServerDisconnect::ServerDisconnect(){}

// Pointer to a func that takes no args and return type is Packet*

using PacketFactory = Packet* (*)();  

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
	uint16_t leOp;
	memcpy(&leOp, data + sizeof(uint16_t), sizeof(leOp));
	uint16_t opcode = le16toh(leOp);

	if (opcode >= NUM_OF_OPCODES || packetFactories[opcode] == nullptr){
		return nullptr;
	}

	PacketFactory factory = packetFactories[opcode]; // Set up factory depending on opcode;

	Packet* pkt = factory(); // Make new packet based on opcode set factory

	pkt->parse(data);

	// The real type is still "hidden" as Packet* and will have to be cast as its proper class

	return pkt;
}
