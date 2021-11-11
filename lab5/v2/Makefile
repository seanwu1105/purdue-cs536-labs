CC = gcc
CFLAGS = -g -Wall
OBJECTS = audiocli.o audiosrv.o ../lib/socket_utils.o ../lib/parameter_checkers.o ../lib/request_codec.o ../lib/queue.o ../lib/pspacing.o ../lib/congestion_controls.o ../lib/audio_client.o ../lib/audio_server.o
LDLIBS = -lasound

.PHONY: all
all: audiocli audiosrv

audiocli: audiocli.o ../lib/socket_utils.o ../lib/parameter_checkers.o ../lib/request_codec.o ../lib/queue.o ../lib/pspacing.o ../lib/audio.o ../lib/congestion_controls.o ../lib/audio_client.o
	$(CC) -o $@ $^ $(LDLIBS)

audiosrv: audiosrv.o ../lib/socket_utils.o ../lib/parameter_checkers.o ../lib/request_codec.o ../lib/queue.o ../lib/pspacing.o ../lib/audio_server.o
	$(CC) -o $@ $^

../lib/audio.o: ../lib/audio.c
	$(CC) -c -o $@ $^ $(LDLIBS)

$(OBJECTS): %.o: %.c

.PHONY: clean
clean:
	$(RM) *.o audiocli audiosrv