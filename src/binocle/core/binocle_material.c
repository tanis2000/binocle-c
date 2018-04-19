//
// Created by Valerio Santinelli on 19/04/18.
//

#include "binocle_material.h"

binocle_material binocle_material_new() {
    binocle_material res = {
            .blend_mode = binocle_blend_new_default()
    };
    return res;
}