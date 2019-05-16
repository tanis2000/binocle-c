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

/**
 * \brief The kind of buffer usage.
 * It can be static for short sounds and stream for big files and sounds that spawn for over a few seconds
 */
typedef enum binocle_audio_buffer_usage {
  BINOCLE_AUDIO_BUFFER_USAGE_STATIC = 0,
  BINOCLE_AUDIO_BUFFER_USAGE_STREAM
} binocle_audio_buffer_usage;

/**
 * \brief The context type of the music file
 */
typedef enum binocle_audio_music_context_type {
  BINOCLE_AUDIO_MUSIC_AUDIO_OGG = 0,
  BINOCLE_AUDIO_MUSIC_AUDIO_FLAC,
  BINOCLE_AUDIO_MUSIC_AUDIO_MP3,
  BINOCLE_AUDIO_MUSIC_AUDIO_WAV,
  BINOCLE_AUDIO_MUSIC_MODULE_XM,
  BINOCLE_AUDIO_MUSIC_MODULE_MOD
} binocle_audio_music_context_type;

/**
 * \brief represents an audio stream
 * Contains information about the sample rate,bit depth, number of channels, audio format and the buffers
 */
typedef struct binocle_audio_stream {
  unsigned int sample_rate; // Frequency (samples per second)
  unsigned int sample_size; // Bit depth (bits per sample): 8, 16, 32 (24 not supported)
  unsigned int channels; // Number of channels (1-mono, 2-stereo)

  void *audio_buffer; // Pointer to internal data used by the audio system.

  int format; // Audio format specifier
  unsigned int source; // Audio source id
  unsigned int buffers[2]; // Audio buffers (double buffering)
} binocle_audio_stream;

/**
 * \brief A music instance
 * Contains the actual context of the music file, the audio stream and information like looping and the number of
 * samples played and left to play.
 */
typedef struct binocle_audio_music {
  binocle_audio_music_context_type ctx_type;
  stb_vorbis *ctx_ogg;
  drflac *ctx_flac;
  drmp3 ctx_mp3;
  drwav ctx_wav;
  jar_xm_context_t *ctx_xm;
  jar_mod_context_t ctx_mod;

  binocle_audio_stream stream; // Audio stream (double buffering)

  int loop_count; // Loops count (times music repeats), -1 means infinite loop
  unsigned int total_samples; // Total number of samples
  unsigned int samples_left; // Number of samples left to end
} binocle_audio_music;

/**
 * \brief an intermediate structure that contains data loaded from a sound file
 */
typedef struct binocle_audio_wave {
  unsigned int sample_count; // Number of samples
  unsigned int sample_rate; // Frequency (samples per second)
  unsigned int sample_size; // Bit depth (bits per sample): 8, 16, 32 (24 not supported)
  unsigned int channels; // Number of channels (1-mono, 2-stereo)
  void *data; // Buffer data pointer
} binocle_audio_wave;

/**
 * \brief The representation of a sound in the audio system
 */
typedef struct binocle_audio_sound {
  void *audio_buffer; // Pointer to internal data used by the audio system

  unsigned int source; // Audio source id
  unsigned int buffer; // Audio buffer id
  int format; // Audio format specifier
} binocle_audio_sound;

/**
 * \brief An audio buffer
 * Contains information like the PCM converter, volume, pitch, playing status, and pointers to other buffers as a
 * linked list.
 */
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

/**
 * \brief The audio system
 */
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

//
// Audio system
//

/**
 * \brief Creates a new audio system
 * @return The audio system
 */
binocle_audio binocle_audio_new();

/**
 * \brief Initialize a newly created audio system
 * @param audio the audio system
 * @return true everything worked fine
 */
bool binocle_audio_init(binocle_audio *audio);

/**
 * \brief Destroys an audio system
 * Releases all the resources allocated by the audio system
 * @param audio the audio system
 */
void binocle_audio_destroy(binocle_audio *audio);

/**
 * \brief The audio callback as required by miniaudio
 * @param pDevice the miniaudio device
 * @param pOutput the output buffer
 * @param pInput the input buffer
 * @param frameCount the current frame count
 */
void binocle_audio_data_callback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount);

/**
 * \brief Internal function that performs mixing of audio buffers
 * @param pUserData the audio system
 * @param framesOut the output buffer
 * @param framesIn the input buffer
 * @param frameCount the frame count
 * @param localVolume the buffer local volume
 */
static void
binocle_audio_mix_audio_frames(void *pUserData, float *framesOut, const float *framesIn, ma_uint32 frameCount,
                               float localVolume);

/**
 * \brief Returns true if the audio device is ready
 * @param audio the audio system
 * @return true if the audio device is ready
 */
bool binocle_audio_is_audio_device_ready(binocle_audio *audio);

/**
 * \brief Sets the master volume of the audio system
 * @param audio the audio system
 * @param volume the volume [0..1]
 */
void binocle_audio_set_master_volume(binocle_audio *audio, float volume);

/**
 * Allocates a new audio buffer
 * @param audio the audio system
 * @param format the sample format as required by miniaudio
 * @param channels the number of channels [1..2]
 * @param sampleRate the sample rate in Hz
 * @param bufferSizeInFrames the size of the buffer in frames
 * @param usage the kind of audio buffer usage
 * @return a new and initialized audio buffer
 */
binocle_audio_buffer *
binocle_audio_create_audio_buffer(binocle_audio *audio, ma_format format, ma_uint32 channels, ma_uint32 sampleRate,
                                  ma_uint32 bufferSizeInFrames, binocle_audio_buffer_usage usage);

/**
 * \brief Deletes an audio buffer
 * @param audio the audio system
 * @param audioBuffer the audio buffer
 */
void binocle_audio_delete_audio_buffer(binocle_audio *audio, binocle_audio_buffer *audioBuffer);

/**
 * \brief Returns true if the audio buffer is playing
 * @param audio_buffer the audio buffer
 * @return true if playing
 */
bool binocle_audio_is_audio_buffer_playing(binocle_audio_buffer *audio_buffer);

/**
 * \brief Plays the audio buffer
 * @param audio_buffer the audio buffer
 */
void binocle_audio_play_audio_buffer(binocle_audio_buffer *audio_buffer);

/**
 * \brief Stops the audio buffer
 * @param audio_buffer the audio buffer
 */
void binocle_audio_stop_audio_buffer(binocle_audio_buffer *audio_buffer);

/**
 * \brief Pauses the audio buffer
 * @param audio_buffer the audio buffer
 */
void binocle_audio_pause_audio_buffer(binocle_audio_buffer *audio_buffer);

/**
 * \brief Resumes the audio buffer
 * @param audio_buffer the audio buffer
 */
void binocle_audio_resume_audio_buffer(binocle_audio_buffer *audio_buffer);

/**
 * \brief Sets the volume of the audio buffer
 * @param audio_buffer the audio buffer
 * @param volume the folume [0..1]
 */
void binocle_audio_set_audio_buffer_volume(binocle_audio_buffer *audio_buffer, float volume);

/**
 * \brief Sets the audio buffer pitch
 * @param audio_buffer the audio buffer
 * @param pitch the pitch level
 */
void binocle_audio_set_audio_buffer_pitch(binocle_audio_buffer *audio_buffer, float pitch);

/**
 * \brief Adds the audio buffer to the list of audio buffers
 * @param audio the audio system
 * @param audio_buffer the audio buffer
 */
void binocle_audio_track_audio_buffer(binocle_audio *audio, binocle_audio_buffer *audio_buffer);

/**
 * \brief Stops tracking an audio buffer and removes it from the list
 * @param audio the audio system
 * @param audio_buffer the audio buffer
 */
void binocle_audio_untrack_audio_buffer(binocle_audio *audio, binocle_audio_buffer *audio_buffer);

/**
 * \brief Returns true if the file extension is the one specified
 * @param file_name the file name
 * @param ext the extension including the starting dot
 * @return true if the filename has the specified extension
 */
bool binocle_audio_is_file_extension(const char *file_name, const char *ext);

/**
 * \brief Load an OGG audio file
 * @param file_name the file name
 * @return a binocle_audio_wave instance
 */
static binocle_audio_wave binocle_audio_load_ogg(const char *file_name);

/**
 * \brief Load a FLAC audio file
 * @param file_name the file name
 * @return a binocle_audio_wave instance
 */
static binocle_audio_wave binocle_audio_load_flac(const char *file_name);

/**
 * \brief Load an MP3 audio file
 * @param file_name the file name
 * @return a binocle_audio_wave instance
 */
static binocle_audio_wave binocle_audio_load_mp3(const char *file_name);

/**
 * \brief Load a WAV audio file
 * @param file_name the file name
 * @return a binocle_audio_wave instance
 */
static binocle_audio_wave binocle_audio_load_wav(const char *file_name);

//
// Sound
//

/**
 * \brief Loads a sound file
 * @param audio the audio system
 * @param file_name the file to read
 * @return a binocle_audio_sound instance
 */
binocle_audio_sound binocle_audio_load_sound(binocle_audio *audio, const char *file_name);

/**
 * \brief Loads a sound file from an intermediate wave format
 * @param audio the audio system
 * @param wave the binocle_audio_wave struct
 * @return a binocle_audio_sound instance
 */
binocle_audio_sound binocle_audio_load_sound_from_wave(binocle_audio *audio, binocle_audio_wave wave);

/**
 * \brief Releases the resources allocated within a binocle_audio_wave
 * @param wave the wave struct
 */
void binocle_audio_unload_wave(binocle_audio_wave wave);

/**
 * \brief Releases the audio buffer of the sound
 * @param audio the audio system
 * @param sound the sound
 */
void binocle_audio_unload_sound(binocle_audio *audio, binocle_audio_sound sound);

/**
 * \brief Updates the audio buffer of the sound
 * @param sound the sound
 * @param data the buffer with the data to copy from
 * @param samplesCount the number of samples
 */
void binocle_audio_update_sound(binocle_audio_sound sound, const void *data, int samplesCount);

/**
 * \brief Plays a sound
 * @param sound the sound
 */
void binocle_audio_play_sound(binocle_audio_sound sound);

/**
 * \brief Pauses a sound
 * @param sound the sound
 */
void binocle_audio_pause_sound(binocle_audio_sound sound);

/**
 * \brief Resumes a sound
 * @param sound the sound
 */
void binocle_audio_resume_sound(binocle_audio_sound sound);

/**
 * \brief Stops a sound
 * @param sound the sound
 */
void binocle_audio_stop_sound(binocle_audio_sound sound);

/**
 * \brief Returns true if the sound is playing
 * @param sound the sound
 * @return true if the sound is playing
 */
bool binocle_audio_is_sound_playing(binocle_audio_sound sound);

/**
 * \brief Sets the volume of the sound
 * @param sound the sound
 * @param volume the volume [0..1]
 */
void binocle_audio_set_sound_volume(binocle_audio_sound sound, float volume);

/**
 * \brief Sets the pitch of the sound
 * @param sound the sound
 * @param pitch the pitch
 */
void binocle_audio_set_sound_pitch(binocle_audio_sound sound, float pitch);

//
// Music
//

/**
 * \brief Loads a music stream from the given file
 * @param audio the audio system
 * @param file_name the filename of the audio file
 * @return the music struct
 */
binocle_audio_music *binocle_audio_load_music_stream(binocle_audio *audio, const char *file_name);

/**
 * \brief Releases the music stream
 * @param audio the audio system
 * @param music the music struct
 */
void binocle_audio_unload_music_stream(binocle_audio *audio, binocle_audio_music *music);

/**
 * \brief Plays a music stream
 * @param music the music stream
 */
void binocle_audio_play_music_stream(binocle_audio_music *music);

/**
 * \brief Pauses a music stream
 * @param music the music stream
 */
void binocle_audio_pause_music_stream(binocle_audio_music *music);

/**
 * \brief Resumes a music stream
 * @param music the music stream
 */
void binocle_audio_resume_music_stream(binocle_audio_music *music);

/**
 * \brief Stops a music stream
 * @param music the music stream
 */
void binocle_audio_stop_music_stream(binocle_audio_music *music);

/**
 * \brief Updates a music stream
 * @param music the music stream
 */
void binocle_audio_update_music_stream(binocle_audio_music *music);

/**
 * \brief Returns true if the music stream is playing
 * @param music the music stream
 * @return true if the music stream is playing
 */
bool binocle_audio_is_music_playing(binocle_audio_music *music);

/**
 * \brief Sets the music stream volume
 * @param music the music stream
 * @param volume the volume [0..1]
 */
void binocle_audio_set_music_volume(binocle_audio_music *music, float volume);

/**
 * \brief Sets the music stream pitch
 * @param music the music stream
 * @param pitch the pitch
 */
void binocle_audio_set_music_pitch(binocle_audio_music *music, float pitch);

/**
 * \brief Sets the number of loops of the music stream
 * If `count` is -1 then it loops indefinitely
 * @param music the music stream
 * @param count how many times to loop
 */
void binocle_audio_set_music_loop_count(binocle_audio_music *music, int count);

/**
 * \brief Gets the length of the music stream in seconds
 * @param music the music stream
 * @return the length of the music stream in seconds
 */
float binocle_audio_get_music_time_length(binocle_audio_music *music);

/**
 * \brief Gets the length of the music stream already played in seconds
 * @param music the music stream
 * @return the length of the music stream already played in seconds
 */
float binocle_audio_get_music_time_played(binocle_audio_music *music);

/**
 * \brief Initialize an audio stream
 * @param audio the audio system
 * @param sample_rate the sample rate
 * @param sample_size the sample size
 * @param channels the number of channels [1..2]
 * @return an initialized audio stream
 */
binocle_audio_stream
binocle_audio_init_audio_stream(binocle_audio *audio, unsigned int sample_rate, unsigned int sample_size,
                                unsigned int channels);

/**
 * \brief Closes an audio stream and frees its buffer
 * @param audio the audio system
 * @param stream an audio stream
 */
void binocle_audio_close_audio_stream(binocle_audio *audio, binocle_audio_stream stream);

/**
 * \brief Updates an audio stream
 * @param stream the audio stream
 * @param data the buffer with audio samples
 * @param samples_count the number of samples
 */
void binocle_audio_update_audio_stream(binocle_audio_stream stream, const void *data, int samples_count);

/**
 * \brief Returns true if the audio buffer of the stream has been processed
 * @param stream the audio stream
 * @return true if the audio buffer has already been processed
 */
bool binocle_audio_is_audio_buffer_processed(binocle_audio_stream stream);

/**
 * \brief Plays an audio stream
 * @param stream the audio stream
 */
void binocle_audio_play_audio_stream(binocle_audio_stream stream);

/**
 * \brief Pauses an audio stream
 * @param stream the audio stream
 */
void binocle_audio_pause_audio_stream(binocle_audio_stream stream);

/**
 * \brief Resumes an audio stream
 * @param stream the audio stream
 */
void binocle_audio_resume_audio_stream(binocle_audio_stream stream);

/**
 * \brief Returns true if the audio stream is playing
 * @param stream the audio stream
 * @return true if the audio stream is playing
 */
bool binocle_audio_is_audio_stream_playing(binocle_audio_stream stream);

/**
 * \brief Stops an audio stream
 * @param stream the audio stream
 */
void binocle_audio_stop_audio_stream(binocle_audio_stream stream);

/**
 * \brief Sets an audio stream volume
 * @param stream the audio stream
 * @param volume the volume [0..1]
 */
void binocle_audio_set_audio_stream_volume(binocle_audio_stream stream, float volume);

/**
 * \brief Sets an audio stream pitch
 * @param stream the audio stream
 * @param pitch the pitch
 */
void binocle_audio_set_audio_stream_pitch(binocle_audio_stream stream, float pitch);


#endif //BINOCLE_AUDIO_H
