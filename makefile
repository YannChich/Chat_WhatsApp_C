CC = gcc
CFLAGS = -I. -fPIC
LDFLAGS = -L. -lst_reactor -Wl,-rpath,.

all : react_server

reactor.o: reactor.c 
	$(CC) $(CFLAGS) -c reactor.c -o reactor.o

libst_reactor.so: reactor.o
	$(CC) -shared -o libst_reactor.so reactor.o

react_server.o: react_server.c
	$(CC) $(CFLAGS) -c react_server.c -o react_server.o

react_server: react_server.o libst_reactor.so
	$(CC) -o react_server react_server.o $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *.o *.so react_server
