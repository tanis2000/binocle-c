//
// Created by Valerio Santinelli on 27/03/22.
//

#ifndef BINOCLE_AUDIO_WRAP_H
#define BINOCLE_AUDIO_WRAP_H

#include "binocle_lua.h"
#include "binocle_audio.h"

#define BINOCLE_AUDIO_MAX_MUSICS 32
#define BINOCLE_AUDIO_MAX_SOUNDS 128

typedef struct l_binocle_audio_t {
  struct binocle_audio *audio;
  binocle_audio_music musics[BINOCLE_AUDIO_MAX_MUSICS];
  int num_musics;
  binocle_audio_sound sounds[BINOCLE_AUDIO_MAX_SOUNDS];
  int num_sounds;
} l_binocle_audio_t;

typedef struct l_binocle_audio_music_t {
  int music_idx;
} l_binocle_audio_music_t;

typedef struct l_binocle_audio_sound_t {
  binocle_audio_sound *sound;
} l_binocle_audio_sound_t;

int luaopen_audio(lua_State *L);

#endif //BINOCLE_AUDIO_WRAP_H
