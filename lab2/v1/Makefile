CC = gcc
CFLAGS = -g -Wall
PROGS = mypingcli mypingsrv

.PHONY: all
all: $(PROGS)

%: %.c
	$(CC) $(CFLAGS)  -o $@ $<

.PHONY: clean
clean:
	$(RM) *.o $(PROGS)