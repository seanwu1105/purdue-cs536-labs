CC = gcc
CFLAGS = -g -Wall
OBJECTS = rcommandserver.o rcommandclient.o

.PHONY: all
all: rcommandserver.bin rcommandclient.bin

rcommandserver.bin: rcommandserver.o
	$(CC) -o $@ $^

rcommandclient.bin: rcommandclient.o
	$(CC) -o $@ $^

$(OBJECTS): %.o: %.c

.PHONY: clean
clean:
	$(RM) *.o *.bin