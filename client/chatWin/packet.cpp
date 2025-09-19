#include "interface.hpp"

int ChatContext::protocolParser(Packet* pkt){ 
	int exitCode = 0;
	uint16_t opcode = pkt->opcode;

	switch(opcode){
		case SMG_VALIDATE: {

			ServerValidate* serverPacket = static_cast<ServerValidate*>(pkt);

			serverValidate(*serverPacket);
			break;
		}

		case SMG_CONNECT: {

			ServerConnect* serverPacket = static_cast<ServerConnect*>(pkt);

			serverConnect(*serverPacket);

			break;
		}

		case SMG_BROADMSG: {

			ServerBroadMsg* serverPacket = static_cast<ServerBroadMsg*>(pkt);

			serverBroadMsg(*serverPacket);

			break;
		}


		case SMG_DISCONNECT: {

			ServerDisconnect* serverPacket = static_cast<ServerDisconnect*>(pkt);

			serverDisconnect(*serverPacket);

			break;
		}

		default: {
			exitCode = -1;
		}
	}
	return exitCode;
}

void ChatContext::serverValidate(ServerValidate& pkt){
	// YOU SHOULD NEVER GET THIS WHILE RUNNING only in startup
	// You can use this in the future to occasionally send out sync's

	// Check if got back good boy mark
	if (!pkt.able){
		// Push back to set up form here
		exit(0);
	}

	// Get and update userconn list

	// This could be an issue with the move
	userConns = move(pkt.userList);
	updateUserWindow();
}

void ChatContext::serverConnect(ServerConnect& pkt){
	// Informs the user about client disconnect
	string username(pkt.username);

	userConns.push_back(username);

	userConns.sort(greater<string>());
	unique_ptr<formMsg> formattedStr = formatConMessage(pkt.timestamp, pkt.username);
	
	appendMsgWin(formattedStr, false);

	// Update the users window
	updateUserWindow();
}

void ChatContext::serverBroadMsg(ServerBroadMsg& pkt){
	unique_ptr<formMsg> formattedStr = formatMessage(pkt.timestamp, pkt.msg, pkt.username);
	
	unique_ptr<ServerBroadMsg> upkt = make_unique<ServerBroadMsg>(pkt);
	appendToLog(move(upkt));

	appendMsgWin(formattedStr, false);
}

void ChatContext::serverDisconnect(ServerDisconnect& pkt){
	// Informs the user about client disconnect
	string username(pkt.username);

	userConns.remove(username);

	// Sort in alphabetical order (this is because how its printed)
	userConns.sort(greater<string>());

	unique_ptr<formMsg> formattedStr = formatDisMessage(pkt.timestamp, pkt.username);

	appendMsgWin(formattedStr, false);

	// Update the users window
	updateUserWindow();
}
