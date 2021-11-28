CC = gcc
CFLAGS = -g -Wall
OBJECTS = zigzagrouter.o zigzagconf.o mypingcli.o mypingsrv.o read_config.o message_codec.o socket_utils.o read_overlay.o zzconfig_codec.o
PROGS = zigzagrouter zigzagconf mypingcli mypingsrv

.PHONY: all
all: $(PROGS)

zigzagrouter: zigzagrouter.o socket_utils.o zzconfig_codec.o
	$(CC) -o $@ $^

zigzagconf: zigzagconf.o socket_utils.o zzconfig_codec.o read_overlay.o
	$(CC) -o $@ $^

mypingcli: mypingcli.o read_config.o socket_utils.o message_codec.o
	$(CC) -o $@ $^

mypingsrv: mypingsrv.c read_config.o socket_utils.o message_codec.o
	$(CC) -o $@ $^

$(OBJECTS): %.o: %.c

.PHONY: clean
clean:
	$(RM) *.o $(PROGS)