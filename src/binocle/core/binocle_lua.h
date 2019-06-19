//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_LUA_H
#define BINOCLE_LUA_H

struct binocle_window;

int lua_test(const char *arg);
int lua_test2(const char *arg);
int lua_testffi(const char *arg, struct binocle_window *window);

#endif //BINOCLE_LUA_H
