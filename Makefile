CC      := g++
CFLAGS  := -Wall -g -fPIC -pedantic

CLIENT_DIR := client
SERVER_DIR := server

CLIENT_SRCS := $(CLIENT_DIR)/client.cpp $(CLIENT_DIR)/interface.cpp 
SERVER_SRCS := $(SERVER_DIR)/server.cpp $(SERVER_DIR)/servUtils.cpp

CLIENT_HDRS := protocol.hpp $(CLIENT_DIR)/client.hpp
SERVER_HDRS := protocol.hpp $(SERVER_DIR)/server.hpp

CLIENT_BIN := $(CLIENT_DIR)/client
SERVER_BIN := $(SERVER_DIR)/server

LINK_CLIENT := cli
LINK_SERVER := serv

.PHONY: all client server clean link

all: client server link

client: $(CLIENT_BIN)

server: $(SERVER_BIN)

$(CLIENT_BIN): $(CLIENT_SRCS) $(CLIENT_HDRS)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_SRCS) -lncurses

$(SERVER_BIN): $(SERVER_SRCS) $(SERVER_HDRS)
	$(CC) $(CFLAGS) -o $@ $(SERVER_SRCS)

link:
	ln -sf $(abspath $(CLIENT_BIN)) $(LINK_CLIENT)
	ln -sf $(abspath $(SERVER_BIN)) $(LINK_SERVER)

clean:
	rm -f $(CLIENT_BIN) $(SERVER_BIN) $(LINK_CLIENT) $(LINK_SERVER)
