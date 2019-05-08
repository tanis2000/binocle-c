//
// Created by Valerio Santinelli on 09/11/2018.
//

#ifndef BINOCLE_AUDIO_H
#define BINOCLE_AUDIO_H

#include "binocle_sdl.h"
#include "miniaudio.h"

#define BINOCLE_AUDIO_MAX_SOUNDS 16
#define BINOCLE_AUDIO_SAMPLE_FORMAT   ma_format_f32
#define BINOCLE_AUDIO_CHANNEL_COUNT   2
#define BINOCLE_AUDIO_SAMPLE_RATE     48000

typedef enum binocle_audio_buffer_usage {
  BINOCLE_AUDIO_BUFFER_USAGE_STATIC = 0,
  BINOCLE_AUDIO_BUFFER_USAGE_STREAM
} binocle_audio_buffer_usage;

typedef struct binocle_sound {
  Mix_Chunk *sound;
} binocle_sound;

typedef struct binocle_music {
  Mix_Music *music;
} binocle_music;

typedef struct binocle_audio_buffer {
  ma_pcm_converter dsp;
  float volume;
  float pitch;
  bool playing;
  bool paused;
  bool looping;
  binocle_audio_buffer_usage usage;
  bool is_sub_buffer_processed[2];
  unsigned int frame_cursor_pos;
  unsigned int buffer_size_in_frames;
  struct binocle_audio_buffer *next;
  struct binocle_audio_buffer *prev;
  unsigned char buffer[1];
} binocle_audio_buffer;

typedef struct binocle_audio {
  binocle_sound *active_sounds[BINOCLE_AUDIO_MAX_SOUNDS];
  uint32_t active_sounds_counter;
  binocle_music active_music;
  bool paused;

  // miniaudio stuff
  ma_decoder decoder;
  ma_device_config device_config;
  ma_device device;
  ma_context context;
  float master_volume;
  binocle_audio_buffer *first_audio_buffer;
  binocle_audio_buffer *last_audio_buffer;
  bool is_audio_initialized;
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
bool binocle_audio_init(binocle_audio *audio);
void binocle_audio_destroy(binocle_audio *audio);
void binocle_audio_init_audio_system();
void binocle_audio_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
static void binocle_audio_mix_audio_frames(ma_device* pDevice, float *framesOut, const float *framesIn, ma_uint32 frameCount, float localVolume);
bool binocle_audio_is_audio_device_ready(binocle_audio *audio);
void binocle_audio_set_master_volume(binocle_audio *audio, float volume);
binocle_audio_buffer *binocle_audio_create_audio_buffer(ma_format format, ma_uint32 channels, ma_uint32 sampleRate, ma_uint32 bufferSizeInFrames, binocle_audio_buffer_usage usage);
void binocle_audio_delete_audio_buffer(binocle_audio_buffer *audioBuffer);
bool binocle_audio_is_audio_buffer_playing(binocle_audio_buffer *audio_buffer);
void binocle_audio_play_audio_buffer(binocle_audio_buffer *audioBuffer);
void binocle_audio_stop_audio_buffer(binocle_audio_buffer *audio_buffer);
void binocle_audio_pause_audio_buffer(binocle_audio_buffer *audioBuffer);
void binocle_audio_resume_audio_buffer(binocle_audio_buffer *audioBuffer);
void binocle_audio_set_audio_buffer_volume(binocle_audio_buffer *audioBuffer, float volume);
void binocle_audio_set_audio_buffer_pitch(binocle_audio_buffer *audioBuffer, float pitch);
void binocle_audio_track_audio_buffer(binocle_audio_buffer *audioBuffer);
void binocle_audio_untrack_audio_buffer(binocle_audio_buffer *audioBuffer);

void binocle_audio_play_music(binocle_audio *audio, binocle_music *music, bool loop);
void binocle_audio_play_sound(binocle_audio *audio, binocle_sound *sound);
binocle_music *binocle_audio_load_music(binocle_audio *audio, char *filename);
bool binocle_audio_load_sound(binocle_audio *audio, char *filename, binocle_sound *sound);
void binocle_audio_stop_all(binocle_audio *audio);
void binocle_audio_pause_all(binocle_audio *audio);
void binocle_audio_resume_all(binocle_audio *audio);
bool binocle_audio_is_paused(binocle_audio *audio);
void binocle_audio_set_music_volume(int volume);

#endif //BINOCLE_AUDIO_H
