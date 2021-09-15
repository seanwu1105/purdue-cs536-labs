CC = gcc
CFLAGS = -g -Wall

.PHONY: all
all: commandserver.bin commandclient.bin

commandserver.bin: simsh1.o parse_command.o
	$(CC) -o commandserver.bin simsh1.o parse_command.o

commandclient.bin: commandclient.o
	$(CC) -o commandclient.bin commandclient.o

simsh1.o parse_command.o commandclient.o: %.o: %.c

.PHONY: clean
clean:
	$(RM) *.o *.bin *.dat