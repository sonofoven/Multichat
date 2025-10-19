#pragma once

#include <cstdint>
#include <cstring>
#include <cstddef>
#include <vector>
#include <ctime>
#include <iostream>
#include <string>
#include <list>
#include <unordered_map>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

#define PORT	8080
#define CHUNK   4096
#define MAXMSG  512
#define NAMELEN 16
#define MAX_EVENTS 30

using std::vector;
using std::unordered_map;
using std::string;
using std::list;

enum opcode {
	CMG_CONNECT, // Connect and auth
	CMG_BROADMSG, // Message from one client to every client
	CMG_SERVMSG, // Message directly to the server
	
	SMG_VALIDATE, // Message back to client saying they good, give the whole user list
	SMG_CONNECT, // Message to all other clients client has connected
	SMG_BROADMSG, // Message from one client to every client
	SMG_DISCONNECT, // Let other clients know client disconnected
	NUM_OF_OPCODES
};

// Base

class Packet {
public:
	// Header
	uint16_t length;
	uint16_t opcode;

	static constexpr size_t headerLen = sizeof(length) + sizeof(opcode);

	virtual void parse(const uint8_t* data) = 0;
	virtual void serialize(vector<uint8_t>& buffer) = 0;
	virtual ~Packet();
};

// Client -> Server

class ClientConnect : public Packet { ///
public:
	time_t timestamp;
	string username;

	void parse(const uint8_t* data) override;
	void serialize(vector<uint8_t>& buffer) override;

	ClientConnect();
	ClientConnect(string& usr, time_t msgTime);
};

class ClientBroadMsg : public Packet { ///
public:
	string msg;

	void parse(const uint8_t* data) override;
	void serialize(vector<uint8_t>& buffer) override;

	ClientBroadMsg();
	ClientBroadMsg(string& message);
};

class ClientServMsg : public Packet { ///
public:
	string msg;

	void parse(const uint8_t* data) override;
	void serialize(vector<uint8_t>& buffer) override;

	ClientServMsg();
	ClientServMsg(string& message);
};

// Server -> Client

class ServerValidate : public Packet { ///
public:
	bool able;
	string servName;
	list<string> userList;

	void parse(const uint8_t* data) override;
	void serialize(vector<uint8_t>& buffer) override;

	ServerValidate();
	ServerValidate(bool a, string servName, unordered_map<string,int>& userMap);
};

class ServerConnect : public Packet { ///
public:
	time_t timestamp;
	string username;

	void parse(const uint8_t* data) override;
	void serialize(vector<uint8_t>& buffer) override;

	ServerConnect();
	ServerConnect(string& usr);
};

class ServerBroadMsg : public Packet { ///
public:
	time_t timestamp;
	string username;
	string msg;

	void parse(const uint8_t* data) override;
	void serialize(vector<uint8_t>& buffer) override;

	ServerBroadMsg();
	ServerBroadMsg(string& usr, string& message);
};

class ServerDisconnect : public Packet {
public:
	time_t timestamp;
	string username;

	void parse(const uint8_t* data) override;
	void serialize(vector<uint8_t>& buffer) override;

	ServerDisconnect();
	ServerDisconnect(string& usr);
};

// Helper functions for buffer serialization

template <typename T>
void pushBack(vector<uint8_t>& buffer, T additive);
void pushStringBack(vector<uint8_t>& buffer, string username);
void pushLenBack(vector<uint8_t>& buffer, string message);
void pushListBack(vector<uint8_t>& buffer, list<string>& userList);

Packet* instancePacketFromData(const uint8_t* data);

size_t parsePacketLen(uint8_t* data);
void epollModify(int epollFd, int fd, int ops, int func);

