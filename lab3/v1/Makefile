CC = gcc
CFLAGS = -g -Wall
OBJECTS = myftpc.o myftps.o socket_utils.o arg_checkers.o request_codec.o

.PHONY: all
all: myftpc myftps

myftpc: myftpc.o socket_utils.o arg_checkers.o request_codec.o
	$(CC) -o $@ $^

myftps: myftps.o socket_utils.o arg_checkers.o request_codec.o
	$(CC) -o $@ $^

$(OBJECTS): %.o: %.c

.PHONY: clean
clean:
	$(RM) *.o myftpc myftps