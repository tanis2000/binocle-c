//
// Created by Valerio Santinelli on 2019-04-27.
//
// Based on work by Auerhaus Development, LLC
// https://github.com/warrenm/AHEasing
//

#ifndef DEMOMAKER_C_BINOCLE_EASING_H
#define DEMOMAKER_C_BINOCLE_EASING_H

// Linear interpolation (no easing)
float binocle_easing_linear_interpolation(float p);

// Quadratic easing; p^2
float binocle_easing_quadratic_ease_in(float p);
float binocle_easing_quadratic_ease_out(float p);
float binocle_easing_quadratic_ease_in_out(float p);

// Cubic easing; p^3
float binocle_easing_cubic_ease_in(float p);
float binocle_easing_cubic_ease_out(float p);
float binocle_easing_cubic_ease_in_out(float p);

// Quartic easing; p^4
float binocle_easing_quartic_ease_in(float p);
float binocle_easing_quartic_ease_out(float p);
float binocle_easing_quartic_ease_in_out(float p);

// Quintic easing; p^5
float binocle_easing_quintic_ease_in(float p);
float binocle_easing_quintic_ease_out(float p);
float binocle_easing_quintic_ease_in_out(float p);

// Sine wave easing; sin(p * PI/2)
float binocle_easing_sine_ease_in(float p);
float binocle_easing_sine_ease_out(float p);
float binocle_easing_sine_ease_in_out(float p);

// Circular easing; sqrt(1 - p^2)
float binocle_easing_circular_ease_in(float p);
float binocle_easing_circular_ease_out(float p);
float binocle_easing_circular_ease_in_out(float p);

// Exponential easing, base 2
float binocle_easing_exponential_ease_in(float p);
float binocle_easing_exponential_ease_out(float p);
float binocle_easing_exponential_ease_in_out(float p);

// Exponentially-damped sine wave easing
float binocle_easing_elastic_ease_in(float p);
float binocle_easing_elastic_ease_out(float p);
float binocle_easing_elastic_ease_in_out(float p);

// Overshooting cubic easing;
float binocle_easing_back_ease_in(float p);
float binocle_easing_back_ease_out(float p);
float binocle_easing_back_ease_in_out(float p);

// Exponentially-decaying bounce easing
float binocle_easing_bounce_ease_in(float p);
float binocle_easing_bounce_ease_out(float p);
float binocle_easing_bounce_ease_in_out(float p);

#endif //DEMOMAKER_C_BINOCLE_EASING_H
