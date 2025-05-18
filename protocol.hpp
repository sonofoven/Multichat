#include <cstdint>
#include <cstring>
#include <cstddef>

#define PORT    8080
#define CHUNK   4096
#define MAXMSG  512
#define NAMELEN 16

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
    virtual void parse(const uint8_t* data) = 0;
    virtual ~Packet();
};

// Client -> Server

class ClientConnect : public Packet {
public:
    uint16_t length;
    uint16_t opcode;
    const char* username;
    void parse(const uint8_t* data) override;
};

class ClientBroadMsg : public Packet {
public:
    uint16_t length;
    uint16_t opcode;
    const char* msg;
    void parse(const uint8_t* data) override;
};

class ClientServMsg : public Packet {
public:
    uint16_t length;
    uint16_t opcode;
    const char* msg;
    void parse(const uint8_t* data) override;
};

class ClientDisconnect : public Packet {
public:
    uint16_t length;
    uint16_t opcode;
    const char* username;
    void parse(const uint8_t* data) override;
};

// Server -> Client

class ServerValidate : public Packet {
public:
    uint16_t length;
    uint16_t opcode;
    bool able;
    void parse(const uint8_t* data) override;
};

class ServerConnect : public Packet {
public:
    uint16_t length;
    uint16_t opcode;
    const char* username;
    void parse(const uint8_t* data) override;
};

class ServerBroadMsg : public Packet {
public:
    uint16_t length;
    uint16_t opcode;
    const char* username;
    const char* msg;
    void parse(const uint8_t* data) override;
};

class ServerDisconnect : public Packet {
public:
    uint16_t length;
    uint16_t opcode;
    const char* username;
    void parse(const uint8_t* data) override;
};


// factory table
using PacketFactory = Packet* (*)();
extern PacketFactory packetFactories[NUM_OF_OPCODES];
void registerPackets();
Packet* instancePacketFromData(const uint8_t* data);
