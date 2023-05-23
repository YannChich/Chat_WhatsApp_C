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
#include "reactor.h"

#define PORT "9034"   // Port we're listening on

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
    // Same implementation as your get_listener_socket function...
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
        // Here you should add functionality to remove the client from the reactor
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

int main(void)
{
    int listener = get_listener_socket();

    if (listener == -1) {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }

    reactor_t* reactor = createReactor();
    addFd(reactor, listener, handle_new_connection);

    startReactor(reactor);

    waitFor(reactor);

    return 0;
}
