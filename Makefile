CC	  := g++
CFLAGS  := -Wall -g -fPIC -pedantic

PROTO_SRC := protocol.cpp

CLIENT_DIR := client
SERVER_DIR := server

# General
CLIENT_SRCS := \
  $(CLIENT_DIR)/client.cpp \
  $(CLIENT_DIR)/interface.cpp \
  $(PROTO_SRC)

# ChatWin
CLIENT_SRCS += \
  $(CLIENT_DIR)/chatWin/control.cpp \
  $(CLIENT_DIR)/chatWin/fd.cpp \
  $(CLIENT_DIR)/chatWin/logging.cpp \
  $(CLIENT_DIR)/chatWin/network.cpp \
  $(CLIENT_DIR)/chatWin/packet.cpp \
  $(CLIENT_DIR)/chatWin/redraw.cpp

# WinErr
CLIENT_SRCS += \
  $(CLIENT_DIR)/errWin/winErr.cpp

# FormWin
CLIENT_SRCS += \
  $(CLIENT_DIR)/formWin/control.cpp \
  $(CLIENT_DIR)/formWin/form.cpp

# MenuWin
CLIENT_SRCS += \
  $(CLIENT_DIR)/menuWin/control.cpp \
  $(CLIENT_DIR)/menuWin/menu.cpp

SERVER_SRCS := \
  $(SERVER_DIR)/server.cpp \
  $(SERVER_DIR)/servUtils.cpp \
  $(SERVER_DIR)/logging.cpp \
  $(PROTO_SRC)

CLIENT_HDRS := protocol.hpp $(CLIENT_DIR)/client.hpp $(CLIENT_DIR)/interface.hpp
SERVER_HDRS := protocol.hpp $(SERVER_DIR)/server.hpp

CLIENT_BIN := $(CLIENT_DIR)/client
SERVER_BIN := $(SERVER_DIR)/server

LINK_CLIENT := cli
LINK_SERVER := serv

.PHONY: all client server clean link rmCliLogs rmServLogs cleanAll

all: clean client server link

client: $(CLIENT_BIN)

server: $(SERVER_BIN)

$(CLIENT_BIN): $(CLIENT_SRCS) $(CLIENT_HDRS)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_SRCS) -lform -lmenu -lncurses

$(SERVER_BIN): $(SERVER_SRCS) $(SERVER_HDRS)
	$(CC) $(CFLAGS) -o $@ $(SERVER_SRCS)

link:
	ln -sf $(abspath $(CLIENT_BIN)) $(LINK_CLIENT)
	ln -sf $(abspath $(SERVER_BIN)) $(LINK_SERVER)

cleanAll: clean rmCliLogs rmServLogs

clean: 
	rm -f $(CLIENT_BIN) $(SERVER_BIN) $(LINK_CLIENT) $(LINK_SERVER)

rmCliLogs:
	rm -rf $(HOME)/.multiChat/logs

rmServLogs:
	rm -rf /var/log/multiChat
