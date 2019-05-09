//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_AUDIO_H
#define BINOCLE_AUDIO_H

//#include "binocle_sdl.h"
#include "miniaudio/stb_vorbis.h"
#include "miniaudio/dr_flac.h"
#include "miniaudio/dr_mp3.h"
#include "miniaudio/dr_wav.h"
#include "miniaudio/jar_xm.h"
#include "miniaudio/jar_mod.h"
#include "miniaudio/miniaudio.h"

#define BINOCLE_AUDIO_MAX_SOUNDS      16
#define BINOCLE_AUDIO_SAMPLE_FORMAT   ma_format_f32
#define BINOCLE_AUDIO_CHANNEL_COUNT   2
#define BINOCLE_AUDIO_SAMPLE_RATE     48000
#define BINOCLE_AUDIO_BUFFER_SIZE     4096

typedef enum binocle_audio_buffer_usage {
  BINOCLE_AUDIO_BUFFER_USAGE_STATIC = 0,
  BINOCLE_AUDIO_BUFFER_USAGE_STREAM
} binocle_audio_buffer_usage;

typedef enum binocle_audio_music_context_type {
  BINOCLE_AUDIO_MUSIC_AUDIO_OGG = 0,
  BINOCLE_AUDIO_MUSIC_AUDIO_FLAC,
  BINOCLE_AUDIO_MUSIC_AUDIO_MP3,
  BINOCLE_AUDIO_MUSIC_AUDIO_WAV,
  BINOCLE_AUDIO_MUSIC_MODULE_XM,
  BINOCLE_AUDIO_MUSIC_MODULE_MOD
} binocle_audio_music_context_type;

typedef struct binocle_audio_stream {
  unsigned int sample_rate;    // Frequency (samples per second)
  unsigned int sample_size;    // Bit depth (bits per sample): 8, 16, 32 (24 not supported)
  unsigned int channels;      // Number of channels (1-mono, 2-stereo)

  void *audio_buffer;          // Pointer to internal data used by the audio system.

  int format;                 // Audio format specifier
  unsigned int source;        // Audio source id
  unsigned int buffers[2];    // Audio buffers (double buffering)
} binocle_audio_stream;

typedef struct binocle_audio_music {
  binocle_audio_music_context_type ctx_type;
  stb_vorbis *ctx_ogg;
  drflac *ctx_flac;
  drmp3 ctx_mp3;
  drwav ctx_wav;
  jar_xm_context_t *ctx_xm;
  jar_mod_context_t ctx_mod;

  binocle_audio_stream stream;                 // Audio stream (double buffering)

  int loop_count;                      // Loops count (times music repeats), -1 means infinite loop
  unsigned int total_samples;          // Total number of samples
  unsigned int samples_left;           // Number of samples left to end
} binocle_audio_music;

typedef struct binocle_audio_wave {
  unsigned int sample_count;   // Number of samples
  unsigned int sample_rate;    // Frequency (samples per second)
  unsigned int sample_size;    // Bit depth (bits per sample): 8, 16, 32 (24 not supported)
  unsigned int channels;      // Number of channels (1-mono, 2-stereo)
  void *data;                 // Buffer data pointer
} binocle_audio_wave;

typedef struct binocle_audio_sound {
  void *audio_buffer;      // Pointer to internal data used by the audio system

  unsigned int source;    // Audio source id
  unsigned int buffer;    // Audio buffer id
  int format;             // Audio format specifier
} binocle_audio_sound;

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
  //binocle_sound *active_sounds[BINOCLE_AUDIO_MAX_SOUNDS];
  uint32_t active_sounds_counter;
  //binocle_music active_music;
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

binocle_audio binocle_audio_new();
bool binocle_audio_init(binocle_audio *audio);
void binocle_audio_destroy(binocle_audio *audio);
void binocle_audio_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
static void binocle_audio_mix_audio_frames(void* pUserData, float *framesOut, const float *framesIn, ma_uint32 frameCount, float localVolume);
bool binocle_audio_is_audio_device_ready(binocle_audio *audio);
void binocle_audio_set_master_volume(binocle_audio *audio, float volume);
binocle_audio_buffer *binocle_audio_create_audio_buffer(binocle_audio *audio, ma_format format, ma_uint32 channels, ma_uint32 sampleRate, ma_uint32 bufferSizeInFrames, binocle_audio_buffer_usage usage);
void binocle_audio_delete_audio_buffer(binocle_audio *audio, binocle_audio_buffer *audioBuffer);
bool binocle_audio_is_audio_buffer_playing(binocle_audio_buffer *audio_buffer);
void binocle_audio_play_audio_buffer(binocle_audio_buffer *audioBuffer);
void binocle_audio_stop_audio_buffer(binocle_audio_buffer *audio_buffer);
void binocle_audio_pause_audio_buffer(binocle_audio_buffer *audioBuffer);
void binocle_audio_resume_audio_buffer(binocle_audio_buffer *audioBuffer);
void binocle_audio_set_audio_buffer_volume(binocle_audio_buffer *audioBuffer, float volume);
void binocle_audio_set_audio_buffer_pitch(binocle_audio_buffer *audioBuffer, float pitch);
void binocle_audio_track_audio_buffer(binocle_audio *audio, binocle_audio_buffer *audioBuffer);
void binocle_audio_untrack_audio_buffer(binocle_audio *audio, binocle_audio_buffer *audioBuffer);
bool binocle_audio_is_file_extension(const char *fileName, const char *ext);
static binocle_audio_wave binocle_audio_load_ogg(const char *fileName);
static binocle_audio_wave binocle_audio_load_flac(const char *fileName);
static binocle_audio_wave binocle_audio_load_mp3(const char *fileName);
static binocle_audio_wave binocle_audio_load_wav(const char *fileName);

binocle_audio_sound binocle_audio_load_sound(binocle_audio *audio, const char *fileName);
binocle_audio_sound binocle_audio_load_sound_from_wave(binocle_audio *audio, binocle_audio_wave wave);
void binocle_audio_unload_wave(binocle_audio_wave wave);
void binocle_audio_unload_sound(binocle_audio *audio, binocle_audio_sound sound);
void binocle_audio_update_sound(binocle_audio_sound sound, const void *data, int samplesCount);
void binocle_audio_play_sound(binocle_audio_sound sound);
void binocle_audio_pause_sound(binocle_audio_sound sound);
void binocle_audio_resume_sound(binocle_audio_sound sound);
void binocle_audio_stop_sound(binocle_audio_sound sound);
bool binocle_audio_is_sound_playing(binocle_audio_sound sound);
void binocle_audio_set_sound_volume(binocle_audio_sound sound, float volume);
void binocle_audio_set_sound_pitch(binocle_audio_sound sound, float pitch);

binocle_audio_music *binocle_audio_load_music_stream(binocle_audio *audio, const char *fileName);
void binocle_audio_unload_music_stream(binocle_audio *audio, binocle_audio_music *music);
void binocle_audio_play_music_stream(binocle_audio_music *music);
void binocle_audio_pause_music_stream(binocle_audio_music *music);
void binocle_audio_resume_music_stream(binocle_audio_music *music);
void binocle_audio_stop_music_stream(binocle_audio_music *music);
void binocle_audio_update_music_stream(binocle_audio_music *music);
bool binocle_audio_is_music_playing(binocle_audio_music *music);
void binocle_audio_set_music_volume(binocle_audio_music *music, float volume);
void binocle_audio_set_music_pitch(binocle_audio_music *music, float pitch);
void binocle_audio_set_music_loop_count(binocle_audio_music *music, int count);
float binocle_audio_get_music_time_length(binocle_audio_music *music);
float binocle_audio_get_music_time_played(binocle_audio_music *music);
binocle_audio_stream binocle_audio_init_audio_stream(binocle_audio *audio, unsigned int sampleRate, unsigned int sampleSize, unsigned int channels);
void binocle_audio_close_audio_stream(binocle_audio *audio, binocle_audio_stream stream);
void binocle_audio_update_audio_stream(binocle_audio_stream stream, const void *data, int samplesCount);
bool binocle_audio_is_audio_buffer_processed(binocle_audio_stream stream);
void binocle_audio_play_audio_stream(binocle_audio_stream stream);
void binocle_audio_pause_audio_stream(binocle_audio_stream stream);
void binocle_audio_resume_audio_stream(binocle_audio_stream stream);
bool binocle_audio_is_audio_stream_playing(binocle_audio_stream stream);
void binocle_audio_stop_audio_stream(binocle_audio_stream stream);
void binocle_audio_set_audio_stream_volume(binocle_audio_stream stream, float volume);
void binocle_audio_set_audio_stream_pitch(binocle_audio_stream stream, float pitch);


#endif //BINOCLE_AUDIO_H
