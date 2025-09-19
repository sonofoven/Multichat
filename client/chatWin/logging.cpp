#include "client.hpp"
#include "interface.hpp"

void ChatContext::startLog(){
	stopLog = false;
	logT(logLoop);
	logT.detach();
}

void ChatContext::stopLog(){
	stopLog = true;
	logT.join();
}

void ChatContext::restoreHistory(){
	// Find logs
	list<path> logFiles = detectLogFiles();

	if (logFiles.empty()){
		return;
	}

	// Sort them (reverse order)
	logFiles.sort(less<path>());

	shared_lock<shared_mutex> lock(fileMtx);
	// Iterate log file
	for (const path & logFile : logFiles){
		if (!exists(logFile)){
			continue;
		}

		ifstream log(logFile);
		if (!log.is_open()){
			continue;
		}

		string line;

		while (getline(log, line)){
			uint8_t* data = (uint8_t*)line.c_str();
			Packet* linePtr = instancePacketFromData(data);

			ServerBroadMsg servPacket = *(static_cast<ServerBroadMsg*>(linePtr));

			unique_ptr<formMsg> formattedStr = formatMessage(servPacket.timestamp, servPacket.msg, servPacket.username);
			appendMsgWin(formattedStr, false);
		}

		log.close();
	}
}

void ChatContext::logLoop(){
	list<path> logFiles = detectLogFiles();
	logFiles.sort(greater<path>());
	weenLogFiles(logFiles);
	while(!chatContext.stopLog){
		unique_lock lock(logMtx);

		// Locks up here and releases the mutex
		while (logQueue.empty()){
			queueCv.wait(lock);
		}

		// Move the queue over and then release it to get more packets
		queue<unique_ptr<Packet>> localQueue = move(logQueue);
		lock.unlock();
		
		while (!localQueue.empty()){
			unique_ptr<Packet> pkt = move(localQueue.front());
			
			vector<uint8_t> logMsg;
			switch (pkt->opcode){

				case SMG_BROADMSG: {
					ServerBroadMsg msg = *(ServerBroadMsg*)pkt.get();
					msg.serialize(logMsg);
					break;
				}

				default: {
					break;
				}
			}

			string logStr(logMsg.begin(), logMsg.end());
			addToLog(logStr, logFiles);
			
			localQueue.pop();
		}
	}
}


void ChatContext::appendToLog(unique_ptr<Packet> pkt){
	lock_guard lock(logMtx);
	logQueue.push(move(pkt));
	queueCv.notify_one();
}


