CC = gcc
CFLAGS = -g -Wall
exe = simsh1.bin
objects = simsh1.o ../lib/parse_command.o

.PHONY: all
all: $(exe)

$(exe): $(objects)
	$(CC) -o $@ $^

$(objects): %.o: %.c

.PHONY: clean
clean:
	$(RM) *.o *.bin