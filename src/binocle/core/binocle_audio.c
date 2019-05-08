//
// Created by Valerio Santinelli on 09/11/2018.
//

#include <miniaudio/miniaudio.h>
#include "binocle_audio.h"
#include "binocle_log.h"

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

static binocle_audio g_audio;

binocle_audio binocle_audio_new() {
  binocle_audio res = { 0 };
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
  if (result != MA_SUCCESS)
  {
    binocle_log_error("Failed to initialize audio context");
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
  audio->device_config.pUserData = &audio->decoder;

  if (ma_device_init(NULL, &audio->device_config, &audio->device) != MA_SUCCESS) {
    binocle_log_error("Failed to open playback device.\n");
    ma_decoder_uninit(&audio->decoder);
    return false;
  }

  if (ma_device_start(&audio->device) != MA_SUCCESS) {
    binocle_log_error("Failed to start playback device.\n");
    ma_device_uninit(&audio->device);
    ma_decoder_uninit(&audio->decoder);
    return false;
  }

  binocle_log_info("Audio device initialized successfully");
  binocle_log_info("Audio backend: miniaudio / %s", ma_get_backend_name(audio->context.backend));
  binocle_log_info("Audio format: %s -> %s", ma_get_format_name(audio->device.playback.format), ma_get_format_name(audio->device.playback.internalFormat));
  binocle_log_info("Audio channels: %d -> %d", audio->device.playback.channels, audio->device.playback.internalChannels);
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

void binocle_audio_init_audio_system() {
  g_audio = binocle_audio_new();
  binocle_audio_init(&g_audio);
}

void binocle_audio_close_audio_system() {
  binocle_audio_destroy(&g_audio);
}

void binocle_audio_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
  // Mix
  (void)pDevice;

  // Init the output buffer to 0
  memset(pOutput, 0, frameCount * pDevice->playback.channels * ma_get_bytes_per_sample(pDevice->playback.format));

  for (binocle_audio_buffer *audio_buffer = g_audio.first_audio_buffer; audio_buffer != NULL ; audio_buffer = audio_buffer->next) {
    // Ignore stopped or paused audio.
    if (!audio_buffer->playing || audio_buffer->paused) continue;

    ma_uint32 frames_read = 0;
    for (;;)
    {
      if (frames_read > frameCount)
      {
        binocle_log_debug("Mixed too many frames from audio buffer");
        break;
      }

      if (frames_read == frameCount) break;

      // Just read as much data as we can from the stream.
      ma_uint32 framesToRead = (frameCount - frames_read);
      while (framesToRead > 0)
      {
        float tempBuffer[1024]; // 512 frames for stereo.

        ma_uint32 framesToReadRightNow = framesToRead;
        if (framesToReadRightNow > sizeof(tempBuffer)/sizeof(tempBuffer[0])/BINOCLE_AUDIO_CHANNEL_COUNT)
        {
          framesToReadRightNow = sizeof(tempBuffer)/sizeof(tempBuffer[0])/BINOCLE_AUDIO_CHANNEL_COUNT;
        }

        ma_uint32 framesJustRead = (ma_uint32)ma_pcm_converter_read(&audio_buffer->dsp, tempBuffer, framesToReadRightNow);
        if (framesJustRead > 0)
        {
          float *framesOut = (float *)pOutput + (frames_read*g_audio.device.playback.channels);
          float *framesIn  = tempBuffer;
          binocle_audio_mix_audio_frames(pDevice, framesOut, framesIn, framesJustRead, audio_buffer->volume);

          framesToRead -= framesJustRead;
          frames_read += framesJustRead;
        }

        // If we weren't able to read all the frames we requested, break.
        if (framesJustRead < framesToReadRightNow)
        {
          if (!audio_buffer->looping)
          {
            binocle_audio_stop_audio_buffer(audio_buffer);
            break;
          }
          else
          {
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

static ma_uint32 binocle_audio_on_audio_buffer_dsp_read(ma_pcm_converter *pDSP, void *pFramesOut, ma_uint32 frameCount, void *pUserData)
{
  binocle_audio_buffer *audioBuffer = (binocle_audio_buffer *)pUserData;

  ma_uint32 subBufferSizeInFrames = (audioBuffer->buffer_size_in_frames > 1)? audioBuffer->buffer_size_in_frames/2 : audioBuffer->buffer_size_in_frames;
  ma_uint32 currentSubBufferIndex = audioBuffer->frame_cursor_pos/subBufferSizeInFrames;

  if (currentSubBufferIndex > 1)
  {
    binocle_log_debug("Frame cursor position moved too far forward in audio stream");
    return 0;
  }

  // Another thread can update the processed state of buffers so we just take a copy here to try and avoid potential synchronization problems.
  bool isSubBufferProcessed[2];
  isSubBufferProcessed[0] = audioBuffer->is_sub_buffer_processed[0];
  isSubBufferProcessed[1] = audioBuffer->is_sub_buffer_processed[1];

  ma_uint32 frameSizeInBytes = ma_get_bytes_per_sample(audioBuffer->dsp.formatConverterIn.config.formatIn)*audioBuffer->dsp.formatConverterIn.config.channels;

  // Fill out every frame until we find a buffer that's marked as processed. Then fill the remainder with 0.
  ma_uint32 framesRead = 0;
  for (;;)
  {
    // We break from this loop differently depending on the buffer's usage. For static buffers, we simply fill as much data as we can. For
    // streaming buffers we only fill the halves of the buffer that are processed. Unprocessed halves must keep their audio data in-tact.
    if (audioBuffer->usage == BINOCLE_AUDIO_BUFFER_USAGE_STATIC)
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
    if (audioBuffer->usage == BINOCLE_AUDIO_BUFFER_USAGE_STATIC)
    {
      framesRemainingInOutputBuffer = audioBuffer->buffer_size_in_frames - audioBuffer->frame_cursor_pos;
    }
    else
    {
      ma_uint32 firstFrameIndexOfThisSubBuffer = subBufferSizeInFrames * currentSubBufferIndex;
      framesRemainingInOutputBuffer = subBufferSizeInFrames - (audioBuffer->frame_cursor_pos - firstFrameIndexOfThisSubBuffer);
    }

    ma_uint32 framesToRead = totalFramesRemaining;
    if (framesToRead > framesRemainingInOutputBuffer) framesToRead = framesRemainingInOutputBuffer;

    memcpy((unsigned char *)pFramesOut + (framesRead*frameSizeInBytes), audioBuffer->buffer + (audioBuffer->frame_cursor_pos*frameSizeInBytes), framesToRead*frameSizeInBytes);
    audioBuffer->frame_cursor_pos = (audioBuffer->frame_cursor_pos + framesToRead) % audioBuffer->buffer_size_in_frames;
    framesRead += framesToRead;

    // If we've read to the end of the buffer, mark it as processed.
    if (framesToRead == framesRemainingInOutputBuffer)
    {
      audioBuffer->is_sub_buffer_processed[currentSubBufferIndex] = true;
      isSubBufferProcessed[currentSubBufferIndex] = true;

      currentSubBufferIndex = (currentSubBufferIndex + 1)%2;

      // We need to break from this loop if we're not looping.
      if (!audioBuffer->looping)
      {
        binocle_audio_stop_audio_buffer(audioBuffer);
        break;
      }
    }
  }

  // Zero-fill excess.
  ma_uint32 totalFramesRemaining = (frameCount - framesRead);
  if (totalFramesRemaining > 0)
  {
    memset((unsigned char *)pFramesOut + (framesRead*frameSizeInBytes), 0, totalFramesRemaining*frameSizeInBytes);

    // For static buffers we can fill the remaining frames with silence for safety, but we don't want
    // to report those frames as "read". The reason for this is that the caller uses the return value
    // to know whether or not a non-looping sound has finished playback.
    if (audioBuffer->usage != BINOCLE_AUDIO_BUFFER_USAGE_STATIC) framesRead += totalFramesRemaining;
  }

  return framesRead;
}

static void binocle_audio_mix_audio_frames(ma_device* pDevice, float *framesOut, const float *framesIn, ma_uint32 frameCount, float localVolume)
{
  for (ma_uint32 iFrame = 0; iFrame < frameCount; ++iFrame)
  {
    for (ma_uint32 iChannel = 0; iChannel < pDevice->playback.channels; ++iChannel)
    {
      float *frameOut = framesOut + (iFrame*pDevice->playback.channels);
      const float *frameIn  = framesIn  + (iFrame*pDevice->playback.channels);

      frameOut[iChannel] += (frameIn[iChannel]*g_audio.master_volume*localVolume);
    }
  }
}

bool binocle_audio_is_audio_device_ready(binocle_audio *audio)
{
  return audio->is_audio_initialized;
}

void binocle_audio_set_master_volume(binocle_audio *audio, float volume)
{
  if (volume < 0.0f) volume = 0.0f;
  else if (volume > 1.0f) volume = 1.0f;

  audio->master_volume = volume;
}

binocle_audio_buffer *binocle_audio_create_audio_buffer(ma_format format, ma_uint32 channels, ma_uint32 sampleRate, ma_uint32 bufferSizeInFrames, binocle_audio_buffer_usage usage)
{
  binocle_audio_buffer *audioBuffer = (binocle_audio_buffer *)calloc(sizeof(*audioBuffer) + (bufferSizeInFrames*channels*ma_get_bytes_per_sample(format)), 1);
  if (audioBuffer == NULL)
  {
    binocle_log_error("CreateAudioBuffer() : Failed to allocate memory for audio buffer");
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

  if (result != MA_SUCCESS)
  {
    binocle_log_error("CreateAudioBuffer() : Failed to create data conversion pipeline");
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

  binocle_audio_track_audio_buffer(audioBuffer);

  return audioBuffer;
}

void binocle_audio_delete_audio_buffer(binocle_audio_buffer *audioBuffer)
{
  if (audioBuffer == NULL)
  {
    binocle_log_error("DeleteAudioBuffer() : No audio buffer");
    return;
  }

  binocle_audio_untrack_audio_buffer(audioBuffer);
  free(audioBuffer);
}

bool binocle_audio_is_audio_buffer_playing(binocle_audio_buffer *audio_buffer)
{
  if (audio_buffer == NULL)
  {
    binocle_log_debug("IsAudioBufferPlaying() : No audio buffer");
    return false;
  }

  return audio_buffer->playing && !audio_buffer->paused;
}

void binocle_audio_play_audio_buffer(binocle_audio_buffer *audioBuffer)
{
  if (audioBuffer == NULL)
  {
    binocle_log_error("PlayAudioBuffer() : No audio buffer");
    return;
  }

  audioBuffer->playing = true;
  audioBuffer->paused = false;
  audioBuffer->frame_cursor_pos = 0;
}

void binocle_audio_stop_audio_buffer(binocle_audio_buffer *audio_buffer)
{
  if (audio_buffer == NULL)
  {
    binocle_log_debug("StopAudioBuffer() : No audio buffer");
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

void binocle_audio_pause_audio_buffer(binocle_audio_buffer *audioBuffer)
{
  if (audioBuffer == NULL)
  {
    binocle_log_error("PauseAudioBuffer() : No audio buffer");
    return;
  }

  audioBuffer->paused = true;
}

void binocle_audio_resume_audio_buffer(binocle_audio_buffer *audioBuffer)
{
  if (audioBuffer == NULL)
  {
    binocle_log_error("ResumeAudioBuffer() : No audio buffer");
    return;
  }

  audioBuffer->paused = false;
}

void binocle_audio_set_audio_buffer_volume(binocle_audio_buffer *audioBuffer, float volume)
{
  if (audioBuffer == NULL)
  {
    binocle_log_error("SetAudioBufferVolume() : No audio buffer");
    return;
  }

  audioBuffer->volume = volume;
}

void binocle_audio_set_audio_buffer_pitch(binocle_audio_buffer *audioBuffer, float pitch)
{
  if (audioBuffer == NULL)
  {
    binocle_log_error("SetAudioBufferPitch() : No audio buffer");
    return;
  }

  float pitchMul = pitch/audioBuffer->pitch;

  // Pitching is just an adjustment of the sample rate. Note that this changes the duration of the sound - higher pitches
  // will make the sound faster; lower pitches make it slower.
  ma_uint32 newOutputSampleRate = (ma_uint32)((float)audioBuffer->dsp.src.config.sampleRateOut / pitchMul);
  audioBuffer->pitch *= (float)audioBuffer->dsp.src.config.sampleRateOut / newOutputSampleRate;

  ma_pcm_converter_set_output_sample_rate(&audioBuffer->dsp, newOutputSampleRate);
}

void binocle_audio_track_audio_buffer(binocle_audio_buffer *audioBuffer)
{
  if (g_audio.first_audio_buffer == NULL) g_audio.first_audio_buffer = audioBuffer;
  else
  {
    g_audio.last_audio_buffer->next = audioBuffer;
    audioBuffer->prev = g_audio.last_audio_buffer;
  }

  g_audio.last_audio_buffer = audioBuffer;
}

void binocle_audio_untrack_audio_buffer(binocle_audio_buffer *audioBuffer)
{
  if (audioBuffer->prev == NULL) g_audio.first_audio_buffer = audioBuffer->next;
  else audioBuffer->prev->next = audioBuffer->next;

  if (audioBuffer->next == NULL) g_audio.last_audio_buffer = audioBuffer->prev;
  else audioBuffer->next->prev = audioBuffer->prev;

  audioBuffer->prev = NULL;
  audioBuffer->next = NULL;
}

//
// Sound loading and playing stuff
//

bool binocle_audio_is_file_extension(const char *fileName, const char *ext)
{
  bool result = false;
  const char *fileExt;

  if ((fileExt = strrchr(fileName, '.')) != NULL)
  {
    if (strcmp(fileExt, ext) == 0) result = true;
  }

  return result;
}


binocle_audio_wave binocle_audio_load_wave(const char *fileName)
{
  // TODO: add support for loading MOD and XM

  binocle_audio_wave wave = { 0 };

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

static binocle_audio_wave binocle_audio_load_ogg(const char *fileName)
{
  binocle_audio_wave wave = { 0 };

  stb_vorbis *oggFile = stb_vorbis_open_filename(fileName, NULL, NULL);

  if (oggFile == NULL) binocle_log_warning("[%s] OGG file could not be opened", fileName);
  else
  {
    stb_vorbis_info info = stb_vorbis_get_info(oggFile);

    wave.sample_rate = info.sample_rate;
    wave.sample_size = 16;                   // 16 bit per sample (short)
    wave.channels = info.channels;
    wave.sample_count = (unsigned int)stb_vorbis_stream_length_in_samples(oggFile)*info.channels;  // Independent by channel

    float totalSeconds = stb_vorbis_stream_length_in_seconds(oggFile);
    if (totalSeconds > 10) binocle_log_warning("[%s] Ogg audio length is larger than 10 seconds (%f), that's a big file in memory, consider music streaming", fileName, totalSeconds);

    wave.data = (short *)malloc(wave.sample_count*wave.channels*sizeof(short));

    // NOTE: Returns the number of samples to process (be careful! we ask for number of shorts!)
    int numSamplesOgg = stb_vorbis_get_samples_short_interleaved(oggFile, info.channels, (short *)wave.data, wave.sample_count*wave.channels);

    binocle_log_debug("[%s] Samples obtained: %i", fileName, numSamplesOgg);

    binocle_log_info("[%s] OGG file loaded successfully (%i Hz, %i bit, %s)", fileName, wave.sample_rate, wave.sample_size, (wave.channels == 1)? "Mono" : "Stereo");

    stb_vorbis_close(oggFile);
  }

  return wave;
}

static binocle_audio_wave binocle_audio_load_flac(const char *fileName)
{
  binocle_audio_wave wave;

  // Decode an entire FLAC file in one go
  uint64_t totalSampleCount;
  wave.data = drflac_open_file_and_read_pcm_frames_s16(fileName, &wave.channels, &wave.sample_rate, &totalSampleCount);

  wave.sample_count = (unsigned int)totalSampleCount;
  wave.sample_size = 16;

  // NOTE: Only support up to 2 channels (mono, stereo)
  if (wave.channels > 2) binocle_log_warning("[%s] FLAC channels number (%i) not supported", fileName, wave.channels);

  if (wave.data == NULL) binocle_log_warning("[%s] FLAC data could not be loaded", fileName);
  else binocle_log_info("[%s] FLAC file loaded successfully (%i Hz, %i bit, %s)", fileName, wave.sample_rate, wave.sample_size, (wave.channels == 1)? "Mono" : "Stereo");

  return wave;
}

static binocle_audio_wave binocle_audio_load_mp3(const char *fileName)
{
  binocle_audio_wave wave = { 0 };

  // Decode an entire MP3 file in one go
  uint64_t totalFrameCount = 0;
  drmp3_config config = { 0 };
  wave.data = drmp3_open_file_and_read_f32(fileName, &config, &totalFrameCount);

  wave.channels = config.outputChannels;
  wave.sample_rate = config.outputSampleRate;
  wave.sample_count = (int)totalFrameCount*wave.channels;
  wave.sample_size = 32;

  // NOTE: Only support up to 2 channels (mono, stereo)
  if (wave.channels > 2) binocle_log_warning("[%s] MP3 channels number (%i) not supported", fileName, wave.channels);

  if (wave.data == NULL) binocle_log_warning("[%s] MP3 data could not be loaded", fileName);
  else binocle_log_info("[%s] MP3 file loaded successfully (%i Hz, %i bit, %s)", fileName, wave.sample_rate, wave.sample_size, (wave.channels == 1)? "Mono" : "Stereo");

  return wave;
}

static binocle_audio_wave binocle_audio_load_wav(const char *fileName)
{
  binocle_audio_wave wave = { 0 };

  // Decode an entire MP3 file in one go
  uint64_t totalFrameCount = 0;
  wave.data = drwav_open_file_and_read_pcm_frames_f32(fileName, &wave.channels, &wave.sample_rate, &totalFrameCount);
  wave.sample_count = (int)totalFrameCount*wave.channels;
  wave.sample_size = 32;

  // NOTE: Only support up to 2 channels (mono, stereo)
  if (wave.channels > 2) binocle_log_warning("[%s] WAV channels number (%i) not supported", fileName, wave.channels);

  if (wave.data == NULL) binocle_log_warning("[%s] WAV data could not be loaded", fileName);
  else binocle_log_info("[%s] WAV file loaded successfully (%i Hz, %i bit, %s)", fileName, wave.sample_rate, wave.sample_size, (wave.channels == 1)? "Mono" : "Stereo");

  return wave;
}

binocle_audio_sound binocle_audio_load_sound(const char *fileName)
{
  binocle_audio_wave wave = binocle_audio_load_wave(fileName);

  binocle_audio_sound sound = binocle_audio_load_sound_from_wave(wave);

  binocle_audio_unload_wave(wave);       // Sound is loaded, we can unload wave

  return sound;
}

binocle_audio_sound binocle_audio_load_sound_from_wave(binocle_audio_wave wave)
{
  binocle_audio_sound sound = { 0 };

  if (wave.data != NULL)
  {
    // When using miniaudio we need to do our own mixing. To simplify this we need convert the format of each sound to be consistent with
    // the format used to open the playback device. We can do this two ways:
    //
    //   1) Convert the whole sound in one go at load time (here).
    //   2) Convert the audio data in chunks at mixing time.
    //
    // I have decided on the first option because it offloads work required for the format conversion to the to the loading stage.
    // The downside to this is that it uses more memory if the original sound is u8 or s16.
    ma_format formatIn  = ((wave.sample_size == 8)? ma_format_u8 : ((wave.sample_size == 16)? ma_format_s16 : ma_format_f32));
    ma_uint32 frameCountIn = wave.sample_count/wave.channels;

    ma_uint32 frameCount = (ma_uint32)ma_convert_frames(NULL, BINOCLE_AUDIO_SAMPLE_FORMAT, BINOCLE_AUDIO_CHANNEL_COUNT, BINOCLE_AUDIO_SAMPLE_RATE, NULL, formatIn, wave.channels, wave.sample_rate, frameCountIn);
    if (frameCount == 0) binocle_log_warning("LoadSoundFromWave() : Failed to get frame count for format conversion");

    binocle_audio_buffer* audioBuffer = binocle_audio_create_audio_buffer(BINOCLE_AUDIO_SAMPLE_FORMAT, BINOCLE_AUDIO_CHANNEL_COUNT, BINOCLE_AUDIO_SAMPLE_RATE, frameCount, BINOCLE_AUDIO_BUFFER_USAGE_STATIC);
    if (audioBuffer == NULL) binocle_log_warning("LoadSoundFromWave() : Failed to create audio buffer");

    if (audioBuffer != NULL) {
      frameCount = (ma_uint32)ma_convert_frames(audioBuffer->buffer, audioBuffer->dsp.formatConverterIn.config.formatIn, audioBuffer->dsp.formatConverterIn.config.channels, audioBuffer->dsp.src.config.sampleRateIn, wave.data, formatIn, wave.channels, wave.sample_rate, frameCountIn);
      if (frameCount == 0) binocle_log_warning("LoadSoundFromWave() : Format conversion failed");
    }

    sound.audio_buffer = audioBuffer;
  }

  return sound;
}

void binocle_audio_unload_wave(binocle_audio_wave wave)
{
  if (wave.data != NULL) free(wave.data);

  binocle_log_info("Unloaded wave data from RAM");
}

void binocle_audio_unload_sound(binocle_audio_sound sound)
{
  binocle_audio_delete_audio_buffer((binocle_audio_buffer *)sound.audio_buffer);

  binocle_log_info("[SND ID %i][BUFR ID %i] Unloaded sound data from RAM", sound.source, sound.buffer);
}

void binocle_audio_update_sound(binocle_audio_sound sound, const void *data, int samplesCount)
{
  binocle_audio_buffer *audioBuffer = (binocle_audio_buffer *)sound.audio_buffer;

  if (audioBuffer == NULL)
  {
    binocle_log_error("UpdateSound() : Invalid sound - no audio buffer");
    return;
  }

  binocle_audio_stop_audio_buffer(audioBuffer);

  // TODO: May want to lock/unlock this since this data buffer is read at mixing time.
  memcpy(audioBuffer->buffer, data, samplesCount*audioBuffer->dsp.formatConverterIn.config.channels*ma_get_bytes_per_sample(audioBuffer->dsp.formatConverterIn.config.formatIn));
}

void binocle_audio_play_sound(binocle_audio_sound sound)
{
  binocle_audio_play_audio_buffer((binocle_audio_buffer *)sound.audio_buffer);
}

void binocle_audio_pause_sound(binocle_audio_sound sound)
{
  binocle_audio_pause_audio_buffer((binocle_audio_buffer *)sound.audio_buffer);
}

void binocle_audio_resume_sound(binocle_audio_sound sound)
{
  binocle_audio_resume_audio_buffer((binocle_audio_buffer *)sound.audio_buffer);
}

void binocle_audio_stop_sound(binocle_audio_sound sound)
{
  binocle_audio_stop_audio_buffer((binocle_audio_buffer *)sound.audio_buffer);
}

bool binocle_audio_is_sound_playing(binocle_audio_sound sound)
{
  return binocle_audio_is_audio_buffer_playing((binocle_audio_buffer *)sound.audio_buffer);
}

void binocle_audio_set_sound_volume(binocle_audio_sound sound, float volume)
{
  binocle_audio_set_audio_buffer_volume((binocle_audio_buffer *)sound.audio_buffer, volume);
}

void binocle_audio_set_sound_pitch(binocle_audio_sound sound, float pitch)
{
  binocle_audio_set_audio_buffer_pitch((binocle_audio_buffer *)sound.audio_buffer, pitch);
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