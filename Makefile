# Makefile
CC = gcc
CFLAGS = -Wall -g
EXTRA = tcpclients.c topics.c payload.c

all: server subscriber $(EXTRA)

# Compileaza server.c
server: server.c $(EXTRA)
	$(CC) $(CFLAGS) server.c -o server

# Compileaza client.c
subscriber: subscriber.c
	$(CC) $(CFLAGS) subscriber.c -o subscriber

.PHONY: clean

clean:
	rm -f server subscriber
