//
// Created by Valerio Santinelli on 13/04/18.
//

#ifndef BINOCLE_TIMER_H
#define BINOCLE_TIMER_H

#include "stdint.h" // for uint32_t

/// General-purpose timer, measured in milliseconds.

typedef struct binocle_timer {
  uint32_t startMark;
  uint32_t stopMark;
  uint32_t pausedMark;

  int32_t running;
  int32_t paused;
} binocle_timer;

binocle_timer binocle_timer_new();

/// Sets a starting point for the timer.
void binocle_timer_start(binocle_timer *timer);

/// Sets a stopping point for the timer.
void binocle_timer_stop(binocle_timer *timer);

/// Temporarily stops the timer.
void binocle_timer_pause(binocle_timer *timer);

/// Stops and starts the timer.
void binocle_timer_restart(binocle_timer *timer);

/// Restarts the timer if it was paused.
void binocle_timer_unpause(binocle_timer *timer);

/// Tells if the timer's still running (hasn't called stop())
int32_t binocle_timer_is_running(binocle_timer *timer);

/// Tells if the timer's paused.
int32_t binocle_timer_is_paused(binocle_timer *timer);

/// Returns the whole timer's difference in milisseconds.
//	@note If the timer's not started, will return 0.
uint32_t binocle_timer_delta(binocle_timer *timer);

/// Returns the milisseconds part of the timer's difference.
uint32_t binocle_timer_delta_ms(binocle_timer *timer);

/// Returns the seconds part of the timer's difference.
uint32_t binocle_timer_delta_s(binocle_timer *timer);

/// Returns the difference between timer's start point and now.
uint32_t binocle_timer_current_time(binocle_timer *timer);

#endif //BINOCLE_TIMER_H
