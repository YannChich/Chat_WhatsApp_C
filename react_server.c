#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <signal.h>
#include "reactor.h"

#define PORT "9034"   // Port we're listening on

reactor_t* global_reactor; 

void signal_handler_ctlrC(int sig) {
    printf("\n");
    printf("---[Exit]--- :The server will shut down, all memory has been freed successfully. \n");
    sleep(0.5);
    stopReactor(global_reactor);
    free(global_reactor->fds);
    free(global_reactor->handlers);
    free(global_reactor);
    exit(0);
    
}
// Get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// Return a listening socket
int get_listener_socket(void)
{
    int listener;     // Listening socket descriptor
    int yes=1;        // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    // Get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) { 
            continue;
        }
        
        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // If we got here, it means we didn't get bound
    if (p == NULL) {
        return -1;
    }

    freeaddrinfo(ai); // All done with this

    // Listen
    if (listen(listener, 10) == -1) {
        return -1;
    }

    return listener;
}

void handle_client(reactor_t* reactor, int client_fd)
{
    char buf[256];
    int nbytes = recv(client_fd, buf, sizeof buf, 0);

    if (nbytes <= 0) {
        if (nbytes == 0) {
            printf("react_server: socket %d hung up\n", client_fd);
        } else {
            perror("recv");
        }
        close(client_fd);
        removeFd(reactor, client_fd);
    } else {
        // Send to everyone else
        for (int i = 0; i < reactor->fd_count; i++) {
            int dest_fd = reactor->fds[i].fd;
            if (dest_fd != client_fd && dest_fd != reactor->fds[0].fd) {
                if (send(dest_fd, buf, nbytes, 0) == -1) {
                    perror("send");
                }
            }
        }
    }
}

void handle_new_connection(reactor_t* reactor, int listener)
{
    struct sockaddr_storage remoteaddr; // Client address
    socklen_t addrlen;

    char remoteIP[INET6_ADDRSTRLEN];

    addrlen = sizeof remoteaddr;
    int newfd = accept(listener,
        (struct sockaddr *)&remoteaddr,
        &addrlen);

    if (newfd == -1) {
        perror("accept");
    } else {
        printf("react_server: new connection from %s on "
            "socket %d\n",
            inet_ntop(remoteaddr.ss_family,
                get_in_addr((struct sockaddr*)&remoteaddr),
                remoteIP, INET6_ADDRSTRLEN),
            newfd);
        addFd(reactor, newfd, handle_client);  // Add the new client to the reactor
    }
}

int main(void)
{
    int listener = get_listener_socket();

    if (listener == -1) {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }

    global_reactor = createReactor();

    signal(SIGINT, signal_handler_ctlrC);

    addFd(global_reactor, listener, handle_new_connection);

    startReactor(global_reactor);

    waitFor(global_reactor);

    return 0;
}