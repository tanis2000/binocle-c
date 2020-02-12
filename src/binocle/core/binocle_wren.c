//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_wren.h"
#include <stdlib.h>
#include <string.h>
#include "binocle_log.h"

binocle_wren_t *binocle_wren_new() {
  binocle_wren_t *res = malloc(sizeof(binocle_wren_t));
  memset(res, 0, sizeof(*res));
  return res;
}

void binocle_wren_init(struct binocle_wren_t *wren) {
  wrenInitConfiguration(&wren->config);
  wren->config.writeFn = binocle_wren_log;
  wren->config.errorFn = binocle_wren_error;
  wren->vm = wrenNewVM(&wren->config);
  WrenInterpretResult result = wrenInterpret(
      wren->vm,
      "my_module",
      "System.print(\"I am running in a VM!\")");
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