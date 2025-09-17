#include "client.hpp"
#include "interface.hpp"

int prevState = SIZE_ERR; //G
int state = SIZE_ERR; //G
atomic<bool> redrawQueued = false;


// Contexts
ChatContext chatContext;
MenuContext menuContext;
FormContext formContext;

int main() {
	struct sigaction sa {}; // U
	sa.sa_handler = sigwinchHandler;
	sa.sa_flags = SA_RESTART;
	sigaction(SIGWINCH, &sa, nullptr);

	cout << "Multichat v" << VERSION << endl; // U

	interfaceStart(); // U

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
