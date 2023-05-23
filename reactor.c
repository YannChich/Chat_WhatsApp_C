#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "reactor.h"

reactor_t* createReactor() {
    reactor_t* reactor = (reactor_t*)malloc(sizeof(reactor_t));
    reactor->fd_capacity = 10;  // Initial capacity
    reactor->fds = (struct pollfd*)malloc(sizeof(struct pollfd) * reactor->fd_capacity);
    reactor->handlers = (handler_t*)malloc(sizeof(handler_t) * reactor->fd_capacity);
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
    // Check if the arrays need to be resized
    if (reactor->fd_count >= reactor->fd_capacity) {
        reactor->fd_capacity *= 2;  // Double the capacity
        reactor->fds = realloc(reactor->fds, sizeof(struct pollfd) * reactor->fd_capacity);
        reactor->handlers = realloc(reactor->handlers, sizeof(handler_t) * reactor->fd_capacity);
    }

    reactor->fds[reactor->fd_count].fd = fd;
    reactor->fds[reactor->fd_count].events = POLLIN;
    reactor->handlers[reactor->fd_count] = handler;
    reactor->fd_count++;
}

void removeFd(reactor_t* reactor, int fd) {
    // Find the fd in the array
    for (int i = 0; i < reactor->fd_count; i++) {
        if (reactor->fds[i].fd == fd) {
            // Move the other fds down
            memmove(&reactor->fds[i], &reactor->fds[i+1], (reactor->fd_count - i - 1) * sizeof(struct pollfd));
            memmove(&reactor->handlers[i], &reactor->handlers[i+1], (reactor->fd_count - i - 1) * sizeof(handler_t));
            reactor->fd_count--;
            break;
        }
    }
}

void waitFor(reactor_t* reactor) {
    pthread_join(reactor->thread, NULL);
}