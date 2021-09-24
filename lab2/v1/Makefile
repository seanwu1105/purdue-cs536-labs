CC = gcc
CFLAGS = -g -Wall
OBJECTS = mypingcli.o mypingsrv.o utils.o
PROGS = mypingcli mypingsrv

.PHONY: all
all: $(PROGS)

mypingcli: mypingcli.o utils.o
	$(CC) -o $@ $^

mypingsrv: mypingsrv.c
	$(CC) -o $@ $^

$(OBJECTS): %.o: %.c

.PHONY: clean
clean:
	$(RM) *.o $(PROGS)