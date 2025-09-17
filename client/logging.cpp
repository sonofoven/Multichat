#include "client.hpp"
#include "interface.hpp"

void logLoop(){
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

list<path> detectLogFiles(){
	// Get the list of fitting logfiles in log dir
	regex pattern(R"(^\d{4}_\d{2}_\d{2}_multiChat\.log$)");
	list<path> logFiles;
	
	shared_lock<shared_mutex> lock(fileMtx);
	for (const auto & entry : directory_iterator(getLogDir())){
		if (entry.is_regular_file()){
			if (regex_match(entry.path().filename().string(), pattern)){
				logFiles.push_back(entry.path());
			}
		}
	}

	return logFiles;
}

void addToLog(string str, list<path>& logFiles){
	// Add to the current or next log file if none exist
	path logFile = logFilePath();

	unique_lock<shared_mutex> lock(fileMtx);
	bool exist = exists(logFile);

	ofstream log(logFile, ios::app);
	if (!log){
		cerr << "Couldn't open log file: " << logFile << endl;
	}

	// Append serialized packet
	log << str << endl;
	log.close();

	logFiles.push_front(logFile);

	if (!exist){
		weenLogFiles(logFiles);
	}
	
	return;
}

void weenLogFiles(list<path>& logFiles){
	// Remove the last log
	while (logFiles.size() > LOG_DAY_MAX){
		if (remove(logFiles.back())){
			logFiles.pop_back();
		} else {
			cerr << "Couldn't remove log: " << logFiles.back() << endl;
			break;
		}
	}

	return;
}

path logFilePath(){
	// Generate file path for the log file based on date
	time_t timestamp = time(NULL);
	tm stampTime = *localtime(&timestamp);

	char outBuf[64];
	strftime(outBuf, sizeof(outBuf), "%Y_%m_%d_multiChat.log", &stampTime);
	path filePath = getLogDir() / outBuf; 

	return filePath;
}

path getLogDir(){
	const char* home = getenv("HOME");
	path logDir = path(home) / STORAGE_DIR / "logs" / clientInfo.addr;
	create_directories(logDir);

	return logDir;
}

void appendToLog(unique_ptr<Packet> pkt){
	lock_guard lock(logMtx);
	logQueue.push(move(pkt));
	queueCv.notify_one();
}


time_t getLatestLoggedMsgTime(){
	// Find logs
	list<path> logFiles = detectLogFiles();
	if (logFiles.empty()){
		return 0;
	}
	logFiles.sort(greater<path>());

	path logPath = logFiles.front();

	if (!exists(logPath)){
		return 0;
	}

	shared_lock<shared_mutex> lock(fileMtx);
	ifstream log(logPath);

	if (!log.is_open()){
		return 0;
	}

	string line, lastLine;

	while (getline(log, line)){
		if (!line.empty()){
			lastLine = line;
		}
	}

	uint8_t* data = (uint8_t*)lastLine.c_str();
	Packet* linePtr = instancePacketFromData(data);

	ServerBroadMsg servPacket = *(static_cast<ServerBroadMsg*>(linePtr));
	return servPacket.timestamp;
}
