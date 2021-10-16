CC = gcc
CFLAGS = -g -Wall
OBJECTS = myftpc.o myftps.o socket_utils.o arg_checkers.o

.PHONY: all
all: myftpc myftps

myftpc: myftpc.o socket_utils.o arg_checkers.o
	$(CC) -o $@ $^

myftps: myftps.o socket_utils.o arg_checkers.o
	$(CC) -o $@ $^

$(OBJECTS): %.o: %.c

.PHONY: clean
clean:
	$(RM) *.o myftpc myftps