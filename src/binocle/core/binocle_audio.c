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

#define MINIAUDIO_IMPLEMENTATION

#include "miniaudio/miniaudio.h"

binocle_audio binocle_audio_new() {
  binocle_audio res = {0};
  res.paused = false;
  //res.active_music = binocle_music_new();
  res.master_volume = 1.0f;
  res.first_audio_buffer = NULL;
  res.last_audio_buffer = NULL;
  return res;
}

bool binocle_audio_init(binocle_audio *audio) {
  ma_context_config context_config = ma_context_config_init();
  ma_result result = ma_context_init(NULL, 0, &context_config, &audio->context);
  if (result != MA_SUCCESS) {
    binocle_log_error("binocle_audio_init(): Failed to initialize audio context");
    return false;
  }

  audio->device_config = ma_device_config_init(ma_device_type_playback);
  audio->device_config.playback.pDeviceID = NULL;
  audio->device_config.playback.format = audio->decoder.outputFormat;
  audio->device_config.playback.channels = audio->decoder.outputChannels;
  audio->device_config.capture.pDeviceID = NULL;
  audio->device_config.capture.format = ma_format_s16;
  audio->device_config.capture.channels = 1;
  audio->device_config.sampleRate = audio->decoder.outputSampleRate;
  audio->device_config.dataCallback = binocle_audio_data_callback;
  audio->device_config.pUserData = audio;

  if (ma_device_init(NULL, &audio->device_config, &audio->device) != MA_SUCCESS) {
    binocle_log_error("binocle_audio_init(): Failed to open playback device.\n");
    ma_decoder_uninit(&audio->decoder);
    return false;
  }

  if (ma_device_start(&audio->device) != MA_SUCCESS) {
    binocle_log_error("binocle_audio_init(): Failed to start playback device.\n");
    ma_device_uninit(&audio->device);
    ma_decoder_uninit(&audio->decoder);
    return false;
  }

  binocle_log_info("Audio device initialized successfully");
  binocle_log_info("Audio backend: miniaudio / %s", ma_get_backend_name(audio->context.backend));
  binocle_log_info("Audio format: %s -> %s", ma_get_format_name(audio->device.playback.format),
                   ma_get_format_name(audio->device.playback.internalFormat));
  binocle_log_info("Audio channels: %d -> %d", audio->device.playback.channels,
                   audio->device.playback.internalChannels);
  binocle_log_info("Audio sample rate: %d -> %d", audio->device.sampleRate, audio->device.playback.internalSampleRate);
  binocle_log_info("Audio buffer size: %d", audio->device.playback.internalBufferSizeInFrames);

  audio->is_audio_initialized = true;

  return true;
}


void binocle_audio_destroy(binocle_audio *audio) {
  if (!audio->is_audio_initialized) {
    binocle_log_warning("Could not close audio device because it is not currently initialized");
    return;
  }
  ma_device_uninit(&audio->device);
  ma_decoder_uninit(&audio->decoder);
  ma_context_uninit(&audio->context);

  binocle_log_info("Audio device closed successfully");
}

void binocle_audio_data_callback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount) {
  // Mix
  (void) pDevice;
  binocle_audio *audio = (binocle_audio *) pDevice->pUserData;

  // Init the output buffer to 0
  memset(pOutput, 0, frameCount * pDevice->playback.channels * ma_get_bytes_per_sample(pDevice->playback.format));

  for (binocle_audio_buffer *audio_buffer = audio->first_audio_buffer;
       audio_buffer != NULL; audio_buffer = audio_buffer->next) {
    // Ignore stopped or paused audio.
    if (!audio_buffer->playing || audio_buffer->paused) continue;

    ma_uint32 frames_read = 0;
    for (;;) {
      if (frames_read > frameCount) {
        binocle_log_debug("Mixed too many frames from audio buffer");
        break;
      }

      if (frames_read == frameCount) break;

      // Just read as much data as we can from the stream.
      ma_uint32 framesToRead = (frameCount - frames_read);
      while (framesToRead > 0) {
        float tempBuffer[1024]; // 512 frames for stereo.

        ma_uint32 framesToReadRightNow = framesToRead;
        if (framesToReadRightNow > sizeof(tempBuffer) / sizeof(tempBuffer[0]) / BINOCLE_AUDIO_CHANNEL_COUNT) {
          framesToReadRightNow = sizeof(tempBuffer) / sizeof(tempBuffer[0]) / BINOCLE_AUDIO_CHANNEL_COUNT;
        }

        ma_uint32 framesJustRead = (ma_uint32) ma_pcm_converter_read(&audio_buffer->dsp, tempBuffer,
                                                                     framesToReadRightNow);
        if (framesJustRead > 0) {
          float *framesOut = (float *) pOutput + (frames_read * audio->device.playback.channels);
          float *framesIn = tempBuffer;
          binocle_audio_mix_audio_frames(audio, framesOut, framesIn, framesJustRead, audio_buffer->volume);

          framesToRead -= framesJustRead;
          frames_read += framesJustRead;
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

}

static ma_uint32 binocle_audio_on_audio_buffer_dsp_read(ma_pcm_converter *pDSP, void *pFramesOut, ma_uint32 frameCount,
                                                        void *pUserData) {
  binocle_audio_buffer *audioBuffer = (binocle_audio_buffer *) pUserData;

  ma_uint32 subBufferSizeInFrames = (audioBuffer->buffer_size_in_frames > 1) ? audioBuffer->buffer_size_in_frames / 2
                                                                             : audioBuffer->buffer_size_in_frames;
  ma_uint32 currentSubBufferIndex = audioBuffer->frame_cursor_pos / subBufferSizeInFrames;

  if (currentSubBufferIndex > 1) {
    binocle_log_debug("Frame cursor position moved too far forward in audio stream");
    return 0;
  }

  // Another thread can update the processed state of buffers so we just take a copy here to try and avoid potential synchronization problems.
  bool isSubBufferProcessed[2];
  isSubBufferProcessed[0] = audioBuffer->is_sub_buffer_processed[0];
  isSubBufferProcessed[1] = audioBuffer->is_sub_buffer_processed[1];

  ma_uint32 frameSizeInBytes = ma_get_bytes_per_sample(audioBuffer->dsp.formatConverterIn.config.formatIn) *
                               audioBuffer->dsp.formatConverterIn.config.channels;

  // Fill out every frame until we find a buffer that's marked as processed. Then fill the remainder with 0.
  ma_uint32 framesRead = 0;
  for (;;) {
    // We break from this loop differently depending on the buffer's usage. For static buffers, we simply fill as much data as we can. For
    // streaming buffers we only fill the halves of the buffer that are processed. Unprocessed halves must keep their audio data in-tact.
    if (audioBuffer->usage == BINOCLE_AUDIO_BUFFER_USAGE_STATIC) {
      if (framesRead >= frameCount) break;
    } else {
      if (isSubBufferProcessed[currentSubBufferIndex]) break;
    }

    ma_uint32 totalFramesRemaining = (frameCount - framesRead);
    if (totalFramesRemaining == 0) break;

    ma_uint32 framesRemainingInOutputBuffer;
    if (audioBuffer->usage == BINOCLE_AUDIO_BUFFER_USAGE_STATIC) {
      framesRemainingInOutputBuffer = audioBuffer->buffer_size_in_frames - audioBuffer->frame_cursor_pos;
    } else {
      ma_uint32 firstFrameIndexOfThisSubBuffer = subBufferSizeInFrames * currentSubBufferIndex;
      framesRemainingInOutputBuffer =
          subBufferSizeInFrames - (audioBuffer->frame_cursor_pos - firstFrameIndexOfThisSubBuffer);
    }

    ma_uint32 framesToRead = totalFramesRemaining;
    if (framesToRead > framesRemainingInOutputBuffer) framesToRead = framesRemainingInOutputBuffer;

    memcpy((unsigned char *) pFramesOut + (framesRead * frameSizeInBytes),
           audioBuffer->buffer + (audioBuffer->frame_cursor_pos * frameSizeInBytes), framesToRead * frameSizeInBytes);
    audioBuffer->frame_cursor_pos = (audioBuffer->frame_cursor_pos + framesToRead) % audioBuffer->buffer_size_in_frames;
    framesRead += framesToRead;

    // If we've read to the end of the buffer, mark it as processed.
    if (framesToRead == framesRemainingInOutputBuffer) {
      audioBuffer->is_sub_buffer_processed[currentSubBufferIndex] = true;
      isSubBufferProcessed[currentSubBufferIndex] = true;

      currentSubBufferIndex = (currentSubBufferIndex + 1) % 2;

      // We need to break from this loop if we're not looping.
      if (!audioBuffer->looping) {
        binocle_audio_stop_audio_buffer(audioBuffer);
        break;
      }
    }
  }

  // Zero-fill excess.
  ma_uint32 totalFramesRemaining = (frameCount - framesRead);
  if (totalFramesRemaining > 0) {
    memset((unsigned char *) pFramesOut + (framesRead * frameSizeInBytes), 0, totalFramesRemaining * frameSizeInBytes);

    // For static buffers we can fill the remaining frames with silence for safety, but we don't want
    // to report those frames as "read". The reason for this is that the caller uses the return value
    // to know whether or not a non-looping sound has finished playback.
    if (audioBuffer->usage != BINOCLE_AUDIO_BUFFER_USAGE_STATIC) framesRead += totalFramesRemaining;
  }

  return framesRead;
}

static void
binocle_audio_mix_audio_frames(void *pUserData, float *framesOut, const float *framesIn, ma_uint32 frameCount,
                               float localVolume) {
  binocle_audio *audio = (binocle_audio *) pUserData;
  ma_device *pDevice = &audio->device;

  for (ma_uint32 iFrame = 0; iFrame < frameCount; ++iFrame) {
    for (ma_uint32 iChannel = 0; iChannel < pDevice->playback.channels; ++iChannel) {
      float *frameOut = framesOut + (iFrame * pDevice->playback.channels);
      const float *frameIn = framesIn + (iFrame * pDevice->playback.channels);

      frameOut[iChannel] += (frameIn[iChannel] * audio->master_volume * localVolume);
    }
  }
}

bool binocle_audio_is_audio_device_ready(binocle_audio *audio) {
  return audio->is_audio_initialized;
}

void binocle_audio_set_master_volume(binocle_audio *audio, float volume) {
  if (volume < 0.0f) volume = 0.0f;
  else if (volume > 1.0f) volume = 1.0f;

  audio->master_volume = volume;
}

binocle_audio_buffer *
binocle_audio_create_audio_buffer(binocle_audio *audio, ma_format format, ma_uint32 channels, ma_uint32 sampleRate,
                                  ma_uint32 bufferSizeInFrames, binocle_audio_buffer_usage usage) {
  binocle_audio_buffer *audioBuffer = (binocle_audio_buffer *) calloc(
      sizeof(*audioBuffer) + (bufferSizeInFrames * channels * ma_get_bytes_per_sample(format)), 1);
  if (audioBuffer == NULL) {
    binocle_log_error("binocle_audio_create_audio_buffer() : Failed to allocate memory for audio buffer");
    return NULL;
  }

  // We run audio data through a format converter.
  ma_pcm_converter_config dspConfig;
  memset(&dspConfig, 0, sizeof(dspConfig));
  dspConfig.formatIn = format;
  dspConfig.formatOut = BINOCLE_AUDIO_SAMPLE_FORMAT;
  dspConfig.channelsIn = channels;
  dspConfig.channelsOut = BINOCLE_AUDIO_CHANNEL_COUNT;
  dspConfig.sampleRateIn = sampleRate;
  dspConfig.sampleRateOut = BINOCLE_AUDIO_SAMPLE_RATE;
  dspConfig.onRead = binocle_audio_on_audio_buffer_dsp_read;
  dspConfig.pUserData = audioBuffer;
  dspConfig.allowDynamicSampleRate = MA_TRUE;    // <-- Required for pitch shifting.
  ma_result result = ma_pcm_converter_init(&dspConfig, &audioBuffer->dsp);

  if (result != MA_SUCCESS) {
    binocle_log_error("binocle_audio_create_audio_buffer() : Failed to create data conversion pipeline");
    free(audioBuffer);
    return NULL;
  }

  audioBuffer->volume = 1.0f;
  audioBuffer->pitch = 1.0f;
  audioBuffer->playing = false;
  audioBuffer->paused = false;
  audioBuffer->looping = false;
  audioBuffer->usage = usage;
  audioBuffer->buffer_size_in_frames = bufferSizeInFrames;
  audioBuffer->frame_cursor_pos = 0;

  // Buffers should be marked as processed by default so that a call to UpdateAudioStream() immediately after initialization works correctly.
  audioBuffer->is_sub_buffer_processed[0] = true;
  audioBuffer->is_sub_buffer_processed[1] = true;

  binocle_audio_track_audio_buffer(audio, audioBuffer);

  return audioBuffer;
}

void binocle_audio_delete_audio_buffer(binocle_audio *audio, binocle_audio_buffer *audioBuffer) {
  if (audioBuffer == NULL) {
    binocle_log_error("binocle_audio_delete_audio_buffer() : No audio buffer");
    return;
  }

  binocle_audio_untrack_audio_buffer(audio, audioBuffer);
  free(audioBuffer);
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

  float pitchMul = pitch / audio_buffer->pitch;

  // Pitching is just an adjustment of the sample rate. Note that this changes the duration of the sound - higher pitches
  // will make the sound faster; lower pitches make it slower.
  ma_uint32 newOutputSampleRate = (ma_uint32)((float) audio_buffer->dsp.src.config.sampleRateOut / pitchMul);
  audio_buffer->pitch *= (float) audio_buffer->dsp.src.config.sampleRateOut / newOutputSampleRate;

  ma_pcm_converter_set_output_sample_rate(&audio_buffer->dsp, newOutputSampleRate);
}

void binocle_audio_track_audio_buffer(binocle_audio *audio, binocle_audio_buffer *audio_buffer) {
  if (audio->first_audio_buffer == NULL) audio->first_audio_buffer = audio_buffer;
  else {
    audio->last_audio_buffer->next = audio_buffer;
    audio_buffer->prev = audio->last_audio_buffer;
  }

  audio->last_audio_buffer = audio_buffer;
}

void binocle_audio_untrack_audio_buffer(binocle_audio *audio, binocle_audio_buffer *audio_buffer) {
  if (audio_buffer->prev == NULL) audio->first_audio_buffer = audio_buffer->next;
  else audio_buffer->prev->next = audio_buffer->next;

  if (audio_buffer->next == NULL) audio->last_audio_buffer = audio_buffer->prev;
  else audio_buffer->next->prev = audio_buffer->prev;

  audio_buffer->prev = NULL;
  audio_buffer->next = NULL;
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

    wave.data = (short *) malloc(wave.sample_count * wave.channels * sizeof(short));

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
  wave.data = drflac_open_file_and_read_pcm_frames_s16(file_name, &wave.channels, &wave.sample_rate, &totalSampleCount);

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
  wave.data = drmp3_open_file_and_read_f32(file_name, &config, &totalFrameCount);

  wave.channels = config.outputChannels;
  wave.sample_rate = config.outputSampleRate;
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
  wave.data = drwav_open_file_and_read_pcm_frames_f32(file_name, &wave.channels, &wave.sample_rate, &totalFrameCount);
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

    ma_uint32 frameCount = (ma_uint32) ma_convert_frames(NULL, BINOCLE_AUDIO_SAMPLE_FORMAT, BINOCLE_AUDIO_CHANNEL_COUNT,
                                                         BINOCLE_AUDIO_SAMPLE_RATE, NULL, formatIn, wave.channels,
                                                         wave.sample_rate, frameCountIn);
    if (frameCount == 0) binocle_log_warning("binocle_audio_load_sound_from_wave() : Failed to get frame count for format conversion");

    binocle_audio_buffer *audioBuffer = binocle_audio_create_audio_buffer(audio, BINOCLE_AUDIO_SAMPLE_FORMAT,
                                                                          BINOCLE_AUDIO_CHANNEL_COUNT,
                                                                          BINOCLE_AUDIO_SAMPLE_RATE, frameCount,
                                                                          BINOCLE_AUDIO_BUFFER_USAGE_STATIC);
    if (audioBuffer == NULL) binocle_log_warning("binocle_audio_load_sound_from_wave() : Failed to create audio buffer");

    if (audioBuffer != NULL) {
      frameCount = (ma_uint32) ma_convert_frames(audioBuffer->buffer,
                                                 audioBuffer->dsp.formatConverterIn.config.formatIn,
                                                 audioBuffer->dsp.formatConverterIn.config.channels,
                                                 audioBuffer->dsp.src.config.sampleRateIn, wave.data, formatIn,
                                                 wave.channels, wave.sample_rate, frameCountIn);
      if (frameCount == 0) binocle_log_warning("binocle_audio_load_sound_from_wave() : Format conversion failed");
    }

    sound.audio_buffer = audioBuffer;
  }

  return sound;
}

void binocle_audio_unload_wave(binocle_audio_wave wave) {
  if (wave.data != NULL) free(wave.data);

  binocle_log_info("Unloaded wave data from RAM");
}

void binocle_audio_unload_sound(binocle_audio *audio, binocle_audio_sound sound) {
  binocle_audio_delete_audio_buffer(audio, (binocle_audio_buffer *) sound.audio_buffer);

  binocle_log_info("[SND ID %i][BUFR ID %i] Unloaded sound data from RAM", sound.source, sound.buffer);
}

void binocle_audio_update_sound(binocle_audio_sound sound, const void *data, int samplesCount) {
  binocle_audio_buffer *audioBuffer = (binocle_audio_buffer *) sound.audio_buffer;

  if (audioBuffer == NULL) {
    binocle_log_error("binocle_audio_update_sound() : Invalid sound - no audio buffer");
    return;
  }

  binocle_audio_stop_audio_buffer(audioBuffer);

  // TODO: May want to lock/unlock this since this data buffer is read at mixing time.
  memcpy(audioBuffer->buffer, data, samplesCount * audioBuffer->dsp.formatConverterIn.config.channels *
                                    ma_get_bytes_per_sample(audioBuffer->dsp.formatConverterIn.config.formatIn));
}

void binocle_audio_play_sound(binocle_audio_sound sound) {
  binocle_audio_play_audio_buffer((binocle_audio_buffer *) sound.audio_buffer);
}

void binocle_audio_pause_sound(binocle_audio_sound sound) {
  binocle_audio_pause_audio_buffer((binocle_audio_buffer *) sound.audio_buffer);
}

void binocle_audio_resume_sound(binocle_audio_sound sound) {
  binocle_audio_resume_audio_buffer((binocle_audio_buffer *) sound.audio_buffer);
}

void binocle_audio_stop_sound(binocle_audio_sound sound) {
  binocle_audio_stop_audio_buffer((binocle_audio_buffer *) sound.audio_buffer);
}

bool binocle_audio_is_sound_playing(binocle_audio_sound sound) {
  return binocle_audio_is_audio_buffer_playing((binocle_audio_buffer *) sound.audio_buffer);
}

void binocle_audio_set_sound_volume(binocle_audio_sound sound, float volume) {
  binocle_audio_set_audio_buffer_volume((binocle_audio_buffer *) sound.audio_buffer, volume);
}

void binocle_audio_set_sound_pitch(binocle_audio_sound sound, float pitch) {
  binocle_audio_set_audio_buffer_pitch((binocle_audio_buffer *) sound.audio_buffer, pitch);
}

//
// Music stuff
//

binocle_audio_music *binocle_audio_load_music_stream(binocle_audio *audio, const char *fileName) {
  binocle_audio_music *music = (binocle_audio_music *) malloc(sizeof(binocle_audio_music));
  bool musicLoaded = true;

  if (binocle_audio_is_file_extension(fileName, ".ogg")) {
    // Open ogg audio stream
    music->ctx_ogg = stb_vorbis_open_filename(fileName, NULL, NULL);

    if (music->ctx_ogg == NULL) musicLoaded = false;
    else {
      stb_vorbis_info info = stb_vorbis_get_info(music->ctx_ogg);  // Get Ogg file info

      // OGG bit rate defaults to 16 bit, it's enough for compressed format
      music->stream = binocle_audio_init_audio_stream(audio, info.sample_rate, 16, info.channels);
      music->total_samples = (unsigned int) stb_vorbis_stream_length_in_samples(music->ctx_ogg) * info.channels;
      music->samples_left = music->total_samples;
      music->ctx_type = BINOCLE_AUDIO_MUSIC_AUDIO_OGG;
      music->loop_count = -1;                       // Infinite loop by default

      binocle_log_debug("[%s] OGG total samples: %i", fileName, music->total_samples);
      binocle_log_debug("[%s] OGG sample rate: %i", fileName, info.sample_rate);
      binocle_log_debug("[%s] OGG channels: %i", fileName, info.channels);
      binocle_log_debug("[%s] OGG memory required: %i", fileName, info.temp_memory_required);
    }
  } else if (binocle_audio_is_file_extension(fileName, ".flac")) {
    music->ctx_flac = drflac_open_file(fileName);

    if (music->ctx_flac == NULL) musicLoaded = false;
    else {
      music->stream = binocle_audio_init_audio_stream(audio, music->ctx_flac->sampleRate,
                                                      music->ctx_flac->bitsPerSample, music->ctx_flac->channels);
      music->total_samples = (unsigned int) music->ctx_flac->totalSampleCount;
      music->samples_left = music->total_samples;
      music->ctx_type = BINOCLE_AUDIO_MUSIC_AUDIO_FLAC;
      music->loop_count = -1;                       // Infinite loop by default

      binocle_log_debug("[%s] FLAC total samples: %i", fileName, music->total_samples);
      binocle_log_debug("[%s] FLAC sample rate: %i", fileName, music->ctx_flac->sampleRate);
      binocle_log_debug("[%s] FLAC bits per sample: %i", fileName, music->ctx_flac->bitsPerSample);
      binocle_log_debug("[%s] FLAC channels: %i", fileName, music->ctx_flac->channels);
    }
  } else if (binocle_audio_is_file_extension(fileName, ".mp3")) {
    int result = drmp3_init_file(&music->ctx_mp3, fileName, NULL);

    if (!result) musicLoaded = false;
    else {
      binocle_log_debug("[%s] MP3 sample rate: %i", fileName, music->ctx_mp3.sampleRate);
      binocle_log_debug("[%s] MP3 bits per sample: %i", fileName, 32);
      binocle_log_debug("[%s] MP3 channels: %i", fileName, music->ctx_mp3.channels);

      music->stream = binocle_audio_init_audio_stream(audio, music->ctx_mp3.sampleRate, 32, music->ctx_mp3.channels);

      // TODO: There is not an easy way to compute the total number of samples available
      // in an MP3, frames size could be variable... we tried with a 60 seconds music... but crashes...
      music->total_samples = drmp3_get_pcm_frame_count(&music->ctx_mp3) * music->ctx_mp3.channels;
      music->samples_left = music->total_samples;
      music->ctx_type = BINOCLE_AUDIO_MUSIC_AUDIO_MP3;
      music->loop_count = -1;                       // Infinite loop by default

      binocle_log_debug("[%s] MP3 total samples: %i", fileName, music->total_samples);
    }
  } else if (binocle_audio_is_file_extension(fileName, ".xm")) {
    int result = jar_xm_create_context_from_file(&music->ctx_xm, 48000, fileName);

    if (!result)    // XM context created successfully
    {
      jar_xm_set_max_loop_count(music->ctx_xm, 0); // Set infinite number of loops

      // NOTE: Only stereo is supported for XM
      music->stream = binocle_audio_init_audio_stream(audio, 48000, 16, 2);
      music->total_samples = (unsigned int) jar_xm_get_remaining_samples(music->ctx_xm);
      music->samples_left = music->total_samples;
      music->ctx_type = BINOCLE_AUDIO_MUSIC_MODULE_XM;
      music->loop_count = -1;                       // Infinite loop by default

      binocle_log_debug("[%s] XM number of samples: %i", fileName, music->total_samples);
      binocle_log_debug("[%s] XM track length: %11.6f sec", fileName, (float) music->total_samples / 48000.0f);
    } else musicLoaded = false;
  } else if (binocle_audio_is_file_extension(fileName, ".mod")) {
    jar_mod_init(&music->ctx_mod);

    if (jar_mod_load_file(&music->ctx_mod, fileName)) {
      // NOTE: Only stereo is supported for MOD
      music->stream = binocle_audio_init_audio_stream(audio, 48000, 16, 2);
      music->total_samples = (unsigned int) jar_mod_max_samples(&music->ctx_mod);
      music->samples_left = music->total_samples;
      music->ctx_type = BINOCLE_AUDIO_MUSIC_MODULE_MOD;
      music->loop_count = -1;                       // Infinite loop by default

      binocle_log_debug("[%s] MOD number of samples: %i", fileName, music->samples_left);
      binocle_log_debug("[%s] MOD track length: %11.6f sec", fileName, (float) music->total_samples / 48000.0f);
    } else musicLoaded = false;
  } else musicLoaded = false;

  if (!musicLoaded) {
    if (music->ctx_type == BINOCLE_AUDIO_MUSIC_AUDIO_OGG) stb_vorbis_close(music->ctx_ogg);
    else if (music->ctx_type == BINOCLE_AUDIO_MUSIC_AUDIO_FLAC) drflac_free(music->ctx_flac);
    else if (music->ctx_type == BINOCLE_AUDIO_MUSIC_AUDIO_MP3) drmp3_uninit(&music->ctx_mp3);
    else if (music->ctx_type == BINOCLE_AUDIO_MUSIC_MODULE_XM) jar_xm_free_context(music->ctx_xm);
    else if (music->ctx_type == BINOCLE_AUDIO_MUSIC_MODULE_MOD) jar_mod_unload(&music->ctx_mod);

    free(music);
    music = NULL;

    binocle_log_warning("[%s] Music file could not be opened", fileName);
  }

  return music;
}

void binocle_audio_unload_music_stream(binocle_audio *audio, binocle_audio_music *music) {
  if (music == NULL) return;

  binocle_audio_close_audio_stream(audio, music->stream);

  if (music->ctx_type == BINOCLE_AUDIO_MUSIC_AUDIO_OGG) stb_vorbis_close(music->ctx_ogg);
  else if (music->ctx_type == BINOCLE_AUDIO_MUSIC_AUDIO_FLAC) drflac_free(music->ctx_flac);
  else if (music->ctx_type == BINOCLE_AUDIO_MUSIC_AUDIO_MP3) drmp3_uninit(&music->ctx_mp3);
  else if (music->ctx_type == BINOCLE_AUDIO_MUSIC_MODULE_XM) jar_xm_free_context(music->ctx_xm);
  else if (music->ctx_type == BINOCLE_AUDIO_MUSIC_MODULE_MOD) jar_mod_unload(&music->ctx_mod);

  free(music);
}

void binocle_audio_play_music_stream(binocle_audio_music *music) {
  if (music != NULL) {
    binocle_audio_buffer *audioBuffer = (binocle_audio_buffer *) music->stream.audio_buffer;

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
}

void binocle_audio_pause_music_stream(binocle_audio_music *music) {
  if (music != NULL) binocle_audio_pause_audio_stream(music->stream);
}

void binocle_audio_resume_music_stream(binocle_audio_music *music) {
  if (music != NULL) binocle_audio_resume_audio_stream(music->stream);
}

void binocle_audio_stop_music_stream(binocle_audio_music *music) {
  if (music == NULL) return;

  binocle_audio_stop_audio_stream(music->stream);

  // Restart music context
  switch (music->ctx_type) {
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

  music->samples_left = music->total_samples;
}

void binocle_audio_update_music_stream(binocle_audio_music *music) {
  if (music == NULL) return;

  bool streamEnding = false;

  unsigned int subBufferSizeInFrames = ((binocle_audio_buffer *) music->stream.audio_buffer)->buffer_size_in_frames / 2;

  // NOTE: Using dynamic allocation because it could require more than 16KB
  void *pcm = calloc(subBufferSizeInFrames * music->stream.channels * music->stream.sample_size / 8, 1);

  int samplesCount = 0;    // Total size of data steamed in L+R samples for xm floats, individual L or R for ogg shorts

  while (binocle_audio_is_audio_buffer_processed(music->stream)) {
    if ((music->samples_left / music->stream.channels) >= subBufferSizeInFrames)
      samplesCount = subBufferSizeInFrames * music->stream.channels;
    else samplesCount = music->samples_left;

    switch (music->ctx_type) {
      case BINOCLE_AUDIO_MUSIC_AUDIO_OGG: {
        // NOTE: Returns the number of samples to process (be careful! we ask for number of shorts!)
        stb_vorbis_get_samples_short_interleaved(music->ctx_ogg, music->stream.channels, (short *) pcm, samplesCount);

      }
        break;
      case BINOCLE_AUDIO_MUSIC_AUDIO_FLAC: {
        // NOTE: Returns the number of samples to process
        unsigned int numSamplesFlac = (unsigned int) drflac_read_pcm_frames_s16(music->ctx_flac, samplesCount,
                                                                                (short *) pcm);

      }
        break;
      case BINOCLE_AUDIO_MUSIC_AUDIO_MP3: {
        // NOTE: samplesCount, actually refers to framesCount and returns the number of frames processed
        drmp3_read_pcm_frames_f32(&music->ctx_mp3, samplesCount / music->stream.channels, (float *) pcm);

      }
        break;
      case BINOCLE_AUDIO_MUSIC_MODULE_XM: {
        // NOTE: Internally this function considers 2 channels generation, so samplesCount/2
        jar_xm_generate_samples_16bit(music->ctx_xm, (short *) pcm, samplesCount / 2);
      }
        break;
      case BINOCLE_AUDIO_MUSIC_MODULE_MOD: {
        // NOTE: 3rd parameter (nbsample) specify the number of stereo 16bits samples you want, so sampleCount/2
        jar_mod_fillbuffer(&music->ctx_mod, (short *) pcm, samplesCount / 2, 0);
      }
        break;
      default:
        break;
    }


    binocle_audio_update_audio_stream(music->stream, pcm, samplesCount);
    if ((music->ctx_type == BINOCLE_AUDIO_MUSIC_MODULE_XM) || (music->ctx_type == BINOCLE_AUDIO_MUSIC_MODULE_MOD)) {
      if (samplesCount > 1) music->samples_left -= samplesCount / 2;
      else music->samples_left -= samplesCount;
    } else music->samples_left -= samplesCount;

    if (music->samples_left <= 0) {
      streamEnding = true;
      break;
    }
  }

  // Free allocated pcm data
  free(pcm);

  // Reset audio stream for looping
  if (streamEnding) {
    binocle_audio_stop_music_stream(music);        // Stop music (and reset)

    // Decrease loopCount to stop when required
    if (music->loop_count > 0) {
      music->loop_count--;        // Decrease loop count
      binocle_audio_play_music_stream(music);    // Play again
    } else {
      if (music->loop_count == -1) binocle_audio_play_music_stream(music);
    }
  } else {
    // NOTE: In case window is minimized, music stream is stopped,
    // just make sure to play again on window restore
    if (binocle_audio_is_music_playing(music)) binocle_audio_play_music_stream(music);
  }
}

bool binocle_audio_is_music_playing(binocle_audio_music *music) {
  if (music == NULL) return false;
  else return binocle_audio_is_audio_stream_playing(music->stream);
}

void binocle_audio_set_music_volume(binocle_audio_music *music, float volume) {
  if (music != NULL) binocle_audio_set_audio_stream_volume(music->stream, volume);
}

void binocle_audio_set_music_pitch(binocle_audio_music *music, float pitch) {
  if (music != NULL) binocle_audio_set_audio_stream_pitch(music->stream, pitch);
}

void binocle_audio_set_music_loop_count(binocle_audio_music *music, int count) {
  // -1 = infinite loop
  if (music != NULL) music->loop_count = count;
}

float binocle_audio_get_music_time_length(binocle_audio_music *music) {
  float totalSeconds = 0.0f;

  if (music != NULL) totalSeconds = (float) music->total_samples / (music->stream.sample_rate * music->stream.channels);

  return totalSeconds;
}

float binocle_audio_get_music_time_played(binocle_audio_music *music) {
  float secondsPlayed = 0.0f;

  if (music != NULL) {
    unsigned int samplesPlayed = music->total_samples - music->samples_left;
    secondsPlayed = (float) samplesPlayed / (music->stream.sample_rate * music->stream.channels);
  }

  return secondsPlayed;
}

binocle_audio_stream
binocle_audio_init_audio_stream(binocle_audio *audio, unsigned int sampleRate, unsigned int sampleSize,
                                unsigned int channels) {
  binocle_audio_stream stream = {0};

  stream.sample_rate = sampleRate;
  stream.sample_size = sampleSize;

  // Only mono and stereo channels are supported, more channels require AL_EXT_MCFORMATS extension
  if ((channels > 0) && (channels < 3)) stream.channels = channels;
  else {
    binocle_log_warning("Init audio stream: Number of channels not supported: %i", channels);
    stream.channels = 1;  // Fallback to mono channel
  }

  ma_format formatIn = ((stream.sample_size == 8) ? ma_format_u8 : ((stream.sample_size == 16) ? ma_format_s16
                                                                                               : ma_format_f32));

  // The size of a streaming buffer must be at least double the size of a period.
  unsigned int periodSize = audio->device.playback.internalBufferSizeInFrames / audio->device.playback.internalPeriods;
  unsigned int subBufferSize = BINOCLE_AUDIO_BUFFER_SIZE;
  if (subBufferSize < periodSize) subBufferSize = periodSize;

  binocle_audio_buffer *audioBuffer = binocle_audio_create_audio_buffer(audio, formatIn, stream.channels,
                                                                        stream.sample_rate, subBufferSize * 2,
                                                                        BINOCLE_AUDIO_BUFFER_USAGE_STREAM);
  if (audioBuffer == NULL) {
    binocle_log_error("binocle_audio_init_audio_stream() : Failed to create audio buffer");
    return stream;
  }

  audioBuffer->looping = true;        // Always loop for streaming buffers.
  stream.audio_buffer = audioBuffer;

  binocle_log_info("[AUD ID %i] Audio stream loaded successfully (%i Hz, %i bit, %s)", stream.source,
                   stream.sample_rate, stream.sample_size, (stream.channels == 1) ? "Mono" : "Stereo");

  return stream;
}

void binocle_audio_close_audio_stream(binocle_audio *audio, binocle_audio_stream stream) {
  binocle_audio_delete_audio_buffer(audio, (binocle_audio_buffer *) stream.audio_buffer);

  binocle_log_info("[AUD ID %i] Unloaded audio stream data", stream.source);
}

// Update audio stream buffers with data
// NOTE 1: Only updates one buffer of the stream source: unqueue -> update -> queue
// NOTE 2: To unqueue a buffer it needs to be processed: IsAudioBufferProcessed()
void binocle_audio_update_audio_stream(binocle_audio_stream stream, const void *data, int samplesCount) {
  binocle_audio_buffer *audioBuffer = (binocle_audio_buffer *) stream.audio_buffer;
  if (audioBuffer == NULL) {
    binocle_log_error("binocle_audio_update_audio_stream() : No audio buffer");
    return;
  }

  if (audioBuffer->is_sub_buffer_processed[0] || audioBuffer->is_sub_buffer_processed[1]) {
    ma_uint32 subBufferToUpdate;

    if (audioBuffer->is_sub_buffer_processed[0] && audioBuffer->is_sub_buffer_processed[1]) {
      // Both buffers are available for updating. Update the first one and make sure the cursor is moved back to the front.
      subBufferToUpdate = 0;
      audioBuffer->frame_cursor_pos = 0;
    } else {
      // Just update whichever sub-buffer is processed.
      subBufferToUpdate = (audioBuffer->is_sub_buffer_processed[0]) ? 0 : 1;
    }

    ma_uint32 subBufferSizeInFrames = audioBuffer->buffer_size_in_frames / 2;
    unsigned char *subBuffer = audioBuffer->buffer +
                               ((subBufferSizeInFrames * stream.channels * (stream.sample_size / 8)) *
                                subBufferToUpdate);

    // Does this API expect a whole buffer to be updated in one go? Assuming so, but if not will need to change this logic.
    if (subBufferSizeInFrames >= (ma_uint32) samplesCount / stream.channels) {
      ma_uint32 framesToWrite = subBufferSizeInFrames;

      if (framesToWrite > ((ma_uint32) samplesCount / stream.channels))
        framesToWrite = (ma_uint32) samplesCount / stream.channels;

      ma_uint32 bytesToWrite = framesToWrite * stream.channels * (stream.sample_size / 8);
      memcpy(subBuffer, data, bytesToWrite);

      // Any leftover frames should be filled with zeros.
      ma_uint32 leftoverFrameCount = subBufferSizeInFrames - framesToWrite;

      if (leftoverFrameCount > 0) {
        memset(subBuffer + bytesToWrite, 0, leftoverFrameCount * stream.channels * (stream.sample_size / 8));
      }

      audioBuffer->is_sub_buffer_processed[subBufferToUpdate] = false;
    } else {
      binocle_log_error("binocle_audio_update_audio_stream() : Attempting to write too many frames to buffer");
      return;
    }
  } else {
    binocle_log_error("Audio buffer not available for updating");
    return;
  }
}

bool binocle_audio_is_audio_buffer_processed(binocle_audio_stream stream) {
  binocle_audio_buffer *audioBuffer = (binocle_audio_buffer *) stream.audio_buffer;
  if (audioBuffer == NULL) {
    binocle_log_error("binocle_audio_is_audio_buffer_processed() : No audio buffer");
    return false;
  }

  return audioBuffer->is_sub_buffer_processed[0] || audioBuffer->is_sub_buffer_processed[1];
}

void binocle_audio_play_audio_stream(binocle_audio_stream stream) {
  binocle_audio_play_audio_buffer((binocle_audio_buffer *) stream.audio_buffer);
}

void binocle_audio_pause_audio_stream(binocle_audio_stream stream) {
  binocle_audio_pause_audio_buffer((binocle_audio_buffer *) stream.audio_buffer);
}

void binocle_audio_resume_audio_stream(binocle_audio_stream stream) {
  binocle_audio_resume_audio_buffer((binocle_audio_buffer *) stream.audio_buffer);
}

bool binocle_audio_is_audio_stream_playing(binocle_audio_stream stream) {
  return binocle_audio_is_audio_buffer_playing((binocle_audio_buffer *) stream.audio_buffer);
}

void binocle_audio_stop_audio_stream(binocle_audio_stream stream) {
  binocle_audio_stop_audio_buffer((binocle_audio_buffer *) stream.audio_buffer);
}

void binocle_audio_set_audio_stream_volume(binocle_audio_stream stream, float volume) {
  binocle_audio_set_audio_buffer_volume((binocle_audio_buffer *) stream.audio_buffer, volume);
}

void binocle_audio_set_audio_stream_pitch(binocle_audio_stream stream, float pitch) {
  binocle_audio_set_audio_buffer_pitch((binocle_audio_buffer *) stream.audio_buffer, pitch);
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