VPATH = ./src
CC = gcc
CFLAGS= -Wall -pedantic -Wextra -Wconversion -std=gnu11 
BIN = ./bin

all: server

server: server.o server_util.o
	$(CC) $(CFLAGS) $^ -o $(BIN)/$@ -lulfius -lorcania

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ -lulfius -lorcania


clean:
	rm *.o
	rm $(BIN)/server



