#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "reactor.h"

#define PORT "9034"

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void handleEvent(reactor_t* reactor, int fd) {
    char buf[256];
    int nbytes;

    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;
    char remoteIP[INET6_ADDRSTRLEN];
    
    // Read data from socket
    if ((nbytes = recv(fd, buf, sizeof buf, 0)) <= 0) {
        // Got error or connection closed by client
        if (nbytes == 0) {
            printf("selectserver: socket %d hung up\n", fd);
        } else {
            perror("recv");
        }
        close(fd); // Bye!
    } else {
        // We got some good data from the client
        for (int i = 0; i < reactor->fd_count; i++) {
            if (reactor->fds[i].fd != fd) {
                if (send(reactor->fds[i].fd, buf, nbytes, 0) == -1) {
                    perror("send");
                }
            }
        }
    }
}

int main(void) {
    int listener;
    int yes=1;
    int i, rv;
    struct addrinfo hints, *ai, *p;

    reactor_t* reactor = createReactor();

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

        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai);

    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    addFd(reactor, listener, handleEvent);

    startReactor(reactor);

    waitFor(reactor);

    return 0;
}
