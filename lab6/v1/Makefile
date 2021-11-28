CC = gcc
CFLAGS = -g -Wall
OBJECTS = zigzagrouter.o mypingcli.o mypingsrv.o read_config.o message_codec.o socket_utils.o
PROGS = zigzagrouter mypingcli mypingsrv

.PHONY: all
all: $(PROGS)

zigzagrouter: zigzagrouter.o socket_utils.o
	$(CC) -o $@ $^

mypingcli: mypingcli.o read_config.o socket_utils.o message_codec.o
	$(CC) -o $@ $^

mypingsrv: mypingsrv.c read_config.o socket_utils.o message_codec.o
	$(CC) -o $@ $^

$(OBJECTS): %.o: %.c

.PHONY: clean
clean:
	$(RM) *.o $(PROGS)