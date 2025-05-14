#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <vector>
#include <string>
#include "../protocol.hpp"


using namespace std;

void sendPacket(int servFd, uint8_t* packet, size_t packetLen);

vector<uint8_t> getTextInput();

PacketHeader makeHeader(opcode code, size_t stringLen);
