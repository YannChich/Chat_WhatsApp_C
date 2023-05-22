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

void handleEvent(void *reactor, int fd) {
    char buf[256];
    int nbytes;

    if (fd == getListener(reactor)) {
        struct sockaddr_storage remoteaddr;
        socklen_t addrlen;
        char remoteIP[INET6_ADDRSTRLEN];
        int newfd = accept(fd, (struct sockaddr *)&remoteaddr, &addrlen);
        if (newfd == -1) {
            perror("accept");
        } else {
            printf("selectserver: new connection from %s on "
                   "socket %d\n",
                   inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr *)&remoteaddr),
                             remoteIP, INET6_ADDRSTRLEN),
                   newfd);
            addFd(reactor, newfd, handleEvent);
        }
    } else {
        if ((nbytes = recv(fd, buf, sizeof buf, 0)) <= 0) {
            if (nbytes == 0) {
                printf("selectserver: socket %d hung up\n", fd);
            } else {
                perror("recv");
            }
            close(fd);
            removeFd(reactor, fd);
        } else {
            for (int j = 0; j < getFdCount(reactor); j++) {
                if (getFd(reactor, j) != getListener(reactor) && getFd(reactor, j) != fd) {
                    if (send(getFd(reactor, j), buf, nbytes, 0) == -1) {
                        perror("send");
                    }
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

    void* reactor = createReactor();

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

    setListener(reactor, listener);
    addFd(reactor, listener, handleEvent);

    startReactor(reactor);

    waitFor(reactor);

    return 0;
}