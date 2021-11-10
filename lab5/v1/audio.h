#if !defined(_AUDIO_H_)
#define _AUDIO_H_

#define AUDIO_REQUEST_INTERVAL_MS 313
#define AUDIO_RATE 8000
#define AUDIO_FRAME_SIZE 4096

#include <alsa/asoundlib.h>

int mulawopen(snd_pcm_t **pcm_handle);
int mulawwrite(snd_pcm_t *pcm_handle, const uint8_t *const data,
               const size_t size);
int mulawclose(snd_pcm_t *pcm_handle);

#endif // _AUDIO_H_