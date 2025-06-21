#include "server.hpp"

void logLoop(int logFd){
	list<string> logFiles = ;
	while(1){
		unique_lock lock(logMtx);

		// Locks up here and releases the mutex
		while (!queue.empty()){
			queueCv.wait(lock);
		}

		// Move the queue over and then release it to get more packets
		queue<unique_ptr<Packet>> localQueue = move(logQueue);
		lock.unlock();
		
		while (!localQueue.empty()){
			unique_ptr<Packet> pkt = localQueue.front();
			
			vector<uint8_t> logMsg;
			switch (pkt->opcode){

				case SMG_CONNECT: {
					(ServerConnect*)pkt.get();
					ServerConnect msg = *pkt;
					msg.serialize(logMsg);
					break;
				}

				case SMG_BROADMSG: {
					(ServerDisconnect*)pkt.get();
					ServerDisconnect msg = *pkt;
					msg.serialize(logMsg);
					break;
				}

				case SMG_DISCONNECT: {
					(ServerBroadMsg*)pkt.get();
					ServerBroadMsg msg = *pkt;
					msg.serialize(logMsg);
					break;
				}

				default: {
					break;
				}
			

			localQueue.pop();
		}
	}
}

string logFileName(){
	time_t timestamp = time(NULL);

}

void addToFile(string str){

}

list<string> detectLogFiles(){

}
