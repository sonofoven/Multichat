#include <cstdint>
#include <cstring>
#include <cstddef>
#include <vector>

#define PORT    8080
#define CHUNK   4096
#define MAXMSG  512
#define NAMELEN 16

using std::vector;

enum opcode {
	CMG_CONNECT, // Connect and auth
	CMG_BROADMSG, // Message from one client to every client
	CMG_SERVMSG, // Message directly to the server
	CMG_DISCONNECT, // Disconnect
	
	SMG_VALIDATE, // Message back to client saying they're good
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

class ClientConnect : public Packet {
public:
    const char* username;

    void parse(const uint8_t* data) override;
    void serialize(vector<uint8_t>& buffer) override;

	ClientConnect();
	ClientConnect(const char* usr);
};

class ClientBroadMsg : public Packet {
public:
    const char* msg;
	size_t msgLen;

    void parse(const uint8_t* data) override;
    void serialize(vector<uint8_t>& buffer) override;

	ClientBroadMsg();
	ClientBroadMsg(size_t messageLen, char* message);
};

class ClientServMsg : public Packet {
public:
    const char* msg;
	size_t msgLen;

    void parse(const uint8_t* data) override;
    void serialize(vector<uint8_t>& buffer) override;

	ClientServMsg();
	ClientServMsg(size_t messageLen, char* message);
};

class ClientDisconnect : public Packet {
public:
    void parse(const uint8_t* data) override;
    void serialize(vector<uint8_t>& buffer) override;

	ClientDisconnect();
};

// Server -> Client

class ServerValidate : public Packet {
public:
    bool able;

    void parse(const uint8_t* data) override;
    void serialize(vector<uint8_t>& buffer) override;

	ServerValidate();
	ServerValidate(bool a);
};

class ServerConnect : public Packet {
public:
    const char* username;

    void parse(const uint8_t* data) override;
    void serialize(vector<uint8_t>& buffer) override;

	ServerConnect();
	ServerConnect(const char* usr);
};

class ServerBroadMsg : public Packet {
public:
    const char* username;
    const char* msg;
	size_t msgLen;

    void parse(const uint8_t* data) override;
    void serialize(vector<uint8_t>& buffer) override;

	ServerBroadMsg();
	ServerBroadMsg(const char* usr, size_t messageLen, char* message);
};

class ServerDisconnect : public Packet {
public:
    const char* username;

    void parse(const uint8_t* data) override;
    void serialize(vector<uint8_t>& buffer) override;

	ServerDisconnect();
	ServerDisconnect(const char* usr);
};

// Helper functions for buffer loading

template <typename T>
void pushBack(vector<uint8_t>& buffer, T additive);
void pushUsernameBack(vector<uint8_t>& buffer, const char* username);
void pushLenBack(vector<uint8_t>& buffer, const char* message, size_t msgLength);


// factory table
using PacketFactory = Packet* (*)();
extern PacketFactory packetFactories[NUM_OF_OPCODES];
void registerPackets();
Packet* instancePacketFromData(const uint8_t* data);
