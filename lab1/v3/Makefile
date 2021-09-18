CC = gcc
CFLAGS = -g -Wall

.PHONY: all
all: commandserver.bin commandclient.bin

commandserver.bin: simsh1.o ../lib/parse_command.o
	$(CC) -o $@ $^

commandclient.bin: commandclient.o
	$(CC) -o $@ $^

simsh1.o ../lib/parse_command.o commandclient.o: %.o: %.c

.PHONY: clean
clean:
	$(RM) *.o *.bin *.dat cfifo*