CC = gcc
CFLAGS = -g -Wall
OBJECTS = rrunnerc.o rrunners.o socket_utils.o

.PHONY: all
all: rrunnerc rrunners

rrunnerc: rrunnerc.o socket_utils.o
	$(CC) -o $@ $^

rrunners: rrunners.o socket_utils.o
	$(CC) -o $@ $^

$(OBJECTS): %.o: %.c

.PHONY: clean
clean:
	$(RM) *.o rrunnerc rrunners