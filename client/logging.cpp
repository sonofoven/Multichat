#include "client.hpp"
#include "interface.hpp"

void startLog(){
	reaccLogTarget = true;
	if (!logT.joinable()){
		logT = thread(logLoop);
	}
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

			if (linePtr){
				ServerBroadMsg servPacket = *(static_cast<ServerBroadMsg*>(linePtr));

				unique_ptr<formMsg> formattedStr = formatMessage(servPacket.timestamp, servPacket.msg, servPacket.username);
				appendMsgWin(formattedStr, false);
				delete linePtr;
			}
		}

		log.close();
	}
}

void logLoop(){
	// Block sigwinch as to not interfere w/ main thread

	list<path> logFiles;
	for(;;){
		
		if (reaccLogTarget){
			logFiles = detectLogFiles();
			logFiles.sort(greater<path>());
			weenLogFiles(logFiles);
			reaccLogTarget = false;
		}

		// THIS MIGHT GENERATE SOME DEADLOCKS
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


void appendToLog(unique_ptr<Packet> pkt){
	lock_guard lock(logMtx);
	logQueue.push(move(pkt));
	queueCv.notify_one();
}
