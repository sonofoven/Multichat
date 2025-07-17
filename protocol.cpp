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

void pushStrBack(vector<uint8_t>& buffer, string& str){
	buffer.insert(buffer.end(), str.begin(), str.end());
	buffer.push_back('\0');
}

void pushListBack(vector<uint8_t>& buffer, list<string>& userList){
	for (auto i : userList){
		pushStrBack(buffer, i);
	}
}

//// Client -> Server ////
// Client Connection

void ClientConnect::parse(const uint8_t* data) {
	length = le16toh(*(uint16_t*)data);
	opcode = le16toh(*(uint16_t*)(data + sizeof(length)));

	username = (char*)(data + headerLen);
}

void ClientConnect::serialize(vector<uint8_t>& buffer) {
	pushBack(buffer, htole16(length));
	pushBack(buffer, htole16(opcode));
	pushStrBack(buffer, username);
}

ClientConnect::ClientConnect(string& usr){
	// + 1 for null byte
	length = headerLen + usr.size() + 1;
	opcode = CMG_CONNECT;

	username = usr;
}

ClientConnect::ClientConnect(){}

// Client Broadcast Message

void ClientBroadMsg::parse(const uint8_t* data) {
	length = le16toh(*(uint16_t*)data);
	opcode = le16toh(*(uint16_t*)(data + sizeof(length)));
	
	msg = (char*)(data + headerLen);
}

void ClientBroadMsg::serialize(vector<uint8_t>& buffer) {
	pushBack(buffer, htole16(length));
	pushBack(buffer, htole16(opcode));
	pushStrBack(buffer, msg);
}

ClientBroadMsg::ClientBroadMsg(string& message){
	length = headerLen + message.size() + 1;
	opcode = CMG_BROADMSG;

	msg = message;
}

ClientBroadMsg::ClientBroadMsg(){}

// Client to Server message (for debug/logging/API)

void ClientServMsg::parse(const uint8_t* data) {
	length = le16toh(*(uint16_t*)data);
	opcode = le16toh(*(uint16_t*)(data + sizeof(length)));

	msg = (char*)(data + headerLen);
}

void ClientServMsg::serialize(vector<uint8_t>& buffer) {
	pushBack(buffer, htole16(length));
	pushBack(buffer, htole16(opcode));
	pushStrBack(buffer, msg);
}

ClientServMsg::ClientServMsg(string& message){
	length = headerLen + message.size();
	opcode = CMG_SERVMSG;

	msg = message;
}

ClientServMsg::ClientServMsg(){}

//// Server -> Client ////

// Server Validation

void ServerValidate::parse(const uint8_t* data){
	length = le16toh(*(uint16_t*)data);
	opcode = le16toh(*(uint16_t*)(data + sizeof(length)));

	able = *(bool*)(data + headerLen);
	servName = (char*)(data + headerLen + sizeof(able));

	// Add all the strings if client is able to connect
	if (able == true){
		// Set the start point of username list
		char* curPtr = (char*)data + headerLen + sizeof(able) + servName.size() + 1;
		char* endLoc = (char*)data + length;
		void* distChk;

		while (curPtr < endLoc){ // While we are within the packet

			// Check to make sure that next null is within the end
			distChk = memchr(curPtr, 0, endLoc - curPtr);

			if (distChk == nullptr){ // If next null is past end
				break;
			}

			string newStr(curPtr);
			userList.push_back(newStr);

			curPtr = (char*)distChk + 1; 
		}
	}
}

void ServerValidate::serialize(vector<uint8_t>& buffer) {
	pushBack(buffer, htole16(length));
	pushBack(buffer, htole16(opcode));
	pushStrBack(buffer, servName);
	pushBack(buffer, able);
	if (able == true || !userList.empty()){
		pushListBack(buffer, userList);
	}
}

ServerValidate::ServerValidate(bool a, string name, unordered_map<string, int>& userMap){
	length = headerLen + sizeof(a) + servName.size() + 1;
	opcode = SMG_VALIDATE;

	able = a;
	servName = name;
	if (able == true){ // Only add the map if the conn is valid
		for (auto i : userMap){
			length += i.first.size() + 1;
			userList.push_back(i.first);
		}
	}
}

ServerValidate::ServerValidate(){}

// Server Connection

void ServerConnect::parse(const uint8_t* data) {
	length = le16toh(*(uint16_t*)data);
	opcode = le16toh(*(uint16_t*)(data + sizeof(length)));
	timestamp = le64toh(*(time_t*)(data + headerLen));

	username = (char*)(data + headerLen + sizeof(time_t));
}

void ServerConnect::serialize(vector<uint8_t>& buffer) {
	pushBack(buffer, htole16(length));
	pushBack(buffer, htole16(opcode));
	pushBack(buffer, htole64(timestamp));
	pushStrBack(buffer, username);
}

ServerConnect::ServerConnect(string& usr){
	time(&timestamp);
	length = headerLen + usr.size() + sizeof(timestamp) + 1;
	opcode = SMG_CONNECT;

	username = usr;
}

ServerConnect::ServerConnect(){}

// Server Broadcast Message

void ServerBroadMsg::parse(const uint8_t* data) {
	length = le16toh(*(uint16_t*)data);
	opcode = le16toh(*(uint16_t*)(data + sizeof(length)));
	timestamp = le64toh(*(time_t*)(data + headerLen));

	char* usrPtr = (char*)(data + headerLen + sizeof(time_t));
	username = usrPtr;
	msg = usrPtr + username.size() + 1;
}

void ServerBroadMsg::serialize(vector<uint8_t>& buffer) {
	pushBack(buffer, htole16(length));
	pushBack(buffer, htole16(opcode));
	pushBack(buffer, htole64(timestamp));
	pushStrBack(buffer, username);
	pushStrBack(buffer, msg);
}

ServerBroadMsg::ServerBroadMsg(string& usr, string& message){
	time(&timestamp);
	length = headerLen + usr.size() + message.size() + sizeof(timestamp) + 2;
	opcode = SMG_BROADMSG;

	username = usr;
	
	msg = message;
}

ServerBroadMsg::ServerBroadMsg(){}

// Server Disconnect

void ServerDisconnect::parse(const uint8_t* data) {
	length = le16toh(*(uint16_t*)data);
	opcode = le16toh(*(uint16_t*)(data + sizeof(length)));
	timestamp = le64toh(*(time_t*)(data + headerLen));

	username = (char*)(data + headerLen + sizeof(time_t));
}

void ServerDisconnect::serialize(vector<uint8_t>& buffer) {
	pushBack(buffer, htole16(length));
	pushBack(buffer, htole16(opcode));
	pushBack(buffer, htole64(timestamp));
	pushStrBack(buffer, username);
}

ServerDisconnect::ServerDisconnect(string& usr){
	time(&timestamp);
	length = headerLen + usr.size() + sizeof(timestamp) + 1;
	opcode = SMG_DISCONNECT;

	username = usr;
}

ServerDisconnect::ServerDisconnect(){}

// Pointer to a func that takes no args and return type is Packet*

using PacketFactory = Packet* (*)();  

Packet* createCConnPacket() { return new ClientConnect(); }
Packet* createCBroadPacket() { return new ClientBroadMsg(); }
Packet* createCServPacket() { return new ClientServMsg(); }

Packet* createSValPacket() { return new ServerValidate(); }
Packet* createSConnPacket() { return new ServerConnect(); }
Packet* createSBroadPacket() { return new ServerBroadMsg(); }
Packet* createSDisPacket() { return new ServerDisconnect(); }

PacketFactory packetFactories[NUM_OF_OPCODES] = { 0 };

void registerPackets() {
	packetFactories[CMG_CONNECT] = createCConnPacket;
	packetFactories[CMG_BROADMSG] = createCBroadPacket;
	packetFactories[CMG_SERVMSG] = createCServPacket;

	packetFactories[SMG_VALIDATE] = createSValPacket;
	packetFactories[SMG_CONNECT] = createSConnPacket;
	packetFactories[SMG_BROADMSG] = createSBroadPacket;
	packetFactories[SMG_DISCONNECT] = createSDisPacket;

}


Packet* instancePacketFromData(const uint8_t* data){
	uint16_t opcode = le16toh(*(uint16_t*)(data + sizeof(uint16_t)));

	if (opcode >= NUM_OF_OPCODES || packetFactories[opcode] == nullptr){
		return nullptr;
	}

	PacketFactory factory = packetFactories[opcode]; // Set up factory depending on opcode;

	Packet* pkt = factory(); // Make new packet based on opcode set factory

	pkt->parse(data);

	// The real type is still "hidden" as Packet* and will have to be cast as its proper class

	return pkt;
}

size_t parsePacketLen(uint8_t* data){
	size_t length = le16toh(*(uint16_t*)data);

	return length;
}


void epollModify(int epollFd, int fd, int ops, int func){
	epoll_event event;
	event.events = ops;
	event.data.fd = fd;

	if (epoll_ctl(epollFd, func, fd, &event)){
		perror("Failed to modify fd in epoll");
		close(epollFd);
		exit(1);
	}
}
