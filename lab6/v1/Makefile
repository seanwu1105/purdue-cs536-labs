CC = gcc
CFLAGS = -g -Wall
OBJECTS = zigzagrouter.o zigzagconf.o socket_utils.o read_overlay.o zzconfig_codec.o print_payload.o
PROGS = zigzagrouter zigzagconf

.PHONY: all
all: $(PROGS)

zigzagrouter: zigzagrouter.o socket_utils.o zzconfig_codec.o print_payload.o
	$(CC) -o $@ $^

zigzagconf: zigzagconf.o socket_utils.o zzconfig_codec.o read_overlay.o print_payload.o
	$(CC) -o $@ $^

$(OBJECTS): %.o: %.c

.PHONY: clean
clean:
	$(RM) *.o $(PROGS)