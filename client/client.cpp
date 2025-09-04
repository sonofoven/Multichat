#include "client.hpp"
#include "interface.hpp"

list<string> userConns = {};
vector<uint8_t> readBuf;
connInfo clientInfo;

vector<uint8_t> writeBuf;
string inputBuf;
string serverName;

mutex logMtx;
shared_mutex fileMtx;
queue<unique_ptr<Packet>> logQueue;
condition_variable queueCv;
int epollFd;

atomic<bool> redrawQueued = false;


int main() {
	struct sigaction sa {};
	sa.sa_handler = sigwinchHandler;
	sa.sa_flags = SA_RESTART;
	sigaction(SIGWINCH, &sa, nullptr);

	cout << "Multichat v" << VERSION << endl;

	// Check if setting file exists

	// if not -> form + generate one

	// Attempt connection

	// If no connection -> prompt reconnect

	// If connection -> check validation

	// If bad validation -> prompt message & kick out

	if (!fileVerify()){
		cout << "File not found" << endl;
		exit(1);
	}

	interfaceStart();

	int servFd = startUp();

	thread logT(logLoop);
	logT.detach();

	if (servFd < 0){
		endwin();
		cout << "Connection Not Possible" << endl;
		exit(1);
	}


	// Set up epoll for both server and key input
	epollFd = -1;
	epoll_event events[MAX_EVENTS];

	// Create our epoll instance
	epollFd = epoll_create1(0);
	if (epollFd == -1){
		perror("Epoll creation failure");
		exit(1);
	}

	UiContext uiContext = setupWindows();

	restoreHistory(uiContext);

	fcntl(servFd, F_SETFL, O_NONBLOCK);
	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
	epollModify(epollFd, servFd, EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR, EPOLL_CTL_ADD);
	epollModify(epollFd, STDIN_FILENO, EPOLLIN | EPOLLET, EPOLL_CTL_ADD);

	while (1){
        int eventCount = epoll_wait(epollFd, events, MAX_EVENTS, -1);

		if (redrawQueued.exchange(false)){
			endwin();
			clearok(stdscr, TRUE);
			refresh();

			int rows, cols;
			getmaxyx(stdscr, rows, cols);
			resizeterm(rows, cols);
			redrawUi(uiContext, rows, cols);
			refresh();
			continue; // Remove this later
		}

        if (eventCount == -1) {
            if (errno == EINTR) {
				continue;
            } else {
				cerr << "epoll wait" << endl;
                break;
            }
            continue;
        }


		for (int i = 0; i < eventCount; i++){

			int fd = events[i].data.fd;
			uint32_t event = events[i].events;

			if (fd == servFd){
				// If something happened w/ serverfd
				if (event & (EPOLLHUP | EPOLLERR)){
					// Server dying event
					close(epollFd);
					close(servFd);
					endwin();
					exit(1);
				}

				if (event & EPOLLOUT){
					// Write event
					handleWrite(servFd);
				}

				if (event & EPOLLIN){
					// Read event
					handleRead(servFd, uiContext);
				}

			} else if (uiContext.uiDisplayed) {
				// If something happened w/ keyboard input
				int ch;

				curs_set(2); // Make the cursor visible
				while ((ch = wgetch(uiContext.inputWin->textWin)) != ERR){
					handleCh(uiContext, ch, servFd);
				}
			}
		}
	}

	close(epollFd);
	close(servFd);
	endwin();
}


