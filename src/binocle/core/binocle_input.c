//
// Created by Valerio Santinelli on 18/04/18.
//

#include "binocle_input.h"

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