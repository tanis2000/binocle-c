//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_log.h"
#include "binocle_audio.h"

#define STB_VORBIS_IMPLEMENTATION

#include "miniaudio/stb_vorbis.h"

#define DR_FLAC_IMPLEMENTATION

#include "miniaudio/dr_flac.h"

#define DR_MP3_IMPLEMENTATION

#include "miniaudio/dr_mp3.h"

#define DR_WAV_IMPLEMENTATION

#include "miniaudio/dr_wav.h"

#define JAR_XM_IMPLEMENTATION

#include "miniaudio/jar_xm.h"

#define JAR_MOD_IMPLEMENTATION

#include "miniaudio/jar_mod.h"

binocle_audio binocle_audio_new() {
  binocle_audio res = {0};
  res.paused = false;
  //res.active_music = binocle_music_new();
  res.master_volume = 1.0f;
  res.first_audio_buffer = NULL;
  res.last_audio_buffer = NULL;
  res.default_size = 0;
  return res;
}

static void binocle_audio_log_callback(ma_context *context, ma_device *device, ma_uint32 logLevel, const char *message)
{
  (void)context;
  (void)device;

  binocle_log_warning("miniaudio: %s", message);   // All log messages from miniaudio are errors
}

bool binocle_audio_init(binocle_audio *audio) {
  binocle_log_info("Initializing audio module");
  ma_context_config context_config = ma_context_config_init();
  context_config.logCallback = binocle_audio_log_callback;
  ma_result result = ma_context_init(NULL, 0, &context_config, &audio->context);
  if (result != MA_SUCCESS) {
    binocle_log_error("binocle_audio_init(): Failed to initialize audio context");
    return false;
  }

  audio->device_config = ma_device_config_init(ma_device_type_playback);
  audio->device_config.playback.pDeviceID = NULL;
  audio->device_config.playback.format = BINOCLE_AUDIO_DEVICE_FORMAT;
  audio->device_config.playback.channels = BINOCLE_AUDIO_DEVICE_CHANNELS;
  audio->device_config.capture.pDeviceID = NULL;
  audio->device_config.capture.format = ma_format_s16;
  audio->device_config.capture.channels = 1;
  audio->device_config.sampleRate = BINOCLE_AUDIO_DEVICE_SAMPLE_RATE;
  audio->device_config.dataCallback = binocle_audio_data_callback;
  audio->device_config.pUserData = audio;

  binocle_log_info("Initializing audio device");
  if (ma_device_init(&audio->context, &audio->device_config, &audio->device) != MA_SUCCESS) {
    binocle_log_error("binocle_audio_init(): Failed to initialize playback device.\n");
    ma_context_uninit(&audio->context);
    return false;
  }

  binocle_log_info("Starting audio device");
  if (ma_device_start(&audio->device) != MA_SUCCESS) {
    binocle_log_error("binocle_audio_init(): Failed to start playback device.\n");
    ma_device_uninit(&audio->device);
    ma_context_uninit(&audio->context);
    return false;
  }

  if (ma_mutex_init(&audio->lock) != MA_SUCCESS)
  {
    binocle_log_warning( "binocle_audio_init(): Failed to create mutex for mixing");
    ma_device_uninit(&audio->device);
    ma_context_uninit(&audio->context);
    return false;
  }

  // Init dummy audio buffers pool for multichannel sound playing
  for (int i = 0; i < BINOCLE_AUDIO_MAX_AUDIO_BUFFER_POOL_CHANNELS; i++)
  {
    // WARNING: An empty audio buffer is created (data = 0)
    // AudioBuffer data just points to loaded sound data
    audio->multi_channel.pool[i] = binocle_audio_load_audio_buffer(audio, BINOCLE_AUDIO_DEVICE_FORMAT, BINOCLE_AUDIO_DEVICE_CHANNELS, audio->device.sampleRate, 0, BINOCLE_AUDIO_BUFFER_USAGE_STATIC);
  }

  binocle_log_info("Audio device initialized successfully");
  binocle_log_info("Audio backend: miniaudio / %s", ma_get_backend_name(audio->context.backend));
  binocle_log_info("Audio format: %s -> %s", ma_get_format_name(audio->device.playback.format),
                   ma_get_format_name(audio->device.playback.internalFormat));
  binocle_log_info("Audio channels: %d -> %d", audio->device.playback.channels,
                   audio->device.playback.internalChannels);
  binocle_log_info("Audio sample rate: %d -> %d", audio->device.sampleRate, audio->device.playback.internalSampleRate);
  binocle_log_info("Audio buffer size: %d", audio->device.playback.internalPeriodSizeInFrames * audio->device.playback.internalPeriods);

  audio->is_audio_initialized = true;

  return true;
}


void binocle_audio_destroy(binocle_audio *audio) {
  if (!audio->is_audio_initialized) {
    binocle_log_warning("Could not close audio device because it is not currently initialized");
    return;
  }

  for (int i = 0; i < BINOCLE_AUDIO_MAX_AUDIO_BUFFER_POOL_CHANNELS; i++)
  {
    //UnloadAudioBuffer(AUDIO.MultiChannel.pool[i]);
    if (audio->multi_channel.pool[i] != NULL)
    {
      ma_data_converter_uninit(&audio->multi_channel.pool[i]->converter);
      binocle_audio_untrack_audio_buffer(audio, audio->multi_channel.pool[i]);
      free(audio->multi_channel.pool[i]);
    }
  }
  ma_mutex_uninit(&audio->lock);
  ma_device_uninit(&audio->device);
  ma_context_uninit(&audio->context);

  binocle_log_info("Audio device closed successfully");
}

binocle_audio_buffer *binocle_audio_load_audio_buffer(binocle_audio *audio, ma_format format, ma_uint32 channels, ma_uint32 sampleRate, ma_uint32 sizeInFrames, int usage)
{
  binocle_audio_buffer *audioBuffer = (binocle_audio_buffer *)calloc(1, sizeof(binocle_audio_buffer));

  if (audioBuffer == NULL)
  {
    binocle_log_warning("AUDIO: Failed to allocate memory for buffer");
    return NULL;
  }

  if (sizeInFrames > 0) audioBuffer->data = calloc(1, sizeInFrames*channels*ma_get_bytes_per_sample(format));

  // Audio data runs through a format converter
  ma_data_converter_config converterConfig = ma_data_converter_config_init(format, BINOCLE_AUDIO_DEVICE_FORMAT, channels, BINOCLE_AUDIO_DEVICE_CHANNELS, sampleRate, audio->device.sampleRate);
  converterConfig.resampling.allowDynamicSampleRate = true;        // Pitch shifting

  ma_result result = ma_data_converter_init(&converterConfig, &audioBuffer->converter);

  if (result != MA_SUCCESS)
  {
    binocle_log_warning("AUDIO: Failed to create data conversion pipeline");
    free(audioBuffer);
    return NULL;
  }

  // Init audio buffer values
  audioBuffer->volume = 1.0f;
  audioBuffer->pitch = 1.0f;
  audioBuffer->pan = 0.5f;

  audioBuffer->playing = false;
  audioBuffer->paused = false;
  audioBuffer->looping = false;
  audioBuffer->usage = usage;
  audioBuffer->frame_cursor_pos = 0;
    audioBuffer->size_in_frames = sizeInFrames;

  // Buffers should be marked as processed by default so that a call to
  // UpdateAudioStream() immediately after initialization works correctly
  audioBuffer->is_sub_buffer_processed[0] = true;
  audioBuffer->is_sub_buffer_processed[1] = true;

  // Track audio buffer to linked list next position
  binocle_audio_track_audio_buffer(audio, audioBuffer);

  return audioBuffer;
}

void binocle_audio_unload_audio_buffer(binocle_audio *audio, binocle_audio_buffer *buffer)
{
  if (buffer != NULL)
  {
    ma_data_converter_uninit(&buffer->converter);
    binocle_audio_untrack_audio_buffer(audio, buffer);
    free(buffer->data);
    free(buffer);
  }
}

static ma_uint32 binocle_audio_read_audio_buffer_frames_in_internal_format(binocle_audio_buffer *audio_buffer, void *framesOut, ma_uint32 frameCount) {
  ma_uint32 subBufferSizeInFrames = (audio_buffer->size_in_frames > 1)? audio_buffer->size_in_frames/2 : audio_buffer->size_in_frames;
  ma_uint32 currentSubBufferIndex = audio_buffer->frame_cursor_pos/subBufferSizeInFrames;

  if (currentSubBufferIndex > 1) return 0;

  // Another thread can update the processed state of buffers so
  // we just take a copy here to try and avoid potential synchronization problems
  bool isSubBufferProcessed[2] = { 0 };
  isSubBufferProcessed[0] = audio_buffer->is_sub_buffer_processed[0];
  isSubBufferProcessed[1] = audio_buffer->is_sub_buffer_processed[1];

  ma_uint32 frameSizeInBytes = ma_get_bytes_per_frame(audio_buffer->converter.config.formatIn, audio_buffer->converter.config.channelsIn);

  // Fill out every frame until we find a buffer that's marked as processed. Then fill the remainder with 0
  ma_uint32 framesRead = 0;
  while (1)
  {
    // We break from this loop differently depending on the buffer's usage
    //  - For static buffers, we simply fill as much data as we can
    //  - For streaming buffers we only fill the halves of the buffer that are processed
    //    Unprocessed halves must keep their audio data in-tact
    if (audio_buffer->usage == BINOCLE_AUDIO_BUFFER_USAGE_STATIC)
    {
      if (framesRead >= frameCount) break;
    }
    else
    {
      if (isSubBufferProcessed[currentSubBufferIndex]) break;
    }

    ma_uint32 totalFramesRemaining = (frameCount - framesRead);
    if (totalFramesRemaining == 0) break;

    ma_uint32 framesRemainingInOutputBuffer;
    if (audio_buffer->usage == BINOCLE_AUDIO_BUFFER_USAGE_STATIC)
    {
      framesRemainingInOutputBuffer = audio_buffer->size_in_frames - audio_buffer->frame_cursor_pos;
    }
    else
    {
      ma_uint32 firstFrameIndexOfThisSubBuffer = subBufferSizeInFrames*currentSubBufferIndex;
      framesRemainingInOutputBuffer = subBufferSizeInFrames - (audio_buffer->frame_cursor_pos - firstFrameIndexOfThisSubBuffer);
    }

    ma_uint32 framesToRead = totalFramesRemaining;
    if (framesToRead > framesRemainingInOutputBuffer) framesToRead = framesRemainingInOutputBuffer;

    memcpy((unsigned char *)framesOut + (framesRead*frameSizeInBytes), audio_buffer->data + (audio_buffer->frame_cursor_pos*frameSizeInBytes), framesToRead*frameSizeInBytes);
    audio_buffer->frame_cursor_pos = (audio_buffer->frame_cursor_pos + framesToRead)%audio_buffer->size_in_frames;
    framesRead += framesToRead;

    // If we've read to the end of the buffer, mark it as processed
    if (framesToRead == framesRemainingInOutputBuffer)
    {
      audio_buffer->is_sub_buffer_processed[currentSubBufferIndex] = true;
      isSubBufferProcessed[currentSubBufferIndex] = true;

      currentSubBufferIndex = (currentSubBufferIndex + 1)%2;

      // We need to break from this loop if we're not looping
      if (!audio_buffer->looping)
      {
        binocle_audio_stop_audio_buffer(audio_buffer);
        break;
      }
    }
  }

  // Zero-fill excess
  ma_uint32 totalFramesRemaining = (frameCount - framesRead);
  if (totalFramesRemaining > 0)
  {
    memset((unsigned char *)framesOut + (framesRead*frameSizeInBytes), 0, totalFramesRemaining*frameSizeInBytes);

    // For static buffers we can fill the remaining frames with silence for safety, but we don't want
    // to report those frames as "read". The reason for this is that the caller uses the return value
    // to know whether or not a non-looping sound has finished playback.
    if (audio_buffer->usage != BINOCLE_AUDIO_BUFFER_USAGE_STATIC) framesRead += totalFramesRemaining;
  }

  return framesRead;
}

// Reads audio data from an AudioBuffer object in device format. Returned data will be in a format appropriate for mixing.
static ma_uint32 binocle_audio_read_audio_buffer_frames_in_mixing_format(binocle_audio_buffer *audioBuffer, float *framesOut, ma_uint32 frameCount)
{
  // What's going on here is that we're continuously converting data from the AudioBuffer's internal format to the mixing format, which
  // should be defined by the output format of the data converter. We do this until frameCount frames have been output. The important
  // detail to remember here is that we never, ever attempt to read more input data than is required for the specified number of output
  // frames. This can be achieved with ma_data_converter_get_required_input_frame_count().
  ma_uint8 inputBuffer[4096] = { 0 };
  ma_uint32 inputBufferFrameCap = sizeof(inputBuffer)/ma_get_bytes_per_frame(audioBuffer->converter.config.formatIn, audioBuffer->converter.config.channelsIn);

  ma_uint32 totalOutputFramesProcessed = 0;
  while (totalOutputFramesProcessed < frameCount)
  {
    ma_uint64 outputFramesToProcessThisIteration = frameCount - totalOutputFramesProcessed;

    ma_uint64 inputFramesToProcessThisIteration = ma_data_converter_get_required_input_frame_count(&audioBuffer->converter, outputFramesToProcessThisIteration);
    if (inputFramesToProcessThisIteration > inputBufferFrameCap)
    {
      inputFramesToProcessThisIteration = inputBufferFrameCap;
    }

    float *runningFramesOut = framesOut + (totalOutputFramesProcessed*audioBuffer->converter.config.channelsOut);

    /* At this point we can convert the data to our mixing format. */
    ma_uint64 inputFramesProcessedThisIteration = binocle_audio_read_audio_buffer_frames_in_internal_format(audioBuffer, inputBuffer, (ma_uint32)inputFramesToProcessThisIteration);    /* Safe cast. */
    ma_uint64 outputFramesProcessedThisIteration = outputFramesToProcessThisIteration;
    ma_data_converter_process_pcm_frames(&audioBuffer->converter, inputBuffer, &inputFramesProcessedThisIteration, runningFramesOut, &outputFramesProcessedThisIteration);

    totalOutputFramesProcessed += (ma_uint32)outputFramesProcessedThisIteration; /* Safe cast. */

    if (inputFramesProcessedThisIteration < inputFramesToProcessThisIteration)
    {
      break;  /* Ran out of input data. */
    }

    /* This should never be hit, but will add it here for safety. Ensures we get out of the loop when no input nor output frames are processed. */
    if (inputFramesProcessedThisIteration == 0 && outputFramesProcessedThisIteration == 0)
    {
      break;
    }
  }

  return totalOutputFramesProcessed;
}

void binocle_audio_data_callback(ma_device *pDevice, void *pFramesOut, const void *pFramesInput, ma_uint32 frameCount) {
  // Mix
  (void) pDevice;
  binocle_audio *audio = (binocle_audio *) pDevice->pUserData;

  // Init the output buffer to 0
  memset(pFramesOut, 0, frameCount * pDevice->playback.channels * ma_get_bytes_per_sample(pDevice->playback.format));

  // Using a mutex here for thread-safety which makes things not real-time
  // This is unlikely to be necessary for this project, but may want to consider how you might want to avoid this
  ma_mutex_lock(&audio->lock);
  for (binocle_audio_buffer *audio_buffer = audio->first_audio_buffer;
       audio_buffer != NULL; audio_buffer = audio_buffer->next) {
    // Ignore stopped or paused audio.
    if (!audio_buffer->playing || audio_buffer->paused) continue;

    ma_uint32 frames_read = 0;
    while(1) {
      if (frames_read >= frameCount) {
        break;
      }

      // Just read as much data as we can from the stream.
      ma_uint32 framesToRead = (frameCount - frames_read);
      while (framesToRead > 0) {
        float tempBuffer[1024] = {0}; // 512 frames for stereo.

        ma_uint32 framesToReadRightNow = framesToRead;
        if (framesToReadRightNow > sizeof(tempBuffer) / sizeof(tempBuffer[0]) / BINOCLE_AUDIO_DEVICE_CHANNELS) {
          framesToReadRightNow = sizeof(tempBuffer) / sizeof(tempBuffer[0]) / BINOCLE_AUDIO_DEVICE_CHANNELS;
        }

        ma_uint32 framesJustRead =  binocle_audio_read_audio_buffer_frames_in_mixing_format(audio_buffer, tempBuffer,
                                                                                           framesToReadRightNow);
        if (framesJustRead > 0) {
          float *framesOut = (float *) pFramesOut + (frames_read * audio->device.playback.channels);
          float *framesIn = tempBuffer;
          binocle_audio_mix_audio_frames(audio, framesOut, framesIn, framesJustRead, audio_buffer);

          framesToRead -= framesJustRead;
          frames_read += framesJustRead;
        }

        if (!audio_buffer->playing)
        {
          frames_read = frameCount;
          break;
        }

        // If we weren't able to read all the frames we requested, break.
        if (framesJustRead < framesToReadRightNow) {
          if (!audio_buffer->looping) {
            binocle_audio_stop_audio_buffer(audio_buffer);
            break;
          } else {
            // Should never get here, but just for safety,
            // move the cursor position back to the start and continue the loop.
            audio_buffer->frame_cursor_pos = 0;
            continue;
          }
        }
      }

      // If for some reason we weren't able to read every frame we'll need to break from the loop.
      // Not doing this could theoretically put us into an infinite loop.
      if (framesToRead > 0) break;
    }
  }
  ma_mutex_unlock(&audio->lock);
}

static void
binocle_audio_mix_audio_frames(void *pUserData, float *framesOut, const float *framesIn, ma_uint32 frameCount, binocle_audio_buffer* buffer) {
  binocle_audio *audio = (binocle_audio *) pUserData;

  const float localVolume = buffer->volume;

  const ma_uint32 nChannels = audio->device.playback.channels;
  if (nChannels == 2)
  {
    const float left = buffer->pan;
    const float right = 1.0f - left;

    // fast sine approximation in [0..1] for pan law: y = 0.5f * x * (3 - x * x);
    const float levels[2] = { localVolume*0.5f*left*(3.0f-left*left), localVolume*0.5f*right*(3.0f-right*right) };

    float *frameOut = framesOut;
    const float *frameIn = framesIn;
    for (ma_uint32 iFrame = 0; iFrame < frameCount; ++iFrame)
    {
      frameOut[0] += (frameIn[0]*levels[0]);
      frameOut[1] += (frameIn[1]*levels[1]);
      frameOut += 2;
      frameIn += 2;
    }
  }
  else // pan is kinda meaningless
  {
    for (ma_uint32 iFrame = 0; iFrame < frameCount; ++iFrame)
    {
      for (ma_uint32 iChannel = 0; iChannel < nChannels; ++iChannel)
      {
        float *frameOut = framesOut + (iFrame * nChannels);
        const float *frameIn = framesIn + (iFrame * nChannels);

        frameOut[iChannel] += (frameIn[iChannel] * localVolume);
      }
    }
  }
}

bool binocle_audio_is_audio_device_ready(binocle_audio *audio) {
  return audio->is_audio_initialized;
}

void binocle_audio_set_master_volume(binocle_audio *audio, float volume) {
  ma_device_set_master_volume(&audio->device, volume);
}

bool binocle_audio_is_audio_buffer_playing(binocle_audio_buffer *audio_buffer) {
  if (audio_buffer == NULL) {
    binocle_log_debug("binocle_audio_is_audio_buffer_playing() : No audio buffer");
    return false;
  }

  return audio_buffer->playing && !audio_buffer->paused;
}

void binocle_audio_play_audio_buffer(binocle_audio_buffer *audio_buffer) {
  if (audio_buffer == NULL) {
    binocle_log_error("binocle_audio_play_audio_buffer() : No audio buffer");
    return;
  }

  audio_buffer->playing = true;
  audio_buffer->paused = false;
  audio_buffer->frame_cursor_pos = 0;
}

void binocle_audio_stop_audio_buffer(binocle_audio_buffer *audio_buffer) {
  if (audio_buffer == NULL) {
    binocle_log_debug("binocle_audio_stop_audio_buffer() : No audio buffer");
    return;
  }

  // Don't do anything if the audio buffer is already stopped.
  if (!binocle_audio_is_audio_buffer_playing(audio_buffer)) return;

  audio_buffer->playing = false;
  audio_buffer->paused = false;
  audio_buffer->frame_cursor_pos = 0;
  audio_buffer->frames_processed = 0;
  audio_buffer->is_sub_buffer_processed[0] = true;
  audio_buffer->is_sub_buffer_processed[1] = true;
}

void binocle_audio_pause_audio_buffer(binocle_audio_buffer *audio_buffer) {
  if (audio_buffer == NULL) {
    binocle_log_error("binocle_audio_pause_audio_buffer() : No audio buffer");
    return;
  }

  audio_buffer->paused = true;
}

void binocle_audio_resume_audio_buffer(binocle_audio_buffer *audio_buffer) {
  if (audio_buffer == NULL) {
    binocle_log_error("binocle_audio_resume_audio_buffer() : No audio buffer");
    return;
  }

  audio_buffer->paused = false;
}

void binocle_audio_set_audio_buffer_volume(binocle_audio_buffer *audio_buffer, float volume) {
  if (audio_buffer == NULL) {
    binocle_log_error("binocle_audio_set_audio_buffer_volume() : No audio buffer");
    return;
  }

  audio_buffer->volume = volume;
}

void binocle_audio_set_audio_buffer_pitch(binocle_audio_buffer *audio_buffer, float pitch) {
  if (audio_buffer == NULL) {
    binocle_log_error("binocle_audio_set_audio_buffer_pitch() : No audio buffer");
    return;
  }

  if (pitch <= 0.0f) {
    return;
  }

  // Pitching is just an adjustment of the sample rate. Note that this changes the duration of the sound - higher pitches
  // will make the sound faster; lower pitches make it slower.
  ma_uint32 newOutputSampleRate = (ma_uint32)((float) audio_buffer->converter.config.sampleRateOut / pitch);
  audio_buffer->pitch *= (float) audio_buffer->converter.config.sampleRateOut / newOutputSampleRate;
  audio_buffer->pitch = pitch;
}

void binocle_audio_set_audio_buffer_pan(binocle_audio_buffer *buffer, float pan)
{
  if (buffer == NULL) {
    binocle_log_error("binocle_audio_set_audio_buffer_pan() : No audio buffer");
    return;
  }
  if (pan < 0.0f) pan = 0.0f;
  else if (pan > 1.0f) pan = 1.0f;

  buffer->pan = pan;
}

void binocle_audio_track_audio_buffer(binocle_audio *audio, binocle_audio_buffer *audio_buffer) {
  if (audio_buffer == NULL) {
    binocle_log_error("binocle_audio_track_audio_buffer() : No audio buffer");
    return;
  }

  ma_mutex_lock(&audio->lock);
  if (audio->first_audio_buffer == NULL) audio->first_audio_buffer = audio_buffer;
  else {
    audio->last_audio_buffer->next = audio_buffer;
    audio_buffer->prev = audio->last_audio_buffer;
  }

  audio->last_audio_buffer = audio_buffer;
  ma_mutex_unlock(&audio->lock);
}

void binocle_audio_untrack_audio_buffer(binocle_audio *audio, binocle_audio_buffer *audio_buffer) {
  if (audio_buffer == NULL) {
    binocle_log_error("binocle_audio_track_audio_buffer() : No audio buffer");
    return;
  }

  ma_mutex_lock(&audio->lock);
  if (audio_buffer->prev == NULL) audio->first_audio_buffer = audio_buffer->next;
  else audio_buffer->prev->next = audio_buffer->next;

  if (audio_buffer->next == NULL) audio->last_audio_buffer = audio_buffer->prev;
  else audio_buffer->next->prev = audio_buffer->prev;

  audio_buffer->prev = NULL;
  audio_buffer->next = NULL;
  ma_mutex_unlock(&audio->lock);
}

//
// Sound loading and playing stuff
//

bool binocle_audio_is_file_extension(const char *file_name, const char *ext) {
  bool result = false;
  const char *file_ext;

  if ((file_ext = strrchr(file_name, '.')) != NULL) {
    if (strcmp(file_ext, ext) == 0) result = true;
  }

  return result;
}


binocle_audio_wave binocle_audio_load_wave(const char *fileName) {
  binocle_audio_wave wave = {0};

  if (binocle_audio_is_file_extension(fileName, ".wav")) {
    wave = binocle_audio_load_wav(fileName);
  } else if (binocle_audio_is_file_extension(fileName, ".ogg")) {
    wave = binocle_audio_load_ogg(fileName);
  } else if (binocle_audio_is_file_extension(fileName, ".flac")) {
    wave = binocle_audio_load_flac(fileName);
  } else if (binocle_audio_is_file_extension(fileName, ".mp3")) {
    wave = binocle_audio_load_mp3(fileName);
  } else {
    binocle_log_warning("[%s] Audio fileformat not supported, it can't be loaded", fileName);
  }

  return wave;
}

static binocle_audio_wave binocle_audio_load_ogg(const char *file_name) {
  binocle_audio_wave wave = {0};

  stb_vorbis *oggFile = stb_vorbis_open_filename(file_name, NULL, NULL);

  if (oggFile == NULL) binocle_log_warning("[%s] OGG file could not be opened", file_name);
  else {
    stb_vorbis_info info = stb_vorbis_get_info(oggFile);

    wave.sample_rate = info.sample_rate;
    wave.sample_size = 16;                   // 16 bit per sample (short)
    wave.channels = info.channels;
    wave.sample_count =
        (unsigned int) stb_vorbis_stream_length_in_samples(oggFile) * info.channels;  // Independent by channel

    float totalSeconds = stb_vorbis_stream_length_in_seconds(oggFile);
    if (totalSeconds > 10)
      binocle_log_warning(
          "[%s] Ogg audio length is larger than 10 seconds (%f), that's a big file in memory, consider music streaming",
          file_name, totalSeconds);

    wave.data = (short *) calloc(1, wave.sample_count * wave.channels * sizeof(short));

    // NOTE: Returns the number of samples to process (be careful! we ask for number of shorts!)
    int numSamplesOgg = stb_vorbis_get_samples_short_interleaved(oggFile, info.channels, (short *) wave.data,
                                                                 wave.sample_count * wave.channels);

    binocle_log_debug("[%s] Samples obtained: %i", file_name, numSamplesOgg);

    binocle_log_info("[%s] OGG file loaded successfully (%i Hz, %i bit, %s)", file_name, wave.sample_rate,
                     wave.sample_size, (wave.channels == 1) ? "Mono" : "Stereo");

    stb_vorbis_close(oggFile);
  }

  return wave;
}

static binocle_audio_wave binocle_audio_load_flac(const char *file_name) {
  binocle_audio_wave wave;

  // Decode an entire FLAC file in one go
  uint64_t totalSampleCount;
  wave.data = drflac_open_file_and_read_pcm_frames_s16(file_name, &wave.channels, &wave.sample_rate, &totalSampleCount, NULL);

  wave.sample_count = (unsigned int) totalSampleCount;
  wave.sample_size = 16;

  // NOTE: Only support up to 2 channels (mono, stereo)
  if (wave.channels > 2) binocle_log_warning("[%s] FLAC channels number (%i) not supported", file_name, wave.channels);

  if (wave.data == NULL) binocle_log_warning("[%s] FLAC data could not be loaded", file_name);
  else
    binocle_log_info("[%s] FLAC file loaded successfully (%i Hz, %i bit, %s)", file_name, wave.sample_rate,
                     wave.sample_size, (wave.channels == 1) ? "Mono" : "Stereo");

  return wave;
}

static binocle_audio_wave binocle_audio_load_mp3(const char *file_name) {
  binocle_audio_wave wave = {0};

  // Decode an entire MP3 file in one go
  uint64_t totalFrameCount = 0;
  drmp3_config config = {0};
  wave.data = drmp3_open_file_and_read_pcm_frames_f32(file_name, &config, &totalFrameCount, NULL);

  wave.channels = config.channels;
  wave.sample_rate = config.sampleRate;
  wave.sample_count = (int) totalFrameCount * wave.channels;
  wave.sample_size = 32;

  // NOTE: Only support up to 2 channels (mono, stereo)
  if (wave.channels > 2) binocle_log_warning("[%s] MP3 channels number (%i) not supported", file_name, wave.channels);

  if (wave.data == NULL) binocle_log_warning("[%s] MP3 data could not be loaded", file_name);
  else
    binocle_log_info("[%s] MP3 file loaded successfully (%i Hz, %i bit, %s)", file_name, wave.sample_rate,
                     wave.sample_size, (wave.channels == 1) ? "Mono" : "Stereo");

  return wave;
}

static binocle_audio_wave binocle_audio_load_wav(const char *file_name) {
  binocle_audio_wave wave = {0};

  // Decode an entire MP3 file in one go
  uint64_t totalFrameCount = 0;
  wave.data = drwav_open_file_and_read_pcm_frames_f32(file_name, &wave.channels, &wave.sample_rate, &totalFrameCount, NULL);
  wave.sample_count = (int) totalFrameCount * wave.channels;
  wave.sample_size = 32;

  // NOTE: Only support up to 2 channels (mono, stereo)
  if (wave.channels > 2) binocle_log_warning("[%s] WAV channels number (%i) not supported", file_name, wave.channels);

  if (wave.data == NULL) binocle_log_warning("[%s] WAV data could not be loaded", file_name);
  else
    binocle_log_info("[%s] WAV file loaded successfully (%i Hz, %i bit, %s)", file_name, wave.sample_rate,
                     wave.sample_size, (wave.channels == 1) ? "Mono" : "Stereo");

  return wave;
}

binocle_audio_sound binocle_audio_load_sound(binocle_audio *audio, const char *file_name) {
  binocle_audio_wave wave = binocle_audio_load_wave(file_name);

  binocle_audio_sound sound = binocle_audio_load_sound_from_wave(audio, wave);

  binocle_audio_unload_wave(wave);       // Sound is loaded, we can unload wave

  return sound;
}

binocle_audio_sound binocle_audio_load_sound_from_wave(binocle_audio *audio, binocle_audio_wave wave) {
  binocle_audio_sound sound = {0};

  if (wave.data != NULL) {
    // When using miniaudio we need to do our own mixing. To simplify this we need convert the format of each sound to be consistent with
    // the format used to open the playback device. We can do this two ways:
    //
    //   1) Convert the whole sound in one go at load time (here).
    //   2) Convert the audio data in chunks at mixing time.
    //
    // I have decided on the first option because it offloads work required for the format conversion to the to the loading stage.
    // The downside to this is that it uses more memory if the original sound is u8 or s16.
    ma_format formatIn = ((wave.sample_size == 8) ? ma_format_u8 : ((wave.sample_size == 16) ? ma_format_s16
                                                                                             : ma_format_f32));
    ma_uint32 frameCountIn = wave.sample_count / wave.channels;

    ma_uint32 frameCount = (ma_uint32) ma_convert_frames(NULL, 0, BINOCLE_AUDIO_DEVICE_FORMAT, BINOCLE_AUDIO_DEVICE_CHANNELS,
                                                         audio->device.sampleRate, NULL, frameCountIn,
                                                        formatIn, wave.channels,
                                                         wave.sample_rate);
    if (frameCount == 0) binocle_log_warning("binocle_audio_load_sound_from_wave() : Failed to get frame count for format conversion");

    binocle_audio_buffer *audioBuffer = binocle_audio_load_audio_buffer(audio, BINOCLE_AUDIO_DEVICE_FORMAT,
                                                                        BINOCLE_AUDIO_DEVICE_CHANNELS,
                                                                        audio->device.sampleRate, frameCount,
                                                                          BINOCLE_AUDIO_BUFFER_USAGE_STATIC);
    if (audioBuffer == NULL) binocle_log_warning("binocle_audio_load_sound_from_wave() : Failed to create audio buffer");

    frameCount = (ma_uint32) ma_convert_frames(audioBuffer->data,
                                              frameCount,
                                              BINOCLE_AUDIO_DEVICE_FORMAT,
                                              BINOCLE_AUDIO_DEVICE_CHANNELS,
                                              audio->device.sampleRate,
                                              wave.data,
                                              frameCountIn,
                                              formatIn,
                                              wave.channels,
                                              wave.sample_rate);
    if (frameCount == 0) binocle_log_warning("binocle_audio_load_sound_from_wave() : Format conversion failed");


    sound.frame_count = frameCount;
    sound.stream.sample_rate = audio->device.sampleRate;
    sound.stream.sample_size = 32;
    sound.stream.channels = BINOCLE_AUDIO_DEVICE_CHANNELS;
    sound.stream.buffer = audioBuffer;
  }

  return sound;
}

void binocle_audio_unload_wave(binocle_audio_wave wave) {
  if (wave.data != NULL) free(wave.data);

  binocle_log_info("Unloaded wave data from RAM");
}

void binocle_audio_unload_sound(binocle_audio *audio, binocle_audio_sound sound) {
  binocle_audio_unload_audio_buffer(audio, sound.stream.buffer);

  binocle_log_info("Unloaded sound data from RAM");
}

void binocle_audio_update_sound(binocle_audio_sound sound, const void *data, int samplesCount) {
  binocle_audio_buffer *audioBuffer = sound.stream.buffer;

  if (audioBuffer == NULL) {
    binocle_log_error("binocle_audio_update_sound() : Invalid sound - no audio buffer");
    return;
  }

  binocle_audio_stop_audio_buffer(audioBuffer);

  // TODO: May want to lock/unlock this since this data buffer is read at mixing time.
  memcpy(audioBuffer->data,
         data,
         samplesCount * ma_get_bytes_per_frame(audioBuffer->converter.config.formatIn,
                                               audioBuffer->converter.config.channelsIn));
}

void binocle_audio_play_sound(binocle_audio_sound sound) {
  binocle_audio_play_audio_buffer(sound.stream.buffer);
}

void binocle_audio_play_sound_multi(binocle_audio *audio, binocle_audio_sound sound) {
  int index = -1;
  unsigned int oldAge = 0;
  int oldIndex = -1;

  // find the first non playing pool entry
  for (int i = 0; i < BINOCLE_AUDIO_MAX_AUDIO_BUFFER_POOL_CHANNELS; i++)
  {
    if (audio->multi_channel.channels[i] > oldAge)
    {
      oldAge = audio->multi_channel.channels[i];
      oldIndex = i;
    }

    if (!binocle_audio_is_audio_buffer_playing(audio->multi_channel.pool[i]))
    {
      index = i;
      break;
    }
  }

  // If no none playing pool members can be index choose the oldest
  if (index == -1)
  {
    binocle_log_warning("SOUND: Buffer pool is already full, count: %i", audio->multi_channel.pool_counter);

    if (oldIndex == -1)
    {
      // Shouldn't be able to get here... but just in case something odd happens!
      binocle_log_warning("SOUND: Buffer pool could not determine oldest buffer not playing sound");
      return;
    }

    index = oldIndex;

    // Just in case...
    binocle_audio_stop_audio_buffer(audio->multi_channel.pool[index]);
  }

  // Experimentally mutex lock doesn't seem to be needed this makes sense
  // as pool[index] isn't playing and the only stuff we're copying
  // shouldn't be changing...

  audio->multi_channel.channels[index] = audio->multi_channel.pool_counter;
  audio->multi_channel.pool_counter++;

  binocle_audio_set_audio_buffer_volume(audio->multi_channel.pool[index], sound.stream.buffer->volume);
  binocle_audio_set_audio_buffer_pitch(audio->multi_channel.pool[index], sound.stream.buffer->pitch);
  binocle_audio_set_audio_buffer_pan(audio->multi_channel.pool[index], sound.stream.buffer->pan);

  audio->multi_channel.pool[index]->looping = sound.stream.buffer->looping;
  audio->multi_channel.pool[index]->usage = sound.stream.buffer->usage;
  audio->multi_channel.pool[index]->is_sub_buffer_processed[0] = false;
  audio->multi_channel.pool[index]->is_sub_buffer_processed[1] = false;
  audio->multi_channel.pool[index]->size_in_frames = sound.stream.buffer->size_in_frames;
  audio->multi_channel.pool[index]->data = sound.stream.buffer->data;

  binocle_audio_play_audio_buffer(audio->multi_channel.pool[index]);
}

void binocle_audio_stop_sound_multi(binocle_audio *audio)
{
  for (int i = 0; i < BINOCLE_AUDIO_MAX_AUDIO_BUFFER_POOL_CHANNELS; i++) binocle_audio_stop_audio_buffer(audio->multi_channel.pool[i]);
}

int binocle_audio_get_sounds_playing(binocle_audio *audio)
{
  int counter = 0;

  for (int i = 0; i < BINOCLE_AUDIO_MAX_AUDIO_BUFFER_POOL_CHANNELS; i++)
  {
    if (binocle_audio_is_audio_buffer_playing(audio->multi_channel.pool[i])) counter++;
  }

  return counter;
}

void binocle_audio_pause_sound(binocle_audio_sound sound) {
  binocle_audio_pause_audio_buffer(sound.stream.buffer);
}

void binocle_audio_resume_sound(binocle_audio_sound sound) {
  binocle_audio_resume_audio_buffer(sound.stream.buffer);
}

void binocle_audio_stop_sound(binocle_audio_sound sound) {
  binocle_audio_stop_audio_buffer(sound.stream.buffer);
}

bool binocle_audio_is_sound_playing(binocle_audio_sound sound) {
  return binocle_audio_is_audio_buffer_playing(sound.stream.buffer);
}

void binocle_audio_set_sound_volume(binocle_audio_sound sound, float volume) {
  binocle_audio_set_audio_buffer_volume(sound.stream.buffer, volume);
}

void binocle_audio_set_sound_pitch(binocle_audio_sound sound, float pitch) {
  binocle_audio_set_audio_buffer_pitch(sound.stream.buffer, pitch);
}

void binocle_audio_set_sound_pan(binocle_audio_sound sound, float pan) {
  binocle_audio_set_audio_buffer_pan(sound.stream.buffer, pan);
}

//
// Music stuff
//

binocle_audio_music binocle_audio_load_music_stream(binocle_audio *audio, const char *file_name) {
  binocle_audio_music music = {0};
  bool musicLoaded = false;

  if (binocle_audio_is_file_extension(file_name, ".wav")) {
    drwav ctxWav = {0};
    bool success = drwav_init_file(&ctxWav, file_name, NULL);

    music.ctx_type = BINOCLE_AUDIO_MUSIC_AUDIO_WAV;
    music.ctx_wav = ctxWav;

    if (success)
    {
      int sampleSize = ctxWav.bitsPerSample;
      if (ctxWav.bitsPerSample == 24) sampleSize = 16;   // Forcing conversion to s16 on UpdateMusicStream()

      music.stream = binocle_audio_load_audio_stream(audio, ctxWav.sampleRate, sampleSize, ctxWav.channels);
      music.frame_count = (unsigned int)ctxWav.totalPCMFrameCount;
      music.looping = true;   // Looping enabled by default
      musicLoaded = true;
    }
  } else if (binocle_audio_is_file_extension(file_name, ".ogg")) {
    music.ctx_type = BINOCLE_AUDIO_MUSIC_AUDIO_OGG;
    music.ctx_ogg = stb_vorbis_open_filename(file_name, NULL, NULL);

    if (music.ctx_ogg != NULL) {
      stb_vorbis_info info = stb_vorbis_get_info(music.ctx_ogg);  // Get Ogg file info

      // OGG bit rate defaults to 16 bit, it's enough for compressed format
      music.stream = binocle_audio_load_audio_stream(audio, info.sample_rate, 16, info.channels);
      music.frame_count = (unsigned int) stb_vorbis_stream_length_in_samples(music.ctx_ogg);
      music.looping = true;
      musicLoaded = true;
    }
  } else if (binocle_audio_is_file_extension(file_name, ".flac")) {
    music.ctx_type = BINOCLE_AUDIO_MUSIC_AUDIO_FLAC;
    music.ctx_flac = drflac_open_file(file_name, NULL);

    if (music.ctx_flac != NULL) {
      music.stream = binocle_audio_load_audio_stream(audio, music.ctx_flac->sampleRate,
                                                      music.ctx_flac->bitsPerSample, music.ctx_flac->channels);
      music.frame_count = (unsigned int) music.ctx_flac->totalPCMFrameCount;
      music.looping = true;
      musicLoaded = true;
    }
  } else if (binocle_audio_is_file_extension(file_name, ".mp3")) {
    music.ctx_type = BINOCLE_AUDIO_MUSIC_AUDIO_MP3;
    int result = drmp3_init_file(&music.ctx_mp3, file_name, NULL);

    if (result > 0) {
      music.stream = binocle_audio_load_audio_stream(audio, music.ctx_mp3.sampleRate, 32, music.ctx_mp3.channels);

      music.frame_count = drmp3_get_pcm_frame_count(&music.ctx_mp3);
      music.looping = true;
      musicLoaded = true;
    }
  } else if (binocle_audio_is_file_extension(file_name, ".xm")) {
    music.ctx_type = BINOCLE_AUDIO_MUSIC_MODULE_XM;
    int result = jar_xm_create_context_from_file(&music.ctx_xm, audio->device.sampleRate, file_name);

    if (result == 0)
    {
      jar_xm_set_max_loop_count(music.ctx_xm, 0); // Set infinite number of loops

      unsigned int bits = 32;
      if (BINOCLE_AUDIO_DEVICE_FORMAT == ma_format_s16) bits = 16;
      else if (BINOCLE_AUDIO_DEVICE_FORMAT == ma_format_u8) bits = 8;
      // NOTE: Only stereo is supported for XM
      music.stream = binocle_audio_load_audio_stream(audio, audio->device.sampleRate, bits, BINOCLE_AUDIO_DEVICE_CHANNELS);
      music.frame_count = jar_xm_get_remaining_samples(music.ctx_xm);
      music.looping = true;
      musicLoaded = true;
    }
  } else if (binocle_audio_is_file_extension(file_name, ".mod")) {
    music.ctx_type = BINOCLE_AUDIO_MUSIC_MODULE_MOD;
    jar_mod_init(&music.ctx_mod);

    if (jar_mod_load_file(&music.ctx_mod, file_name)) {
      // NOTE: Only stereo is supported for MOD
      music.stream = binocle_audio_load_audio_stream(audio, audio->device.sampleRate, 16, BINOCLE_AUDIO_DEVICE_CHANNELS);
      music.frame_count = (unsigned int) jar_mod_max_samples(&music.ctx_mod);
      music.looping = true;
      musicLoaded = true;
    }
  } else {
    binocle_log_warning("File format not supported: %s", file_name);
  }

  if (!musicLoaded) {
    if (music.ctx_type == BINOCLE_AUDIO_MUSIC_AUDIO_WAV) drwav_uninit(&music.ctx_wav);
    else if (music.ctx_type == BINOCLE_AUDIO_MUSIC_AUDIO_OGG) stb_vorbis_close(music.ctx_ogg);
    else if (music.ctx_type == BINOCLE_AUDIO_MUSIC_AUDIO_FLAC) drflac_free(music.ctx_flac, NULL);
    else if (music.ctx_type == BINOCLE_AUDIO_MUSIC_AUDIO_MP3) drmp3_uninit(&music.ctx_mp3);
    else if (music.ctx_type == BINOCLE_AUDIO_MUSIC_MODULE_XM) jar_xm_free_context(music.ctx_xm);
    else if (music.ctx_type == BINOCLE_AUDIO_MUSIC_MODULE_MOD) jar_mod_unload(&music.ctx_mod);

    binocle_log_warning("[%s] Music file could not be opened", file_name);
  } else {
    binocle_log_info("[%s] Music file loaded successfully", file_name);
    binocle_log_info("    > Sample rate:   %i Hz", music.stream.sample_rate);
    binocle_log_info("    > Sample size:   %i bits", music.stream.sample_size);
    binocle_log_info("    > Channels:      %i (%s)", music.stream.channels, (music.stream.channels == 1)? "Mono" : (music.stream.channels == 2)? "Stereo" : "Multi");
    binocle_log_info("    > Total frames:  %i", music.frame_count);
  }

  return music;
}

void binocle_audio_unload_music_stream(binocle_audio *audio, binocle_audio_music *music) {
  if (music == NULL) return;

  binocle_audio_unload_audio_stream(audio, music->stream);

  if (music->ctx_type == BINOCLE_AUDIO_MUSIC_AUDIO_WAV) drwav_uninit(&music->ctx_wav);
  else if (music->ctx_type == BINOCLE_AUDIO_MUSIC_AUDIO_OGG) stb_vorbis_close(music->ctx_ogg);
  else if (music->ctx_type == BINOCLE_AUDIO_MUSIC_AUDIO_FLAC) drflac_free(music->ctx_flac, NULL);
  else if (music->ctx_type == BINOCLE_AUDIO_MUSIC_AUDIO_MP3) drmp3_uninit(&music->ctx_mp3);
  else if (music->ctx_type == BINOCLE_AUDIO_MUSIC_MODULE_XM) jar_xm_free_context(music->ctx_xm);
  else if (music->ctx_type == BINOCLE_AUDIO_MUSIC_MODULE_MOD) jar_mod_unload(&music->ctx_mod);
}

void binocle_audio_play_music_stream(binocle_audio_music *music) {
  if (music == NULL) {
    binocle_log_warning("binocle_audio_play_music_stream() : Music is undefined");
    return;
  }

  binocle_audio_buffer *audioBuffer = (binocle_audio_buffer *) music->stream.buffer;

  if (audioBuffer == NULL) {
    binocle_log_error("binocle_audio_play_music_stream() : No audio buffer");
    return;
  }

  // For music streams, we need to make sure we maintain the frame cursor position. This is hack for this section of code in UpdateMusicStream()
  //     // NOTE: In case window is minimized, music stream is stopped,
  //     // just make sure to play again on window restore
  //     if (IsMusicPlaying(music)) PlayMusicStream(music);
  ma_uint32 frameCursorPos = audioBuffer->frame_cursor_pos;

  binocle_audio_play_audio_stream(music->stream); // <-- This resets the cursor position.

  audioBuffer->frame_cursor_pos = frameCursorPos;

}

void binocle_audio_pause_music_stream(binocle_audio_music *music) {
  if (music != NULL) binocle_audio_pause_audio_stream(music->stream);
}

void binocle_audio_resume_music_stream(binocle_audio_music *music) {
  if (music != NULL) binocle_audio_resume_audio_stream(music->stream);
}

void binocle_audio_stop_music_stream(binocle_audio_music *music) {
  if (music == NULL) {
    binocle_log_warning("binocle_audio_stop_music_stream() : Music is undefined");
    return;
  }

  binocle_audio_stop_audio_stream(music->stream);

  // Restart music context
  switch (music->ctx_type) {
  case BINOCLE_AUDIO_MUSIC_AUDIO_WAV:
    drwav_seek_to_pcm_frame(&music->ctx_wav, 0);
    break;
    case BINOCLE_AUDIO_MUSIC_AUDIO_OGG:
      stb_vorbis_seek_start(music->ctx_ogg);
      break;
    case BINOCLE_AUDIO_MUSIC_AUDIO_FLAC:
      drflac_seek_to_pcm_frame(music->ctx_flac, 0);
      break;
    case BINOCLE_AUDIO_MUSIC_AUDIO_MP3:
      drmp3_seek_to_pcm_frame(&music->ctx_mp3, 0);
      break;
    case BINOCLE_AUDIO_MUSIC_MODULE_XM: /* TODO: seek to start of XM */ break;
    case BINOCLE_AUDIO_MUSIC_MODULE_MOD:
      jar_mod_seek_start(&music->ctx_mod);
      break;
    default:
      break;
  }
}

void binocle_audio_seek_music_stream(binocle_audio_music *music, unsigned int frame) {
  if (music == NULL) {
    binocle_log_warning("binocle_audio_seek_music_stream() : Music is undefined");
    return;
  }

  // Seek the music context
  switch (music->ctx_type) {
  case BINOCLE_AUDIO_MUSIC_AUDIO_WAV:
    drwav_seek_to_pcm_frame(&music->ctx_wav, frame);
    break;
    case BINOCLE_AUDIO_MUSIC_AUDIO_OGG:
      stb_vorbis_seek(music->ctx_ogg, frame);
      break;
    case BINOCLE_AUDIO_MUSIC_AUDIO_FLAC:
      drflac_seek_to_pcm_frame(music->ctx_flac, frame);
      break;
    case BINOCLE_AUDIO_MUSIC_AUDIO_MP3:
      drmp3_seek_to_pcm_frame(&music->ctx_mp3, frame);
      break;
    case BINOCLE_AUDIO_MUSIC_MODULE_XM: /* TODO: seek to sample of XM */ break;
    case BINOCLE_AUDIO_MUSIC_MODULE_MOD:
      /* TODO seek to sample of mod */
      break;
    default:
      break;
  }
}

void binocle_audio_update_music_stream(binocle_audio_music *music) {
  if (music == NULL) {
    binocle_log_warning("binocle_audio_update_music_stream() : Music is undefined");
    return;
  }

  if (music->stream.buffer == NULL) {
    binocle_log_warning("binocle_audio_update_music_stream() : Buffer is undefined");
    return;
  }

  bool streamEnding = false;

  unsigned int subBufferSizeInFrames = music->stream.buffer->size_in_frames / 2;

  // NOTE: Using dynamic allocation because it could require more than 16KB
  void *pcm = calloc(subBufferSizeInFrames * music->stream.channels * music->stream.sample_size / 8, 1);

  int frameCountToStream = 0; // Total size of data in frames to be streamed
  unsigned int framesLeft = music->frame_count - music->stream.buffer->frames_processed;

  while (binocle_audio_is_audio_stream_processed(music->stream)) {
    if (framesLeft >= subBufferSizeInFrames) frameCountToStream = subBufferSizeInFrames;
    else frameCountToStream = framesLeft;

    switch (music->ctx_type) {
    case BINOCLE_AUDIO_MUSIC_AUDIO_WAV: {
      // NOTE: Returns the number of samples to process (not required)
      if (music->stream.sample_size == 16) drwav_read_pcm_frames_s16(&music->ctx_wav, frameCountToStream, (short *)pcm);
      else if (music->stream.sample_size == 32) drwav_read_pcm_frames_f32(&music->ctx_wav, frameCountToStream, (float *)pcm);
    }
    break;
      case BINOCLE_AUDIO_MUSIC_AUDIO_OGG: {
        // NOTE: Returns the number of samples to process (be careful! we ask for number of shorts!)
        stb_vorbis_get_samples_short_interleaved(music->ctx_ogg, music->stream.channels, (short *) pcm, frameCountToStream*music->stream.channels);

      }
        break;
      case BINOCLE_AUDIO_MUSIC_AUDIO_FLAC: {
        // NOTE: Returns the number of samples to process
        unsigned int numSamplesFlac = (unsigned int) drflac_read_pcm_frames_s16(music->ctx_flac, frameCountToStream*music->stream.channels,
                                                                                (short *) pcm);

      }
        break;
      case BINOCLE_AUDIO_MUSIC_AUDIO_MP3: {
        // NOTE: samplesCount, actually refers to framesCount and returns the number of frames processed
        drmp3_read_pcm_frames_f32(&music->ctx_mp3, frameCountToStream, (float *) pcm);
      }
        break;
      case BINOCLE_AUDIO_MUSIC_MODULE_XM: {
        // NOTE: Internally we consider 2 channels generation, so sampleCount/2
        if (BINOCLE_AUDIO_DEVICE_FORMAT == ma_format_f32) jar_xm_generate_samples((jar_xm_context_t *)music->ctx_xm, (float *)pcm, frameCountToStream);
        else if (BINOCLE_AUDIO_DEVICE_FORMAT == ma_format_s16) jar_xm_generate_samples_16bit((jar_xm_context_t *)music->ctx_xm, (short *)pcm, frameCountToStream);
        else if (BINOCLE_AUDIO_DEVICE_FORMAT == ma_format_u8) jar_xm_generate_samples_8bit((jar_xm_context_t *)music->ctx_xm, (char *)pcm, frameCountToStream);
      }
        break;
      case BINOCLE_AUDIO_MUSIC_MODULE_MOD: {
        // NOTE: 3rd parameter (nbsample) specify the number of stereo 16bits samples you want, so sampleCount/2
        jar_mod_fillbuffer(&music->ctx_mod, (short *) pcm, frameCountToStream, 0);
      }
        break;
      default:
        break;
    }


    binocle_audio_update_audio_stream(music->stream, pcm, frameCountToStream);

    framesLeft -= frameCountToStream;

    if (framesLeft <= 0)
    {
      streamEnding = true;
      break;
    }
  }

  // Free allocated pcm data
  free(pcm);

  if (streamEnding) {
    binocle_audio_stop_music_stream(music);        // Stop music (and reset)

    // Decrease loopCount to stop when required
    if (music->looping) {
      binocle_audio_play_music_stream(music);    // Play again
    }
  } else {
    // NOTE: In case window is minimized, music stream is stopped,
    // just make sure to play again on window restore
    if (binocle_audio_is_music_stream_playing(music)) binocle_audio_play_music_stream(music);
  }
}

bool binocle_audio_is_music_stream_playing(binocle_audio_music *music) {
  if (music == NULL) return false;
  else return binocle_audio_is_audio_stream_playing(music->stream);
}

void binocle_audio_set_music_volume(binocle_audio_music *music, float volume) {
  if (music != NULL) binocle_audio_set_audio_stream_volume(music->stream, volume);
}

void binocle_audio_set_music_pitch(binocle_audio_music *music, float pitch) {
  if (music != NULL) binocle_audio_set_audio_buffer_pitch(music->stream.buffer, pitch);
}

void binocle_audio_set_music_pan(binocle_audio_music *music, float pan) {
  if (music != NULL) binocle_audio_set_audio_buffer_pan(music->stream.buffer, pan);
}

float binocle_audio_get_music_time_length(binocle_audio_music *music) {
  if (music == NULL) {
    binocle_log_warning("binocle_audio_get_music_time_length(): music is undefined");
    return 0.0f;
  }
  float totalSeconds = 0.0f;

  totalSeconds = (float) music->frame_count / music->stream.sample_rate;

  return totalSeconds;
}

float binocle_audio_get_music_time_played(binocle_audio_music *music) {
  if (music == NULL) {
    binocle_log_warning("binocle_audio_get_music_time_played(): music is undefined");
    return 0.0f;
  }

  if (music->stream.buffer == NULL) {
    binocle_log_warning("binocle_audio_get_music_time_played(): buffer is undefined");
    return 0.0f;
  }

  float secondsPlayed = 0.0f;

  if (music->ctx_type == BINOCLE_AUDIO_MUSIC_MODULE_XM)
  {
    uint64_t framesPlayed = 0;

    jar_xm_get_position(music->ctx_xm, NULL, NULL, NULL, &framesPlayed);
    secondsPlayed = (float)framesPlayed/music->stream.sample_rate;
  }
  else
  {
    unsigned int framesPlayed = music->stream.buffer->frames_processed;
    secondsPlayed = (float)framesPlayed/music->stream.sample_rate;
  }


  return secondsPlayed;
}

binocle_audio_stream
binocle_audio_load_audio_stream(binocle_audio *audio, unsigned int sample_rate, unsigned int sample_size,
                                unsigned int channels) {
  binocle_audio_stream stream = {0};

  stream.sample_rate = sample_rate;
  stream.sample_size = sample_size;
  stream.channels = channels;

  ma_format formatIn = ((stream.sample_size == 8)? ma_format_u8 : ((stream.sample_size == 16)? ma_format_s16 : ma_format_f32));

  // The size of a streaming buffer must be at least double the size of a period.
  unsigned int periodSize = audio->device.playback.internalPeriodSizeInFrames;

  // If the buffer is not set, compute one that would give us a buffer good enough for a decent frame rate
  unsigned int subBufferSize = (audio->default_size == 0)? audio->device.sampleRate/30 : audio->default_size;
  if (subBufferSize < periodSize) subBufferSize = periodSize;

  // Create a double audio buffer of defined size
  stream.buffer = binocle_audio_load_audio_buffer(audio,
                                                  formatIn,
                                                  stream.channels,
                                                  stream.sample_rate,
                                                  subBufferSize * 2,
                                                  BINOCLE_AUDIO_BUFFER_USAGE_STREAM);
  if (stream.buffer == NULL) {
    binocle_log_error("binocle_audio_load_audio_stream() : Failed to load audio buffer, stream could not be created");
    return stream;
  }

  stream.buffer->looping = true;        // Always loop for streaming buffers.

  binocle_log_info("Audio stream loaded successfully (%i Hz, %i bit, %s)",
                   stream.sample_rate, stream.sample_size, (stream.channels == 1) ? "Mono" : "Stereo");

  return stream;
}

void binocle_audio_unload_audio_stream(binocle_audio *audio, binocle_audio_stream stream) {
  binocle_audio_unload_audio_buffer(audio, stream.buffer);

  binocle_log_info("Unloaded audio stream data");
}

// Update audio stream buffers with data
// NOTE 1: Only updates one buffer of the stream source: dequeue -> update -> queue
// NOTE 2: To dequeue a buffer it needs to be processed: binocle_audio_is_audio_stream_processed()
void binocle_audio_update_audio_stream(binocle_audio_stream stream, const void *data, int frame_count) {
  if (stream.buffer == NULL) {
    binocle_log_error("binocle_audio_update_audio_stream() : No audio buffer");
    return;
  }

  if (stream.buffer->is_sub_buffer_processed[0] || stream.buffer->is_sub_buffer_processed[1]) {
    ma_uint32 subBufferToUpdate = 0;

    if (stream.buffer->is_sub_buffer_processed[0] && stream.buffer->is_sub_buffer_processed[1]) {
      // Both buffers are available for updating. Update the first one and make sure the cursor is moved back to the front.
      subBufferToUpdate = 0;
      stream.buffer->frame_cursor_pos = 0;
    } else {
      // Just update whichever sub-buffer is processed.
      subBufferToUpdate = (stream.buffer->is_sub_buffer_processed[0]) ? 0 : 1;
    }

    ma_uint32 subBufferSizeInFrames = stream.buffer->size_in_frames / 2;
    unsigned char *subBuffer = stream.buffer->data +
                               ((subBufferSizeInFrames * stream.channels * (stream.sample_size / 8)) *
                                subBufferToUpdate);

    stream.buffer->frames_processed += subBufferSizeInFrames;

    // Does this API expect a whole buffer to be updated in one go? Assuming so, but if not will need to change this logic.
    if (subBufferSizeInFrames >= (ma_uint32)frame_count) {
      ma_uint32 framesToWrite = subBufferSizeInFrames;

      if (framesToWrite > (ma_uint32)frame_count) framesToWrite = (ma_uint32)frame_count;

      ma_uint32 bytesToWrite = framesToWrite*stream.channels*(stream.sample_size/8);
      memcpy(subBuffer, data, bytesToWrite);

      // Any leftover frames should be filled with zeros.
      ma_uint32 leftoverFrameCount = subBufferSizeInFrames - framesToWrite;

      if (leftoverFrameCount > 0) memset(subBuffer + bytesToWrite, 0, leftoverFrameCount*stream.channels*(stream.sample_size/8));

      stream.buffer->is_sub_buffer_processed[subBufferToUpdate] = false;
    } else {
      binocle_log_error("binocle_audio_update_audio_stream() : Attempting to write too many frames to buffer");
      return;
    }
  } else {
    binocle_log_error("Audio buffer not available for updating");
    return;
  }
}

bool binocle_audio_is_audio_stream_processed(binocle_audio_stream stream) {
  if (stream.buffer == NULL) {
    binocle_log_error("binocle_audio_is_audio_stream_processed() : No audio buffer");
    return false;
  }

  return stream.buffer->is_sub_buffer_processed[0] || stream.buffer->is_sub_buffer_processed[1];
}

void binocle_audio_play_audio_stream(binocle_audio_stream stream) {
  binocle_audio_play_audio_buffer(stream.buffer);
}

void binocle_audio_pause_audio_stream(binocle_audio_stream stream) {
  binocle_audio_pause_audio_buffer(stream.buffer);
}

void binocle_audio_resume_audio_stream(binocle_audio_stream stream) {
  binocle_audio_resume_audio_buffer(stream.buffer);
}

bool binocle_audio_is_audio_stream_playing(binocle_audio_stream stream) {
  return binocle_audio_is_audio_buffer_playing(stream.buffer);
}

void binocle_audio_stop_audio_stream(binocle_audio_stream stream) {
  binocle_audio_stop_audio_buffer(stream.buffer);
}

void binocle_audio_set_audio_stream_volume(binocle_audio_stream stream, float volume) {
  binocle_audio_set_audio_buffer_volume(stream.buffer, volume);
}

void binocle_audio_set_audio_stream_pitch(binocle_audio_stream stream, float pitch) {
  binocle_audio_set_audio_buffer_pitch(stream.buffer, pitch);
}

void binocle_audio_set_audio_stream_pan(binocle_audio_stream stream, float pan) {
  binocle_audio_set_audio_buffer_pan(stream.buffer, pan);
}

// Default size for new audio streams
void binocle_audio_set_audio_stream_buffer_size_default(binocle_audio *audio, int size)
{
  audio->default_size = size;
}

uint32_t binocle_audio_convert_time_to_sample(float time_in_seconds, uint32_t sample_rate) {
  return time_in_seconds * sample_rate;
}

uint32_t binocle_audio_convert_beat_to_sample(uint32_t beat, uint32_t bpm, uint32_t sample_rate) {
  return (float)beat * ((60.0f / (float)bpm) * (float)sample_rate);
}

/*

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
 */