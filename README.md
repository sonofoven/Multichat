# MultiChat v1.0

MultiChat is a C++ terminal-based chat application using the `ncurses` library. It features a client-server architecture that allows multiple users to connect to a central server and exchange messages in real-time without provisioning threads for each client.

## Features

* **Client-Server Model**: A central server handles connections, message broadcasting, and user management.
* **Terminal UI**: A full-featured terminal interface built with `ncurses`, `menu`, and `form` libraries.
* **Real-time Messaging**: Clients can send and receive broadcast messages from all other connected users.
* **User Status**: See a list of currently connected users, with real-time updates when users join or leave the chat.
* **Connection Management**: The client can attempt to reconnect if the connection fails.
* **Persistent Configuration**: The client saves connection details (IP, Port, Username) to a config file (`~/.multiChat/config`) after the first successful connection.
* **Message History**: The client logs received messages and restores them on startup, providing a persistent chat history.
* **Server-side Logging**: The server also maintains logs of chat activity.

## Components

1.  **Server (`serv`)**
	* An `epoll`-based asynchronous server that manages multiple client connections efficiently.
	* Handles client connections, disconnections, and message broadcasting.
	* Validates usernames to prevent duplicates.
	* Logs all broadcast messages and connection events and updates clients if need be.

2.  **Client (`cli`)**
	* An `ncurses`-based UI with separate windows for messages, the user list, and text input.
	* A state-driven controller manages the UI (e.g., connection form, main chat, reconnect menu).
	* Handles user input for sending messages and scrolling the chat history.
	* Stores and reloads chat history from log files and syncs with the server to save bits.

## Architecture

## How to Build

### Dependencies

* `g++` (C++ compiler)
* `make`
* `libncurses`
* `libform`
* `libmenu`

On a Debian-based system (like Ubuntu), you can install the dependencies with:
```bash
sudo apt-get update
sudo apt-get install build-essential libncurses-dev libform-dev libmenu-dev
