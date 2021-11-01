CC = gcc
CFLAGS = -g -Wall
OBJECTS = rrunnerc.o rrunners.o ../lib/roadrunner_client.o ../lib/roadrunner_server.o ../lib/socket_utils.o ../lib/arg_checkers.o ../lib/request_codec.o ../lib/packet_codec.o ../lib/bbcodec.o ../lib/access_control.o

.PHONY: all
all: rrunnerc rrunners

rrunnerc: rrunnerc.o ../lib/socket_utils.o ../lib/arg_checkers.o ../lib/request_codec.o ../lib/packet_codec.o ../lib/roadrunner_client.o ../lib/bbcodec.o ../lib/access_control.o
	$(CC) -o $@ $^

rrunners: rrunners.o ../lib/socket_utils.o ../lib/arg_checkers.o ../lib/request_codec.o ../lib/packet_codec.o ../lib/roadrunner_server.o ../lib/bbcodec.o ../lib/access_control.o
	$(CC) -o $@ $^

$(OBJECTS): %.o: %.c

.PHONY: clean
clean:
	$(RM) *.o rrunnerc rrunners