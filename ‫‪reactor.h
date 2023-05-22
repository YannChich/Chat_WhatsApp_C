#ifndef _REACTOR_H_
#define _REACTOR_H_

#define MAX_FD 1024

typedef void (handler_t)(int);


typedef struct reactor_t{
    struct pollfd fds[MAX_FD];
    handler_t handlers[MAX_FD];
    int fd_count;
    int running;
    pthread_t thread;
} reactor_t;

void *createReactor();
void *stopReactor(void *this);
void *startReactor(void *this);
void *addFd (void *this, int fd,handler_t handler);
void *WaitFor (void *this);

#endif // _REACTOR_H_
