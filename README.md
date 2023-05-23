# Chat_WhatsApp_C

## Project Overview
This project is a simplified chat server that uses the reactor pattern for managing input and output. The
server accepts connections from multiple clients and relays messages from one client to all others. It
utilizes multi-threading to handle multiple client connections simultaneously.

## File Descriptions
• reactor.h: This is the header file for the reactor. It contains the definitions of the reactor
structure and the function prototypes associated with it.
• reactor.c: This file contains the implementation of the reactor. It includes the functions that
create and manage the reactor and handle events.
• react_server.c: This is the main server file. It sets up a listener socket to accept client
connections, sets up the reactor, and handles client input.
• makefile: This file contains commands to compile and build the project.

## How to Run the Server
1. Open a terminal window.
2. Navigate to the project directory using cd.
3. Run make to build the server. This will create the react_server executable.
4. Run the server by typing ./react_server.

## How to Test the Server with nc (Netcat)
Once the server is running, you can test it by opening multiple terminal windows and connecting to the
server with nc (Netcat).
Here are the steps:
1. Open a new terminal window.
2. Type nc localhost 9034 and press Enter. This command connects to the server at localhost
on port 9034. Repeat this step in each new terminal window.
3. Once connected, you can type messages into each nc window. The server should relay the
messages to all connected clients.

## How to Stop the Server
You can stop the server by pressing Ctrl+C in the terminal window running the server. The server is set
up to handle the SIGINT signal generated by Ctrl+C and will clean up and exit gracefully.