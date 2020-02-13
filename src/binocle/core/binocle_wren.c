//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_wren.h"
#include <stdlib.h>
#include <string.h>
#include "binocle_log.h"
#include "binocle_sdl.h"
#include "binocle_input.h"

binocle_wren_t *binocle_wren_new() {
  binocle_wren_t *res = malloc(sizeof(binocle_wren_t));
  memset(res, 0, sizeof(*res));
  return res;
}

void binocle_wren_init(struct binocle_wren_t *wren) {
  wrenInitConfiguration(&wren->config);
  wren->config.writeFn = binocle_wren_log;
  wren->config.errorFn = binocle_wren_error;
  wren->config.bindForeignMethodFn = binocle_wren_bind_foreign_method_fn;
  wren->config.bindForeignClassFn = binocle_wren_bind_foreign_class_fn;
  wren->vm = wrenNewVM(&wren->config);
}

void binocle_wren_log(WrenVM *vm, const char* text) {
  binocle_log_info(text);
}

void binocle_wren_error(WrenVM *vm, WrenErrorType type,
                        const char* module,
                        int line,
                        const char* message) {
  binocle_log_error("Error in module %s, line %d, message: %s", module, line, message);
}

bool binocle_wren_run_script(binocle_wren_t *wren, char *filename) {
  char *buffer = NULL;
  size_t size;
  if (!binocle_sdl_load_text_file(filename, &buffer, &size)) {
    binocle_log_error("Couldn't load file: %s\n", filename);
    return false;
  }

  WrenInterpretResult result = wrenInterpret(
      wren->vm,
      "main",
      buffer);

  if (result != WREN_RESULT_SUCCESS) {
    binocle_log_error("Failed to run script: %s\n", filename);
    return false;
  }

  if (wren->last_script_run != NULL) {
    free(wren->last_script_run);
  }
  wren->last_script_run = malloc(strlen(filename)+1);
  strcpy(wren->last_script_run, filename);
  return true;
}

void binocle_wren_input_is_key_pressed(WrenVM *vm) {
  binocle_input *input = (binocle_input *)wrenGetUserData(vm);
  bool res = binocle_input_is_key_pressed(input, KEY_A);
  wrenSetSlotBool(vm, 0, res);
}

void binocle_wren_wrap_input(binocle_wren_t *wren, struct binocle_input *input) {
  wrenSetUserData(wren->vm, input);
}

void binocle_wren_input_instance(WrenVM *vm) {
  binocle_input *input = (binocle_input *)wrenGetUserData(vm);
  binocle_input *new_input = (binocle_input*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(binocle_input));
  memcpy(new_input, input, sizeof(binocle_input));
}

WrenForeignMethodFn binocle_wren_bind_foreign_method_fn(
    WrenVM* vm,
    const char* module,
    const char* className,
    bool isStatic,
    const char* signature) {
  if (strcmp(module, "main") == 0)
  {
    if (strcmp(className, "Input") == 0)
    {
      if (!isStatic && strcmp(signature, "is_key_pressed()") == 0)
      {
        return binocle_wren_input_is_key_pressed; // C function for Math.add(_,_).
      }
      // Other foreign methods on Math...
    }
    // Other classes in main...
  }
}

WrenForeignClassMethods binocle_wren_bind_foreign_class_fn(
    WrenVM* vm, const char* module, const char* className) {
  WrenForeignClassMethods methods = { NULL, NULL };


  if (strcmp(module, "main") == 0)
  {
    if (strcmp(className, "Input") == 0)
    {
      methods.allocate = binocle_wren_input_instance;
      return methods; // C function for Math.add(_,_).
      // Other foreign methods on Math...
    }
    // Other classes in main...
  }
}