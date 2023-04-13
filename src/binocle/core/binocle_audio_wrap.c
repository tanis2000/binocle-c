//
// Created by Valerio Santinelli on 27/03/22.
//

#include "binocle_audio_wrap.h"
#include "binocle_sdl.h"
#include "binocle_audio.h"

int l_binocle_audio_new(lua_State *L) {
  l_binocle_audio_t *audio = lua_newuserdata(L, sizeof(l_binocle_audio_t));
  lua_getfield(L, LUA_REGISTRYINDEX, "binocle_audio");
  lua_setmetatable(L, -2);
  SDL_memset(audio, 0, sizeof(*audio));
  binocle_audio audio_instance = binocle_audio_new();
  audio->audio = SDL_malloc(sizeof(binocle_audio));
  SDL_memcpy(audio->audio, &audio_instance, sizeof(binocle_audio));
  return 1;
}

int l_binocle_audio_init(lua_State *L) {
  l_binocle_audio_t *audio = luaL_checkudata(L, 1, "binocle_audio");
  binocle_audio_init(audio->audio);
  return 0;
}

int l_binocle_audio_load_music_stream(lua_State *L) {
  l_binocle_audio_t *audio = luaL_checkudata(L, 1, "binocle_audio");
  const char *filename = lua_tostring(L, 2);
  audio->musics[audio->num_musics] = binocle_audio_load_music_stream(audio->audio, filename);
  l_binocle_audio_music_t *lmusic = lua_newuserdata(L, sizeof(l_binocle_audio_music_t));
  lua_getfield(L, LUA_REGISTRYINDEX, "binocle_audio_music");
  lua_setmetatable(L, -2);
  SDL_memset(lmusic, 0, sizeof(*lmusic));
  lmusic->music_idx = audio->num_musics;
  audio->num_musics++;
  return 1;
}

int l_binocle_audio_play_music_stream(lua_State *L) {
  l_binocle_audio_t *audio = luaL_checkudata(L, 1, "binocle_audio");
  l_binocle_audio_music_t *music = luaL_checkudata(L, 2, "binocle_audio_music");
  binocle_audio_play_music_stream(&audio->musics[music->music_idx]);
  return 0;
}

int l_binocle_audio_set_music_volume(lua_State *L) {
  l_binocle_audio_t *audio = luaL_checkudata(L, 1, "binocle_audio");
  l_binocle_audio_music_t *music = luaL_checkudata(L, 2, "binocle_audio_music");
  float volume = (float)lua_tonumber(L, 3);
  binocle_audio_set_music_volume(&audio->musics[music->music_idx], volume);
  return 0;
}

int l_binocle_audio_update_music_stream(lua_State *L) {
  l_binocle_audio_t *audio = luaL_checkudata(L, 1, "binocle_audio");
  l_binocle_audio_music_t *music = luaL_checkudata(L, 2, "binocle_audio_music");
  binocle_audio_update_music_stream(&audio->musics[music->music_idx]);
  return 0;
}

int l_binocle_audio_load_sound(lua_State *L) {
  l_binocle_audio_t *audio = luaL_checkudata(L, 1, "binocle_audio");
  const char *filename = luaL_checkstring(L, 2);
  binocle_audio_sound sound = binocle_audio_load_sound(audio->audio, filename);
  l_binocle_audio_sound_t *lsound = lua_newuserdata(L, sizeof(l_binocle_audio_sound_t));
  luaL_getmetatable(L, "binocle_audio_sound");
  lua_setmetatable(L, -2);
  SDL_memset(lsound, 0, sizeof(*lsound));
  lsound->sound = SDL_malloc(sizeof(binocle_audio_sound));
  SDL_memcpy(lsound->sound, &sound, sizeof(binocle_audio_sound));
  return 1;
}

int l_binocle_audio_play_sound(lua_State *L) {
  l_binocle_audio_sound_t *sound = luaL_checkudata(L, 1, "binocle_audio_sound");
  binocle_audio_play_sound(*sound->sound);
  return 0;
}

int l_binocle_audio_set_sound_volume(lua_State *L) {
  l_binocle_audio_sound_t *sound = luaL_checkudata(L, 1, "binocle_audio_sound");
  float volume = (float)lua_tonumber(L, 2);
  binocle_audio_set_sound_volume(*sound->sound, volume);
  return 0;
}

static const struct luaL_Reg audio [] = {
  {"new", l_binocle_audio_new},
  {"init", l_binocle_audio_init},
  {"load_music", l_binocle_audio_load_music_stream},
  {"play_music", l_binocle_audio_play_music_stream},
  {"set_music_volume", l_binocle_audio_set_music_volume},
  {"update_music_stream", l_binocle_audio_update_music_stream},
  {"load_sound", l_binocle_audio_load_sound},
  {"play_sound", l_binocle_audio_play_sound},
  {"set_sound_volume", l_binocle_audio_set_sound_volume},
  {NULL, NULL}
};

int luaopen_audio(lua_State *L) {
  luaL_newlib(L, audio);
  lua_setglobal(L, "audio");
  luaL_newmetatable(L, "binocle_audio");
  luaL_newmetatable(L, "binocle_audio_music");
  luaL_newmetatable(L, "binocle_audio_sound");
  lua_pop(L, 3);
  return 1;
}