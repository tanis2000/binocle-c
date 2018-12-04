//
// Created by Valerio Santinelli on 09/11/2018.
//

#include "binocle_audio.h"
#include "binocle_log.h"

binocle_sound binocle_sound_new() {
  binocle_sound res = {0};
  res.sound = NULL;
  return res;
}

void binocle_sound_load(binocle_sound *sound, char *filename) {
  sound->sound = Mix_LoadWAV(filename);
  if (sound->sound == NULL) {
    binocle_log_error("Cannot load sound file: %s", filename);
    return;
  }
}

void binocle_sound_play(binocle_sound *sound) {
  if (sound->sound == NULL) {
    return;
  }

  Mix_PlayChannel(-1, sound->sound, 0);
}

void binocle_sound_stop(binocle_sound *sound) {
  Mix_HaltChannel(-1);
}

void binocle_sound_pause(binocle_sound *sound) {
  Mix_Pause(-1);
}

void binocle_sound_resume(binocle_sound *sound) {
  if (Mix_Paused(-1)) {
    Mix_Resume(-1);
  }
}

void binocle_sound_destroy(binocle_sound *sound) {
  Mix_FreeChunk(sound->sound);
  sound->sound = NULL;
}



binocle_music binocle_music_new() {
  binocle_music res = {0};
  res.music = NULL;
  return res;
}

void binocle_music_load(binocle_music *music, char *filename) {
  music->music = Mix_LoadMUS(filename);
  if (music->music == NULL) {
    binocle_log_error("Cannot load music file: %s", filename);
    binocle_log_error("Mix error: %s", Mix_GetError());
    return;
  }
}

void binocle_music_play(binocle_music *music, bool loop) {
  int loops = 0;
  if (loop) {
    loops -= 1;
  }
  Mix_PlayMusic(music->music, loops);
}

void binocle_music_stop(binocle_music *music) {
  Mix_HaltMusic();
}

void binocle_music_pause(binocle_music *music) {
  Mix_PauseMusic();
}

void binocle_music_resume(binocle_music *music) {
  if (Mix_PausedMusic()) {
    Mix_ResumeMusic();
  }
}

void binocle_music_destroy(binocle_music *music) {
  Mix_FreeMusic(music->music);
  music->music = NULL;
}


binocle_audio binocle_audio_new() {
  binocle_audio res = { 0 };
  res.paused = false;
  res.active_music = binocle_music_new();
  return res;
}

void binocle_audio_destroy(binocle_audio *audio) {
  // TODO: free up all the allocated stuff and consider `audio` as the audio system who takes care of its resources
}

void binocle_audio_play_music(binocle_audio *audio, binocle_music *music, bool loop) {
  binocle_music_play(music, loop);
}

void binocle_audio_play_sound(binocle_audio *audio, binocle_sound *sound) {
  binocle_sound_play(sound);
}

binocle_music *binocle_audio_load_music(binocle_audio *audio, char *filename) {
  binocle_music_load(&audio->active_music, filename);
  return &audio->active_music;
}

bool binocle_audio_load_sound(binocle_audio *audio, char *filename, binocle_sound *sound) {
  if (audio->active_sounds_counter >= BINOCLE_AUDIO_MAX_SOUNDS) {
    binocle_log_error("Too many sounds already loaded. The max number is %d", BINOCLE_AUDIO_MAX_SOUNDS);
    return false;
  }
  binocle_sound_load(sound, filename);
  audio->active_sounds[audio->active_sounds_counter] = sound;
  audio->active_sounds_counter++;
  return true;
}

void binocle_audio_stop_all(binocle_audio *audio) {
  binocle_music_stop(&audio->active_music);
  for (int i = 0 ; i < audio->active_sounds_counter ; i++) {
    binocle_sound_stop(audio->active_sounds[i]);
  }
}

void binocle_audio_pause_all(binocle_audio *audio) {
  binocle_music_pause(&audio->active_music);
  for (int i = 0 ; i < audio->active_sounds_counter ; i++) {
    binocle_sound_pause(audio->active_sounds[i]);
  }
  audio->paused = true;
}

void binocle_audio_resume_all(binocle_audio *audio) {
  binocle_music_resume(&audio->active_music);
  for (int i = 0 ; i < audio->active_sounds_counter ; i++) {
    binocle_sound_resume(audio->active_sounds[i]);
  }
  audio->paused = false;
}

bool binocle_audio_is_paused(binocle_audio *audio) {
  return audio->paused;
}

void binocle_audio_set_music_volume(int volume) {
  Mix_VolumeMusic(volume);
}