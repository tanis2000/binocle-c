//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_timer.h"
#include "binocle_sdl.h"

binocle_timer binocle_timer_new() {
  binocle_timer result = {0};
  result.startMark = 0;
  result.stopMark = 0;
  result.pausedMark = 0;
  result.running = -1;
  result.paused = -1;
  return result;
}

void binocle_timer_start(binocle_timer *timer) {
  if (timer->running) return;

  timer->startMark = SDL_GetTicks();
  timer->stopMark = 0;
  timer->pausedMark = 0;
  timer->running = 0;
  timer->paused = -1;
}

void binocle_timer_stop(binocle_timer *timer) {
  if (!timer->running) return;

  timer->stopMark = SDL_GetTicks();
  timer->running = -1;
  timer->paused = -1;
}

void binocle_timer_restart(binocle_timer *timer) {
  binocle_timer_stop(timer);
  binocle_timer_start(timer);
}

void binocle_timer_pause(binocle_timer *timer) {
  if (!timer->running || timer->paused) return;

  timer->running = -1;
  timer->paused = 0;
  timer->pausedMark = (SDL_GetTicks()) - (timer->startMark);
}

void binocle_timer_unpause(binocle_timer *timer) {
  if (!timer->paused || timer->running) return;

  timer->running = 0;
  timer->paused = -1;

  timer->startMark = (SDL_GetTicks()) - (timer->pausedMark);
  timer->pausedMark = 0;
}

int32_t binocle_timer_is_running(const binocle_timer *timer) {
  return timer->running;
}

int32_t binocle_timer_is_paused(const binocle_timer *timer) {
  return timer->paused;
}

uint32_t binocle_timer_delta(const binocle_timer *timer) {
  if (binocle_timer_is_running(timer))
    return binocle_timer_current_time(timer);

  if (timer->paused)
    return timer->pausedMark;

  // Something went wrong here
  if (timer->startMark == 0)
    return 0;

  return timer->stopMark - timer->startMark;
}

uint32_t binocle_timer_delta_ms(const binocle_timer *timer) {
  return binocle_timer_delta(timer) % 1000;
}

uint32_t binocle_timer_delta_s(const binocle_timer *timer) {
  return binocle_timer_delta(timer) / 1000;
}

uint32_t binocle_timer_current_time(const binocle_timer *timer) {
  return SDL_GetTicks() - timer->startMark;
}
