CC=g++
CFLAGS=-O3
EXTRAFLAGS=-lpqxx -lpq -pthread

all: main #client

main: main.cpp socket.hpp socket.cpp Temper.h creat.hpp transact.hpp helper.hpp
	$(CC) $(CFLAGS) -o main main.cpp socket.hpp socket.cpp Temper.h creat.hpp transact.hpp helper.hpp $(EXTRAFLAGS)
#client: client.cpp socket.hpp
#	$(CC) $(CFLAGS) -o client client.cpp socket.hpp $(EXTRAFLAGS)

clean:
	rm -f *~ *.o main

clobber:
	rm -f *~ *.o
