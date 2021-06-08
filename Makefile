VPATH = ./src
CC = gcc
CFLAGS= -Wall -pedantic -Wextra -Wconversion -std=gnu11 
BIN = /usr/bin

all: server_users server_files

server_%: server_%.o server_util.o
	$(CC) $(CFLAGS) $^ -o $(BIN)/$@ -lulfius -lorcania -ljansson

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ -lulfius -lorcania -ljansson


clean:
	rm *.o $(BIN)/server*



