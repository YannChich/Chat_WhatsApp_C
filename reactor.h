#ifndef _REACTOR_H_
#define _REACTOR_H_

#include <poll.h>
#include <pthread.h>

#define MAX_FD 1024

struct reactor_t;


typedef void (*handler_t)(struct reactor_t*, int);

typedef struct reactor_t{
    struct pollfd *fds;         // Dynamically allocated array
    handler_t* handlers;        // Dynamically allocated array
    int fd_count;
    int fd_capacity;            // Current capacity of the array
    int running;
    pthread_t thread;
} reactor_t;

reactor_t* createReactor();
void stopReactor(reactor_t* reactor);
void startReactor(reactor_t* reactor);
void addFd (reactor_t* reactor, int fd,handler_t handler);
void removeFd(reactor_t* reactor, int fd);
void waitFor (reactor_t* reactor);

#endif