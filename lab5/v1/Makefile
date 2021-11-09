CC = gcc
CFLAGS = -g -Wall
OBJECTS = audiocli.o audiosrv.o socket_utils.o

.PHONY: all
all: audiocli audiosrv

audiocli: audiocli.o socket_utils.o
	$(CC) -o $@ $^

audiosrv: audiosrv.o socket_utils.o
	$(CC) -o $@ $^

$(OBJECTS): %.o: %.c

.PHONY: clean
clean:
	$(RM) *.o audiocli audiosrv