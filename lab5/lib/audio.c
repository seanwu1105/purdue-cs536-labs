#include <alsa/asoundlib.h>
#include <stdio.h>

#include "audio.h"

int mulawopen(snd_pcm_t **pcm_handle)
{
    int status = 0;
    snd_pcm_hw_params_t *params;
    unsigned int rate = AUDIO_RATE;

    if ((status = snd_pcm_open(pcm_handle, "default", SND_PCM_STREAM_PLAYBACK,
                               0)) < 0)
    {
        fprintf(stderr, "Cannot open PCM audio device\n");
        return status;
    }

    snd_pcm_hw_params_alloca(&params);

    if ((status = snd_pcm_hw_params_any(*pcm_handle, params)) < 0)
    {
        fprintf(stderr, "Cannot initialize PCM hardware parameter structure\n");
        return status;
    }

    if ((status = snd_pcm_hw_params_set_access(
             *pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
    {
        fprintf(stderr, "Cannot set PCM access type\n");
        return status;
    }

    if ((status = snd_pcm_hw_params_set_format(*pcm_handle, params,
                                               SND_PCM_FORMAT_MU_LAW)) < 0)
    {
        fprintf(stderr, "Cannot set PCM sample format\n");
        return status;
    }

    if ((status = snd_pcm_hw_params_set_channels(*pcm_handle, params, 1)) < 0)
    {
        fprintf(stderr, "Cannot set PCM channels\n");
        return status;
    }

    if ((status = snd_pcm_hw_params_set_rate_near(*pcm_handle, params, &rate,
                                                  0)) < 0)
    {
        fprintf(stderr, "Cannot set PCM sample rate\n");
        return status;
    }

    if ((status = snd_pcm_hw_params(*pcm_handle, params)) < 0)
    {
        fprintf(stderr, "Cannot set PCM hardware parameters\n");
        return status;
    }

    snd_pcm_uframes_t frames;
    if ((status = snd_pcm_hw_params_get_period_size(params, &frames, 0)) < 0)
    {
        fprintf(stderr, "Cannot get PCM period size\n");
        return status;
    }

    if (frames != AUDIO_FRAME_SIZE)
    {
        fprintf(stderr, "PCM period size is not %d\n", AUDIO_FRAME_SIZE);
        return -1;
    }

    return 0;
}

static int mulawrecover(snd_pcm_t *pcm_handle)
{
    int status = 0;
    if ((status = snd_pcm_prepare(pcm_handle)) < 0)
    {
        fprintf(stderr, "Cannot recover PCM\n");
        return status;
    }
    return 0;
}

int mulawwrite(snd_pcm_t *pcm_handle, const uint8_t *const data,
               const size_t size)
{
    static int write_status = 0;
    if (write_status < 0)
    {
        if (write_status == -EPIPE)
        {
            int status = 0;
            if ((status = mulawrecover(pcm_handle)) < 0)
            {
                fprintf(stderr, "Cannot recover PCM device: %d\n", status);
                return status;
            }
            else
                fprintf(stderr, "Cannot write to PCM device: Queue is Empty\n");
        }
        else
        {
            fprintf(stderr, "Cannot write to PCM device: %d\n", write_status);
            return write_status;
        }
    }

    write_status = snd_pcm_writei(pcm_handle, data, (snd_pcm_uframes_t)size);
    return 0;
}

int mulawclose(snd_pcm_t *pcm_handle)
{
    int status = 0;
    if ((status = snd_pcm_drain(pcm_handle)) < 0)
    {
        fprintf(stderr, "Cannot drain PCM audio device\n");
        return status;
    }

    if ((status = snd_pcm_close(pcm_handle)) < 0)
    {
        fprintf(stderr, "Cannot close PCM audio device\n");
        return status;
    }
    return 0;
}
