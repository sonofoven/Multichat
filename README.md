# MultiChat v1.0

MultiChat is a C++ terminal-based chat application using the `ncurses` library. It features a client-server architecture that allows multiple users to connect to a central server and exchange messages in real-time without provisioning threads for each client.

## Features

* **Client-Server Model**: A central server handles connections, message broadcasting, and user management.
* **Terminal UI**: A full-featured terminal interface built with `ncurses`, `menu`, and `form` libraries.
* **Real-time Messaging**: Clients can send and receive broadcast messages from all other connected users.
* **Dynamic Windows** Windows will resize based on terminal size and the messaging window is scrollable. 
* **User Status**: See a list of currently connected users, with real-time updates when users join or leave the chat.
* **Connection Management**: The client can attempt to reconnect if the connection fails.
* **Persistent Configuration**: The client saves connection details (IP, Port, Username) to a config file (`~/.multiChat/config`) after the first successful connection.
* **Message History**: The client logs received messages and restores them on startup, providing a persistent chat history.
* **Server-side Logging**: The server maintains logs of chat activity.
* **Client-side Logging**: The client also maintains logs of chat activity per server then syncs with the server.

## Components

1. **Server**

   * An `epoll`-based asynchronous server that manages multiple client connections efficiently.
   * Handles client connections, disconnections, and message broadcasting.
   * Validates usernames to prevent duplicates.
   * Logs all broadcast messages and connection events and updates clients if need be.

2. **Client**

   * An `ncurses`-based UI with separate windows for messages, the user list, and text input.
   * Resizes appropriately with window changes.
   * Scroll back within the message window.
   * A state-driven controller manages the UI (e.g., connection form, main chat, reconnect menu).
   * Handles user input for sending messages and scrolling the chat history.
   * Stores and reloads chat history from log files and syncs with the server to save bits.

## Architecture

The application follows a classic client-server model designed for I/O efficiency using non-blocking, event-driven programming.

* **Server Architecture**: The server's core is a single-threaded event loop powered by Linux's `epoll` API. This allows it to handle thousands of simultaneous connections without the overhead of a thread per client. A second thread is used only for accepting new connections to prevent blocking the main event loop. A third thread with a condition variable handles writing to log files asynchronously. All client state is stored in-memory.

* **Client Architecture**: The client is also built around an `epoll` event loop that monitors standard input, the server socket, and a `signalfd` for window resize events. Polling on the client side is important as `ncurses` is not thread safe and so user input and the socket must be handled within the same thread, hence the polling. It uses a state machine pattern to manage the UI flow, transitioning between states like filling out the connection form, chatting, and reconnecting. The UI is rendered with the `ncurses` library.

* **Communication Protocol**: A custom binary protocol is used for all client-server communication. Each message is a "packet" with a fixed-length header containing the total packet length and an opcode, followed by a variable-length payload. This allows for efficient parsing on both ends.


## How to Build and Run

### Dependencies

* `g++`
* `make`
* `libncurses-dev`
* `libform-dev`
* `libmenu-dev`

On a Debian-based system (like Ubuntu), you can install the dependencies with:

```bash
sudo apt-get update && sudo apt-get install build-essential libncurses-dev libform-dev libmenu-dev
```

### Building the Application

From the root directory of the project, simply run `make`:
```bash
make
```
This will compile two binaries: `server/server` and `client/client`.

### Running the Application

1. **Start the Server**: Open a terminal and run the server executable. You can optionally provide a name for your server (if omitted the default is `Multichat`). It saves log files to a directory in the current directory called `servLogs`.
```bash
server/server "The Dude Zone"
```
2. **Start the Client**: In one or more separate terminals, run the client executable. The client serves configuration and log information in `~/.multiChat`.
```bash
client/client
```

On the first run, you will be prompted to enter the server's IP address (use `127.0.0.1` for local), the port (default is `8080`), and a username.

## Docker Support

MultiChat includes a Docker setup for easy server deployment and consistent behavior.
The Docker Hub page is avaliable [here](https://hub.docker.com/r/novesen/multichat), it also contains instructions how to build the container yourself.

### Example Docker Compose

This example is avaliable in `server/Docker` as well as the original Dockerfile
```bash
services:
  multichat-server:
    image: "novesen/multichat:latest"
    ports:
      - "2215:8080"
    environment:
      MULTICHAT_NAME: "The Dude Zone"
    volumes:
      - ./logs:/app/Multichat/server/servLogs
```

This builds the server image, maps port 8080 to 2215, sets the server name, and persists logs to the host.

### Running the Server

From `server/Docker`, run:
```bash
docker compose up -d
```  
- Server logs appear in the host `logs` folder.


## Deficiencies and Known Issues

*Perfect is the enemy of good*

* **Platform Specificity**: The application is **not POSIX compliant** and will only build and run on **Linux**. It relies on Linux-specific APIs like `epoll` and `signalfd`. It will not work on macOS (which uses `kqueue`) or Windows.

* **No Encryption**: All communication is sent in plaintext. There is no TLS/SSL, making it insecure for use over public networks.

* **No User Authentication**: Usernames are claimed on a first-come, first-served basis without any password or authentication mechanism.

* **No Private Messaging**: The protocol only supports broadcasting messages to all connected users.

* **No Inline Editing**: When typing a message you cannot use to cursor to edit text in the line
before you send meaning mid-line modifications require deletion of half the line/message.

* **No Server Groups**: The protocol only supports broadcasting messages to all connected users.

* **One client per system**: Logs all go to the same space in the computer so if you have two users connected, logged messages would duplicate. It's possible, it's just not pretty or intended.

* **Domain names not accepted**: When prompted for a ip, you will not be able to enter in a domain name as it doesn't dereference anything.


## What I Learned
These are concepts or systems that I did not have knowledge of before this project started
* **C++**
* **Ncurses**
* **Multi Threading**
* **Sockets**
* **Polling**
* **Mutexes and Semaphores**
* **Non-blocking File Descriptors**
