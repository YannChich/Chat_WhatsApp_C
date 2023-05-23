CC=gcc
CFLAGS=-I.
DEPS = reactor.h
OBJ = client.o react_server.o reactor.o
LIB_NAME=st_reactor.so
LIB_OBJ=reactor.o

all: $(LIB_NAME) client react_server

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -fPIC

$(LIB_NAME): $(LIB_OBJ)
	$(CC) -shared -o $@ $^

client: client.o $(LIB_NAME)
	$(CC) -o $@ $^ $(CFLAGS) -L. -l:$(LIB_NAME)

react_server: react_server.o $(LIB_NAME)
	$(CC) -o $@ $^ $(CFLAGS) -L. -l:$(LIB_NAME)

clean:
	rm -f *.o client react_server $(LIB_NAME)