//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_BINOCLE_HTTP_WRAP_H
#define BINOCLE_BINOCLE_HTTP_WRAP_H

#if defined(BINOCLE_HTTP)

#include "binocle_lua.h"

int luaopen_http(lua_State *L);

#endif // BINOCLE_HTTP

#endif // BINOCLE_BINOCLE_HTTP_WRAP_H
