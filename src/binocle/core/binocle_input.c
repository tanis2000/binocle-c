//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include <string.h>
#include <kazmath/kazmath.h>
#include "binocle_input.h"
#include "binocle_camera.h"

binocle_input binocle_input_new() {
  binocle_input res = {
      .quit_requested = false
  };
  for (int i = 0; i < (int) KEY_MAX; i++) {
    res.previousKeys[i] = res.currentKeys[i] = false;
  }

  for (int i = 0; i < (int) MOUSE_MAX; i++) {
    res.previousMouseButtons[i] = res.currentMouseButtons[i] = false;
  }
  return res;
}

void binocle_input_update(binocle_input *input) {
  int i;
  const Uint8 *state = SDL_GetKeyboardState(NULL);
  for (i = 0; i < KEY_MAX; i++) {
    input->previousKeys[i] = input->currentKeys[i];
    input->currentKeys[i] = state[i];
  }
  for (i = 0; i < MOUSE_MAX; i++) {
    if (i == 0) continue;
    input->previousMouseButtons[i] = input->currentMouseButtons[i];
    input->currentMouseButtons[i] = (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(i)) == 0 ? false : true;
  }
  input->curPrintableKey = KEY_UNKNOWN;

  memset(input->text, 0, SDL_TEXTINPUTEVENT_TEXT_SIZE);

  input->mouseWheelX = 0;
  input->mouseWheelY = 0;

  // Get key events from the OS
  SDL_Event event = {0};
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        input->quit_requested = true;
        break;

      case SDL_WINDOWEVENT: {
        switch (event.window.event) {
          case SDL_WINDOWEVENT_FOCUS_LOST:
            input->willPause = true;
            break;

          case SDL_WINDOWEVENT_FOCUS_GAINED:
            input->willPause = false;
            break;

          case SDL_WINDOWEVENT_RESIZED:
            input->resized = true;
            input->newWindowSize.x = event.window.data1;
            input->newWindowSize.y = event.window.data2;
            break;
        }
        break;
      }

      case SDL_KEYDOWN: {
        int index = event.key.keysym.scancode;

        if (binocle_input_is_printable((binocle_input_keyboard_key) index))
          input->curPrintableKey = (binocle_input_keyboard_key) index;
      }
        break;

      case SDL_KEYUP: {

        int index = event.key.keysym.scancode;
      }
        break;

      case SDL_MOUSEMOTION:
        input->mouseX = event.motion.x;
        input->mouseY = event.motion.y;
        break;

      case SDL_MOUSEBUTTONDOWN:

        break;

      case SDL_MOUSEBUTTONUP:

        break;

      case SDL_MOUSEWHEEL: {
        input->mouseWheelX = event.wheel.x;
        input->mouseWheelY = event.wheel.y;
        break;
      }

      case SDL_FINGERDOWN:
        input->touch.x = event.tfinger.x;
        input->touch.y = event.tfinger.y;
        input->touch.dx = event.tfinger.dx;
        input->touch.dy = event.tfinger.dy;
        input->touch.pressure = event.tfinger.pressure;
        input->touch.type = FINGER_DOWN;
        break;
      case SDL_FINGERMOTION:
        input->touch.x = event.tfinger.x;
        input->touch.y = event.tfinger.y;
        input->touch.dx = event.tfinger.dx;
        input->touch.dy = event.tfinger.dy;
        input->touch.pressure = event.tfinger.pressure;
        input->touch.type = FINGER_MOTION;
        break;
      case SDL_FINGERUP:
        input->touch.x = event.tfinger.x;
        input->touch.y = event.tfinger.y;
        input->touch.dx = event.tfinger.dx;
        input->touch.dy = event.tfinger.dy;
        input->touch.pressure = event.tfinger.pressure;
        input->touch.type = FINGER_UP;
        break;
      case SDL_TEXTINPUT: {
        memcpy(input->text, event.text.text, SDL_TEXTINPUTEVENT_TEXT_SIZE);
        break;
      }
      default:
        break;
    }
  }
  // If we're paused we let the CPU resources free and slow down
  if (input->willPause && !input->force_skip_pause) {
    SDL_Delay(100);
  }
}

bool binocle_input_is_key_down(binocle_input input, int key) {
  if (input.isLocked)
    return false;

  if (key < 0 || key >= KEY_MAX)
    return false;

  return input.currentKeys[key] && !input.previousKeys[key];
}

bool binocle_input_is_key_up(binocle_input input, int key) {
  if (input.isLocked)
    return false;

  if (key < 0 || key >= KEY_MAX)
    return false;

  return !input.currentKeys[key] && input.previousKeys[key];
}

bool binocle_input_is_key_pressed(binocle_input *input, binocle_input_keyboard_key key) {
  if (input->isLocked)
    return false;

  int sdl_key = (int) (key);

  if (input->currentKeys[sdl_key])
    return true;

  return false;
}

bool binocle_input_shift(binocle_input input) {
  return (binocle_input_is_key_pressed(&input, KEY_LEFT_SHIFT) ||
          binocle_input_is_key_pressed(&input, KEY_RIGHT_SHIFT));
}

bool binocle_input_ctrl(binocle_input input) {
  return (binocle_input_is_key_pressed(&input, KEY_LEFT_CTRL) ||
          binocle_input_is_key_pressed(&input, KEY_RIGHT_CTRL));
}

bool binocle_input_alt(binocle_input input) {
  return (binocle_input_is_key_pressed(&input, KEY_LEFT_ALT) ||
          binocle_input_is_key_pressed(&input, KEY_RIGHT_ALT));
}

bool binocle_input_is_mouse_down(binocle_input input, binocle_input_mouse_button button) {
  if (input.isLocked)
    return false;

  if (button < 0 || button >= MOUSE_MAX)
    return false;

  return input.currentMouseButtons[button] && !input.previousMouseButtons[button];
}

bool binocle_input_is_mouse_up(binocle_input input, binocle_input_mouse_button button) {
  if (input.isLocked)
    return false;

  if (button < 0 || button >= MOUSE_MAX)
    return false;

  return !input.currentMouseButtons[button] && input.previousMouseButtons[button];
}

bool binocle_input_is_mouse_pressed(binocle_input input, binocle_input_mouse_button button) {
  if (input.isLocked)
    return false;

  int sdl_key = (int) (button);

  if (input.currentMouseButtons[sdl_key])
    return true;

  return false;
}

int binocle_input_get_mouse_x(binocle_input input) { return input.mouseX; }

int binocle_input_get_mouse_y(binocle_input input) { return input.mouseY; }

bool binocle_input_quit_requested(binocle_input input) { return (input.quit_requested); }

bool binocle_input_pause_requested(binocle_input input) { return (input.willPause); }

bool binocle_input_is_mouse_inside(binocle_input input, kmAABB2 rectangle) {
  int mouse_x = input.mouseX;
  int mouse_y = input.mouseY;
  if (input.inverted_mouse_position) {
    mouse_y = input.screen_height - mouse_y;
  }
  if ((mouse_x >= rectangle.min.x) &&
      (mouse_x <= rectangle.min.x + rectangle.max.x) &&
      (mouse_y >= rectangle.min.y) &&
      (mouse_y <= rectangle.min.y + rectangle.max.y))
    return true;

  return false;
}

kmVec2 binocle_input_get_mouse_position(binocle_input input, binocle_camera camera) {
  int mouse_x = input.mouseX;
  int mouse_y = input.mouseY;
  if (input.inverted_mouse_position) {
    mouse_y = input.screen_height - mouse_y;
  }
  kmVec2 res = {
      .x = mouse_x + camera.origin.x,
      .y = mouse_y + camera.origin.y
  };
  return res;
}

bool binocle_input_is_printable(SDL_Keycode key) {
  return ((key > SDLK_SPACE) && (key < SDLK_z));
}

bool binocle_input_is_printable_key_down(binocle_input input) {
  return (binocle_input_is_printable(input.curPrintableKey));
}

const char binocle_input_get_cur_printable_key(binocle_input input) {
  char c = (char) (input.curPrintableKey);
  return c;
}

void binocle_input_lock(binocle_input input) { input.isLocked = true; }

void binocle_input_unlock(binocle_input input) { input.isLocked = false; }

bool binocle_input_is_touch_down(binocle_input input, unsigned int finger) {
  if (input.touch.type == FINGER_DOWN) {
    return true;
  }
  return false;
}

kmVec2 binocle_input_get_touch_position(binocle_input input, unsigned int finger, binocle_camera camera) {
  kmVec2 world_pos;
  world_pos.x = input.touch.x + camera.origin.x;
  world_pos.y = input.touch.y + camera.origin.y;
  return world_pos;
}

void binocle_input_invert_mouse_position(binocle_input *input, uint32_t screen_height) {
  input->inverted_mouse_position = true;
  input->screen_height = screen_height;
}

void binocle_input_default_mouse_position(binocle_input *input) {
  input->inverted_mouse_position = false;
}
