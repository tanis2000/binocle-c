//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_WREN_H
#define BINOCLE_WREN_H

#include "wren/src/include/wren.h"

struct binocle_input;

typedef struct binocle_wren_t {
  WrenConfiguration config;
  WrenVM* vm;
  char *last_script_run;
} binocle_wren_t;

binocle_wren_t *binocle_wren_new();
void binocle_wren_init(struct binocle_wren_t *wren);
void binocle_wren_log(WrenVM *vm, const char* text);
void binocle_wren_error(WrenVM *vm, WrenErrorType type,
                        const char* module,
                        int line,
                        const char* message);
bool binocle_wren_run_script(binocle_wren_t *wren, char *filename);
WrenForeignMethodFn binocle_wren_bind_foreign_method_fn(
    WrenVM* vm,
    const char* module,
    const char* className,
    bool isStatic,
    const char* signature);
WrenForeignClassMethods binocle_wren_bind_foreign_class_fn(
    WrenVM* vm, const char* module, const char* className);
void binocle_wren_wrap_input(binocle_wren_t *wren, struct binocle_input *input);

#endif //BINOCLE_WREN_H
