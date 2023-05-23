CC=gcc
CFLAGS=-I.
DEPS = reactor.h
OBJ = server.o reactor.o
LIB_NAME=st_reactor.so
LIB_OBJ=reactor.o

all: $(LIB_NAME) server

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -fPIC

$(LIB_NAME): $(LIB_OBJ)
	$(CC) -shared -o $@ $^

server: server.o $(LIB_NAME)
	$(CC) -o $@ $^ $(CFLAGS) -L. -l:$(LIB_NAME)

clean:
	rm -f *.o server $(LIB_NAME)