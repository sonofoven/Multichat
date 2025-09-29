#include "interface.hpp"

void interfaceStart(){
	initscr();
	noecho();
	cbreak(); 
	curs_set(0);
	refresh();
}

WINDOW* createWindow(int height, int width, int starty, int startx, bool boxOn, bool scroll){
	WINDOW* localWin;
	localWin = newwin(height, width, starty, startx);

	if (boxOn){
		box(localWin, 0, 0);
	}
	
	if (scroll){
		scrollok(localWin, TRUE);
		idlok(localWin, TRUE);
	}

	wrefresh(localWin);

	return localWin;
}


Win* createUserWin(int lines, int cols) {
	int height = lines;
	int width = cols / 6;
	int startY = 0;
	int startX = 0;
	bool boxOn = true;
	bool scrollOn = true;
	string title = "| Users |";

	Win* window = new Win();
	
	window->bordWin = createWindow(height, 
								  width, 
								  startY, 
								  startX, 
								  boxOn, 
								  !scrollOn);

	window->textWin = createWindow(height - (2*VALIGN), 
								  width - (2*HALIGN), 
								  startY + VALIGN, 
								  startX + HALIGN,
								  !boxOn,
								  scrollOn);

	
	int leftPad = (width - title.length())/2;
	mvwprintw(window->bordWin, 0, leftPad, title.c_str());

	wrefresh(window->bordWin);

	return window;
}


MsgWin* createMsgWin(string title, int lines, int cols){
	int height = lines - (lines /6);
	int userWidth = cols/6;
	int width = cols - userWidth;
	int startY = 0;
	int startX = cols / 6;
	bool boxOn = true;
	bool scrollOn = true;

	MsgWin* window = new MsgWin();

	window->bordWin = createWindow(height, 
								  width, 
								  startY, 
								  startX,
								  boxOn, 
								  !scrollOn);

	int padWidth = width - (2*HALIGN);
	int padHeight = PAD_BUF_MULTI * MAX_MSG_BUF;
	window->textWin = newpad(padHeight, 
							 padWidth);

	title = "| "+ title + " |";
	int leftPadding = (width - title.length())/2;
	mvwprintw(window->bordWin, 0, leftPadding, title.c_str());

	wrefresh(window->bordWin);

	return window;
}

Win* createInputWin(int lines, int cols){
	int height = lines / 6;
	int userWidth = cols/6;
	int width = cols - userWidth;
	int startY = lines - (lines /6);
	int startX = cols / 6;
	bool boxOn = true;
	bool scrollOn = true;

	Win* window = new Win();
	
	window->bordWin = createWindow(height, 
								  width, 
								  startY, 
								  startX,
								  boxOn,
								  !scrollOn);

	window->textWin = createWindow(height - (2*VALIGN), 
								  width - (2*HALIGN), 
								  startY + VALIGN, 
								  startX + HALIGN,
								  !boxOn,
								  scrollOn);

	nodelay(window->textWin, TRUE);
	keypad(window->textWin, TRUE);

	return window;
}


unique_ptr<formMsg> formatMessage(time_t time, string& message, string& username){
	unique_ptr<formMsg> outMsg = make_unique<formMsg>();

	vector<chtype>& hdr = outMsg->header;
	vector<chtype>& msg = outMsg->message;

	pushBackStr(formatTime(time), hdr, A_DIM);

	hdr.push_back((chtype)' ' | A_BOLD);
	hdr.push_back((chtype)'[' | A_BOLD);

	// Append username to the beginning of the message in bold
	for (char& ch : username){
		hdr.push_back((chtype)ch | A_BOLD);
	}

	hdr.push_back((chtype)']' | A_BOLD);
	hdr.push_back((chtype)' ');

	// Convert uint8_t message to chtype
	for (char& ch : message){
		msg.push_back((chtype)ch);
	}

	return outMsg;
}

unique_ptr<formMsg> formatDisMessage(time_t time, string& username){
	unique_ptr<formMsg> outMsg = make_unique<formMsg>();

	vector<chtype>& hdr = outMsg->header;
	vector<chtype>& msg = outMsg->message;

	pushBackStr(formatTime(time), hdr, A_DIM);

	hdr.push_back((chtype)' ' | A_BOLD);

	pushBackStr("<---  ", msg, A_BOLD);

	pushBackStr(username, msg, A_BOLD);

	pushBackStr(" has disconnected  --->", msg, A_BOLD);

	return outMsg;
}

unique_ptr<formMsg> formatConMessage(time_t time, string& username){
	unique_ptr<formMsg> outMsg = make_unique<formMsg>();

	vector<chtype>& hdr = outMsg->header;
	vector<chtype>& msg = outMsg->message;
	pushBackStr(formatTime(time), hdr, A_DIM);

	hdr.push_back((chtype)' ' | A_BOLD);

	pushBackStr("<---  ", msg, A_BOLD);

	pushBackStr(username, msg, A_BOLD);

	pushBackStr(" has connected  --->", msg, A_BOLD);

	return outMsg;
}

string formatTime(time_t timestamp){
	tm stampTime = *localtime(&timestamp);
	
	time_t curTime = time(NULL);
	tm currentTime = *localtime(&curTime);

	char outBuf[64];
	string str;

	if (currentTime.tm_year != stampTime.tm_year){
		// Different year
		strftime(outBuf, sizeof(outBuf), "%b %Y", &stampTime);
		str += outBuf;

	} else if (currentTime.tm_mon != stampTime.tm_mon){
		// Different month
		strftime(outBuf, sizeof(outBuf), "%b ", &stampTime);
		str += outBuf + to_string(stampTime.tm_mday) + dateStr(stampTime.tm_mday);

	} else if (currentTime.tm_mday != stampTime.tm_mday && (currentTime.tm_mday - stampTime.tm_mday) > 6){
		// Different day, different week
		str += to_string(stampTime.tm_mday) + dateStr(stampTime.tm_mday);
		strftime(outBuf, sizeof(outBuf), " %I:%M%p", &stampTime);
		str += outBuf;

	} else if (currentTime.tm_mday != stampTime.tm_mday){
		// Different day, same week
		strftime(outBuf, sizeof(outBuf), "%a %I:%M%p", &stampTime);
		str += outBuf;

	} else {
		// Same day
		strftime(outBuf, sizeof(outBuf), "%I:%M%p", &stampTime);
		str += outBuf;
	}
	
	return str;

}

string dateStr(int day){
	string ending;

	if (day % 100 >= 11 && day % 100 <= 13){
		ending = "th";
	} else {
		switch (day % 10){
			case 1:
				ending = "st";
				break;
			case 2:
				ending = "nd";
				break;
			case 3:
				ending = "rd";
				break;
			default:
				ending = "th";
		}
	}
	return ending;
}



inline char getBaseChar(chtype ch){
	return (char)(ch & A_CHARTEXT);
}

void pushBackStr(string str, vector<chtype>& outBuf, attr_t attr){
	for (char& ch : str){
		outBuf.push_back((chtype)ch | attr);
	}
}

void Win::freeWin(){
	if (textWin){
		delwin(textWin);
	}

	if (bordWin){
		delwin(textWin);
	}
}

int lineCount(const unique_ptr<formMsg>& formStr, int maxCols){
	int lines = 0;
	int headLen = formStr->header.size();
	int msgLen = formStr->message.size();

	if (headLen >= maxCols){
		lines = headLen / maxCols;
		headLen %= maxCols;
	}

	int lineWidth = maxCols - headLen;
	if (lineWidth <= 0){
		lineWidth = 1;
	}

	lines += (msgLen + lineWidth - 1) / lineWidth;

	return lines;
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


WINDOW* centerWin(WINDOW* parent, string& title, string& caption, int height, int width){ 
	int startY = (LINES - height)/2;
	int startX = (COLS - width)/2;

	if (parent == NULL){
		parent = stdscr;
	}

	WINDOW* localWin = newwin(height, width, startY, startX);

	wrefresh(localWin);


	int leftPad = (width - caption.length())/2;
	int topPad = VALIGN * 2;

	mvwprintw(localWin, topPad, leftPad, caption.c_str());

	box(localWin, 0,0);

	leftPad = (width - title.length())/2;
	mvwprintw(localWin, 0, leftPad, title.c_str());

	return localWin;
}


path getConfDir(){
	const char* home = getenv("HOME");
	// Not a check but if you have this unset, you have bigger problems
	path configDir = path(home) / STORAGE_DIR;
	create_directories(configDir);
	path configFile = configDir / "config";

	return configFile;
}

void redrawUi(){

}
