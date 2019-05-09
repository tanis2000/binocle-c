//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_material.h"

binocle_material binocle_material_new() {
    binocle_material res = {
            .blend_mode = binocle_blend_new_default()
    };
    return res;
}