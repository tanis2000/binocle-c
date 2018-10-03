//
// Created by Valerio Santinelli on 13/04/18.
//

#include "binocle_color.h"

binocle_color binocle_color_new(float r, float g, float b, float a) {
    binocle_color res;
    res.r = r;
    res.g = g;
    res.b = b;
    res.a = a;
    return res;
}