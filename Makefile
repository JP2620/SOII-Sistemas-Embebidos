VPATH = ./src
CC = gcc
CFLAGS= -Wall -pedantic -Wextra -Wconversion -std=gnu11 
BIN = ./bin

all: server

server: server.o
	$(CC) $(CFLAGS) $< -o $(BIN)/$@ -lulfius

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ -lulfius


clean:
	rm *.o
	rm $(BIN)/server



