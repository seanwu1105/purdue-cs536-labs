CC = gcc
CFLAGS = -g -Wall
OBJECTS = audiocli.o audiosrv.o socket_utils.o parameter_checkers.o request_codec.o queue.o pspacing.o
LDLIBS = -lasound

.PHONY: all
all: audiocli audiosrv

audiocli: audiocli.o socket_utils.o parameter_checkers.o request_codec.o queue.o pspacing.o audio.o
	$(CC) -o $@ $^ $(LDLIBS)

audiosrv: audiosrv.o socket_utils.o parameter_checkers.o request_codec.o queue.o pspacing.o
	$(CC) -o $@ $^

audio.o: audio.c
	$(CC) -c -o $@ $^ $(LDLIBS)

$(OBJECTS): %.o: %.c

.PHONY: clean
clean:
	$(RM) *.o audiocli audiosrv