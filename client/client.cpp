#include "client.hpp"
#include "interface.hpp"

list<string> userConns = {};
vector<uint8_t> readBuf;
connInfo clientInfo;

vector<uint8_t> writeBuf;
string inputBuf;
string serverName;

int epollFd;

int main() {
	cout << "Multichat v" << VERSION << endl;

	// Check if setting file exists

	// if not -> form + generate one

	// Attempt connection

	// If no connection -> prompt reconnect

	// If connection -> check validation

	// If bad validation -> prompt message & kick out
	if (!fileVerify()){
		exit(1);
	}

	interfaceStart();

	int servFd = startUp();

	if (servFd < 0){
		endwin();
		cout << "Connection Not Possible" << endl;
		exit(1);
	}

	//cout << "Connection Validated" << endl;

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

	fcntl(servFd, F_SETFL, O_NONBLOCK);
	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
	epollModify(epollFd, servFd, EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR, EPOLL_CTL_ADD);
	epollModify(epollFd, STDIN_FILENO, EPOLLIN | EPOLLET, EPOLL_CTL_ADD);

	while (1){
        int eventCount = epoll_wait(epollFd, events, MAX_EVENTS, -1);
        if (eventCount == -1) {
            if (errno == EINTR) {
                break;
            } else if (errno != EINTR) {
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

			} else {
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
