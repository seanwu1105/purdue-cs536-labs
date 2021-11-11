#if !defined(_AUDIO_CLIENT_H_)
#define _AUDIO_CLIENT_H_

#include <alsa/asoundlib.h>
#include <netdb.h>
#include <stdint.h>
#include <sys/socket.h>

#include "audio.h"
#include "audio_client_config.h"
#include "congestion_controls.h"
#include "queue.h"

#define REQUIRED_ARGC 10
#define FILE_REQUEST_TIMEOUT_MS 500
#define CONGESTION_CONTROL_PARAMETERS_FILENAME "audiocliparam.dat"
#define FREAD_BUFFER_SIZE 4096
#define BUFFER_UNIT_SIZE AUDIO_FRAME_SIZE

int stream_audio_to_device(snd_pcm_t **pcm_handle, Queue *queue);
int get_config(int argc, char **argv, Config *config,
               const char *const help_msg);
int read_parameters_file(Config *const config);
int start_client(snd_pcm_t **pcm_handle, Queue *const queue, Config *config,
                 const CongestionControlMethod congestion_control_methods[]);
int request_file(const int sockfd, const Config *const config);
int start_streaming_and_cancel_request_timeout(
    const int sockfd, Queue *const queue, const Config *const config,
    const CongestionControlMethod congestion_control_methods[]);
int send_feedback(const int sockfd, const struct sockaddr *const server_addr,
                  const socklen_t server_addr_len, const Config *const config,
                  long double *const packets_per_second,
                  const Queue *const queue,
                  const CongestionControlMethod congestion_control_methods[]);

#endif // _AUDIO_CLIENT_H_