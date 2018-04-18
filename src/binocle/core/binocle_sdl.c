//
// Created by Valerio Santinelli on 13/04/18.
//

#include "binocle_sdl.h"
#include "binocle_log.h"

bool binocle_sdl_init() {

  // Will initialize subsystems separatedly

  // Make sure we don't end up using Retina
  SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "1");

  // Make sure textures are being created using nearest pixel sampling
  // 0 = nearest pixel sampling
  // 1 = linear filtering
  // 2 = anisotropic filtering
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");


  if (SDL_Init(0) < 0) {
    binocle_log_error("SDL_Init: Couldn't start SDL");
    binocle_log_error(SDL_GetError());
    binocle_sdl_exit();
  }

  // Initializing everything related to AUDIO

  if (SDL_WasInit(SDL_INIT_AUDIO) != 0) {
    binocle_log_error("SDL_WasInit: Tried to reinitailize Audio");
    binocle_log_error(SDL_GetError());
    binocle_sdl_exit();
  }
  if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
    binocle_log_error("SDL_Init: Couldn't start Audio");
    binocle_log_error(SDL_GetError());
    binocle_sdl_exit();
  }

  int flags = MIX_INIT_OGG | MIX_INIT_MP3;
  if ((Mix_Init(flags) & flags) != flags) {
    binocle_log_warning("Mix_Init: Couldn't start Audio");
    binocle_log_warning(SDL_GetError());
  }

  // TODO: How do I find out the optimal
  //       audio rate of a music?

  if (-1 == Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, // audio rate
                          MIX_DEFAULT_FORMAT,    // format
                          2,                     // channels
                          4096))                 // buffers
  {
    binocle_log_warning("Mix_OpenAudio: Couldn't start Audio");
    binocle_log_warning(SDL_GetError());
  }

  // Reserving 16 channels (meaning 16 simultaneous SFXs playing)
  Mix_AllocateChannels(16);

  /* print out some info on the formats this run of SDL_mixer supports */
  {
    int i, n = Mix_GetNumChunkDecoders();
    SDL_Log("There are %d available chunk(sample) decoders:\n", n);
    for (i = 0; i < n; ++i)
      SDL_Log("	%s\n", Mix_GetChunkDecoder(i));
    n = Mix_GetNumMusicDecoders();
    SDL_Log("There are %d available music decoders:\n", n);
    for (i = 0; i < n; ++i)
      SDL_Log("	%s\n", Mix_GetMusicDecoder(i));
  }


  // Initializing everything related to VIDEO

  // Subtle bug here: If VIDEO is initialized but EVENTS
  //                  not or vice-versa.

  flags = (SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  if (SDL_WasInit(flags) != 0) {
    binocle_log_error("SDL_WasInit: Tried to reinitailize Video");
    binocle_log_error(SDL_GetError());
    binocle_sdl_exit();
  }
  if (SDL_InitSubSystem(flags) < 0) {
    binocle_log_error("SDL_Init: Couldn't start Video");
    binocle_log_error(SDL_GetError());
    binocle_sdl_exit();
  }


  return true;
}

void binocle_sdl_exit() {
  // Freeing everything related to VIDEO


  // Freeing everything related to AUDIO

  Mix_AllocateChannels(0); // Frees all allocated channels

  //Mix_Quit(); segmentation fault! why?
  Mix_CloseAudio();

  // Freeing everything related to FONTS

  //XXX: release TTF stuff
  /*if (TTF_WasInit())
      TTF_Quit();*/

  SDL_Quit();
}

time_t binocle_sdl_get_last_write_time(char *Filename)
{
  time_t LastWriteTime = 0;

  struct stat FileStatus;
  if(stat(Filename, &FileStatus) == 0)
  {
    LastWriteTime = FileStatus.st_mtime;
  }

  return(LastWriteTime);
}