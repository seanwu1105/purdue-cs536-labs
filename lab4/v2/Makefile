CC = gcc
CFLAGS = -g -Wall
OBJECTS = rrunnerc.o rrunners.o socket_utils.o ../lib/arg_checkers.o request_codec.o ../lib/packet_codec.o

.PHONY: all
all: rrunnerc rrunners

rrunnerc: rrunnerc.o ../lib/socket_utils.o ../lib/arg_checkers.o request_codec.o ../lib/packet_codec.o
	$(CC) -o $@ $^

rrunners: rrunners.o ../lib/socket_utils.o ../lib/arg_checkers.o request_codec.o ../lib/packet_codec.o
	$(CC) -o $@ $^

$(OBJECTS): %.o: %.c

.PHONY: clean
clean:
	$(RM) *.o rrunnerc rrunners