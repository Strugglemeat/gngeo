/*  gngeo a neogeo emulator
 *  Copyright (C) 2001 Peponas Mathieu
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "sound.h"
#include "emu.h"
#include "memory.h"
#include "gnutil.h"
#include "ym2610.h"

SDL_AudioSpec *desired, *obtain;

#define MIXER_MAX_CHANNELS  16
#define BUFFER_LEN          16384
#define NB_SAMPLES          2048
Uint16 play_buffer[BUFFER_LEN];


#ifndef USE_OSS

void update_sdl_stream(void *userdata, Uint8 *stream, int len)
{
  YM2610Update_stream(len / 4);
  memcpy(stream, (Uint8 *) play_buffer, len);
}
void dummy_stream(void *userdata, Uint8 *stream, int len)
{
}
static int sound_initialize = GN_FALSE;

int init_sdl_audio(void)
{
  if(sound_initialize == GN_TRUE) {
    close_sdl_audio();
  }
  sound_initialize = GN_TRUE;
  SDL_InitSubSystem(SDL_INIT_AUDIO);

  desired = (SDL_AudioSpec *) malloc(sizeof(SDL_AudioSpec));
  obtain = (SDL_AudioSpec *) malloc(sizeof(SDL_AudioSpec));
  desired->freq = SAMPLE_RATE;
  desired->samples = NB_SAMPLES;

  desired->format = AUDIO_S16;
  desired->channels = 2;
  desired->callback = update_sdl_stream;
  desired->userdata = NULL;
  SDL_OpenAudio(desired, obtain);
  return GN_TRUE;
}

void close_sdl_audio(void)
{
  sound_initialize = GN_FALSE;
  SDL_PauseAudio(1);
  SDL_CloseAudio();
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
  if(desired) {
    free(desired);
  }
  desired = NULL;
  if(obtain) {
    free(obtain);
  }
  obtain = NULL;
}

void pause_audio(int on)
{
  printf("PAUSE audio %d\n", on);
  SDL_PauseAudio(on);
}

#else
int dev_dsp = 0;
pthread_t audio_thread;
volatile int paused = 0;
int buflen = 0;


void *fill_audio_data(void *ptr)
{
  printf("Update audio\n");

  while(1) {

    if(!paused) {
      YM2610Update_stream(buflen / 4);
      write(dev_dsp, play_buffer, buflen);
    }

  }
}
void pause_audio(int on)
{
  static init = 0;
  if(init == 0) {
    pthread_create(&audio_thread, NULL, fill_audio_data, NULL);
    init = 1;
  }
  paused = on;
}


int init_sdl_audio(void)
{
  int format;
  int channels = 2;
  int speed = SAMPLE_RATE;
  int arg = 0x9;

  dev_dsp = open("/dev/dsp", O_WRONLY);
  if(dev_dsp == 0) {
    printf("Couldn't open /dev/dsp\n");
    return GN_FALSE;
  }

  if(ioctl(dev_dsp, SNDCTL_DSP_SETFRAGMENT, &arg)) {
    printf(" SNDCTL_DSP_SETFRAGMENT Error\n");
    return GN_FALSE;
  }

  format = AFMT_S16_LE;
  if(ioctl(dev_dsp, SNDCTL_DSP_SETFMT, &format) == -1) {
    perror("SNDCTL_DSP_SETFMT");
    return GN_FALSE;
  }
  if(ioctl(dev_dsp, SNDCTL_DSP_CHANNELS, &channels) == -1) {
    perror("SNDCTL_DSP_CHANNELS");
    return GN_FALSE;
  }
  if(ioctl(dev_dsp, SNDCTL_DSP_SPEED, &speed) == -1) {
    perror("SNDCTL_DSP_SPEED");
    return GN_FALSE;
  }



  if(ioctl(dev_dsp, SNDCTL_DSP_GETBLKSIZE, &buflen) == -1) {
    return GN_FALSE;
  }
  printf("Buf Len=%d\n", buflen);
  buflen *= 2;

  return GN_TRUE;
}

void close_sdl_audio(void)
{
  close(dev_dsp);
}

#endif
