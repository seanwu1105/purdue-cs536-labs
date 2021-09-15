CC = gcc
CFLAGS = -g -Wall
exe = simsh1.bin
objects = simsh1.o parse_command.o

.PHONY: all
all: $(exe)

$(exe): $(objects)
	$(CC) -o $(exe) $(objects)

$(objects): %.o: %.c

.PHONY: clean
clean:
	$(RM) *.o *.bin