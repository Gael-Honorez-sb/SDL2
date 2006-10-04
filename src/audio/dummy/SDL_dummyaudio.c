/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2006 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org

    This file written by Ryan C. Gordon (icculus@icculus.org)
*/
#include "SDL_config.h"

/* Output audio to nowhere... */

#include "SDL_rwops.h"
#include "SDL_timer.h"
#include "SDL_audio.h"
#include "../SDL_audiomem.h"
#include "../SDL_audio_c.h"
#include "../SDL_audiodev_c.h"
#include "SDL_dummyaudio.h"

/* The tag name used by DUMMY audio */
#define DUMMYAUD_DRIVER_NAME         "dummy"

/* Audio driver functions */
static int DUMMYAUD_OpenDevice(_THIS, const char *devname, int iscapture);
static void DUMMYAUD_WaitDevice(_THIS);
static void DUMMYAUD_PlayDevice(_THIS);
static Uint8 *DUMMYAUD_GetDeviceBuf(_THIS);
static void DUMMYAUD_CloseDevice(_THIS);

/* Audio driver bootstrap functions */
static int
DUMMYAUD_Available(void)
{
    /* !!! FIXME: check this at a higher level... */
    /* only ever use this driver if explicitly requested. */
    const char *envr = SDL_getenv("SDL_AUDIODRIVER");
    if (envr && (SDL_strcmp(envr, DUMMYAUD_DRIVER_NAME) == 0)) {
        return (1);
    }
    return (0);
}

static int
DUMMYAUD_Init(SDL_AudioDriverImpl *impl)
{
    /* Set the function pointers */
    impl->OpenDevice = DUMMYAUD_OpenDevice;
    impl->WaitDevice = DUMMYAUD_WaitDevice;
    impl->PlayDevice = DUMMYAUD_PlayDevice;
    impl->GetDeviceBuf = DUMMYAUD_GetDeviceBuf;
    impl->CloseDevice = DUMMYAUD_CloseDevice;
    impl->OnlyHasDefaultOutputDevice = 1;

    return 1;
}

AudioBootStrap DUMMYAUD_bootstrap = {
    DUMMYAUD_DRIVER_NAME, "SDL dummy audio driver",
    DUMMYAUD_Available, DUMMYAUD_Init
};

/* This function waits until it is possible to write a full sound buffer */
static void
DUMMYAUD_WaitDevice(_THIS)
{
    /* Don't block on first calls to simulate initial fragment filling. */
    if (this->hidden->initial_calls)
        this->hidden->initial_calls--;
    else
        SDL_Delay(this->hidden->write_delay);
}

static void
DUMMYAUD_PlayDevice(_THIS)
{
    /* no-op...this is a null driver. */
}

static Uint8 *
DUMMYAUD_GetDeviceBuf(_THIS)
{
    return (this->hidden->mixbuf);
}

static void
DUMMYAUD_CloseDevice(_THIS)
{
    if (this->hidden->mixbuf != NULL) {
        SDL_FreeAudioMem(this->hidden->mixbuf);
        this->hidden->mixbuf = NULL;
    }
    SDL_free(this->hidden);
    this->hidden = NULL;
}

static int
DUMMYAUD_OpenDevice(_THIS, const char *devname, int iscapture)
{
    float bytes_per_sec = 0.0f;

    /* Initialize all variables that we clean on shutdown */
    this->hidden = (struct SDL_PrivateAudioData *)
                    SDL_malloc((sizeof *this->hidden));
    if (this->hidden == NULL) {
        SDL_OutOfMemory();
        return 0;
    }
    SDL_memset(this->hidden, 0, (sizeof *this->hidden));

    /* Allocate mixing buffer */
    this->hidden->mixlen = this->spec.size;
    this->hidden->mixbuf = (Uint8 *) SDL_AllocAudioMem(this->hidden->mixlen);
    if (this->hidden->mixbuf == NULL) {
        DUMMYAUD_CloseDevice(this);
        return 0;
    }
    SDL_memset(this->hidden->mixbuf, this->spec.silence, this->spec.size);

    bytes_per_sec = (float) (SDL_AUDIO_BITSIZE(this->spec.format) / 8) *
                             this->spec.channels * this->spec.freq;

    /*
     * We try to make this request more audio at the correct rate for
     *  a given audio spec, so timing stays fairly faithful.
     * Also, we have it not block at all for the first two calls, so
     *  it seems like we're filling two audio fragments right out of the
     *  gate, like other SDL drivers tend to do.
     */
    this->hidden->initial_calls = 2;
    this->hidden->write_delay =
        (Uint32) ((((float) this->spec.size) / bytes_per_sec) * 1000.0f);

    /* We're ready to rock and roll. :-) */
    return 1;
}

/* vi: set ts=4 sw=4 expandtab: */
