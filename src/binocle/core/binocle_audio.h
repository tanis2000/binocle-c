//
// Created by Valerio Santinelli on 09/11/2018.
//

#ifndef HELIRUN_BINOCLE_AUDIO_H
#define HELIRUN_BINOCLE_AUDIO_H

#include "binocle_sdl.h"

#define BINOCLE_AUDIO_MAX_SOUNDS 16

typedef struct binocle_sound {
  Mix_Chunk *sound;
} binocle_sound;

typedef struct binocle_music {
  Mix_Music *music;
} binocle_music;

typedef struct binocle_audio {
  binocle_sound *active_sounds[BINOCLE_AUDIO_MAX_SOUNDS];
  uint32_t active_sounds_counter;
  binocle_music active_music;
  bool paused;
} binocle_audio;

binocle_sound binocle_sound_new();
void binocle_sound_destroy(binocle_sound *sound);
void binocle_sound_load(binocle_sound *sound, char *filename);
void binocle_sound_play(binocle_sound *sound);
void binocle_sound_stop(binocle_sound *sound);
void binocle_sound_pause(binocle_sound *sound);
void binocle_sound_resume(binocle_sound *sound);

binocle_music binocle_music_new();
void binocle_music_destroy(binocle_music *music);
void binocle_music_load(binocle_music *music, char *filename);
void binocle_music_play(binocle_music *music, bool loop);
void binocle_music_stop(binocle_music *music);
void binocle_music_pause(binocle_music *music);
void binocle_music_resume(binocle_music *music);

binocle_audio binocle_audio_new();
void binocle_audio_destroy(binocle_audio *audio);
void binocle_audio_play_music(binocle_audio *audio, binocle_music *music, bool loop);
void binocle_audio_play_sound(binocle_audio *audio, binocle_sound *sound);
binocle_music *binocle_audio_load_music(binocle_audio *audio, char *filename);
bool binocle_audio_load_sound(binocle_audio *audio, char *filename, binocle_sound *sound);
void binocle_audio_stop_all(binocle_audio *audio);
void binocle_audio_pause_all(binocle_audio *audio);
void binocle_audio_resume_all(binocle_audio *audio);
bool binocle_audio_is_paused(binocle_audio *audio);
void binocle_audio_set_music_volume(int volume);

#endif //HELIRUN_BINOCLE_AUDIO_H
