CC = gcc
CFLAGS = -g -Wall
OBJECTS = rcommandserver.o rcommandclient.o ../lib/socket_utils.o parse_addrinfo_arg.o

.PHONY: all
all: rcommandserver.bin rcommandclient.bin

rcommandserver.bin: rcommandserver.o ../lib/socket_utils.o parse_addrinfo_arg.o
	$(CC) -o $@ $^

rcommandclient.bin: rcommandclient.o ../lib/socket_utils.o parse_addrinfo_arg.o
	$(CC) -o $@ $^

$(OBJECTS): %.o: %.c

.PHONY: clean
clean:
	$(RM) *.o *.bin