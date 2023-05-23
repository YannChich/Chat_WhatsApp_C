#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "reactor.h"

reactor_t* createReactor() {
    reactor_t* reactor = malloc(sizeof(reactor_t));
    reactor->handlers = malloc(sizeof(handler_t) * MAX_FD); // Allocate memory for handlers
    reactor->fd_count = 0;
    reactor->running = 0;
    return reactor;
}

void stopReactor(reactor_t* reactor) {
    reactor->running = 0;
}

void* runReactor(void* arg) {
    reactor_t* reactor = (reactor_t*) arg;
    reactor->running = 1;

    while (reactor->running) {
        int ready_fds = poll(reactor->fds, reactor->fd_count, -1);
        if (ready_fds <= 0) {
            continue;
        }

        for (int i = 0; i < reactor->fd_count; i++) {
            if (reactor->fds[i].revents & POLLIN) {
                reactor->handlers[i](reactor, reactor->fds[i].fd);
            }
        }
    }

    return NULL;
}

void startReactor(reactor_t* reactor) {
    pthread_create(&(reactor->thread), NULL, runReactor, reactor);
}

void addFd(reactor_t* reactor, int fd, handler_t handler) {
    reactor->fds[reactor->fd_count].fd = fd;
    reactor->fds[reactor->fd_count].events = POLLIN;
    reactor->handlers[reactor->fd_count] = handler;
    reactor->fd_count++;
}

void waitFor(reactor_t* reactor) {
    pthread_join(reactor->thread, NULL);
}
