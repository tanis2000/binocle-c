#pragma once

cpSpace *world;
cpBody *background_body;
cpBody *ball_body;
cpShape *ball_shape;
binocle_sprite *ball_sprite;
bool dragging_ball;
kmVec2 mouse_prev_pos;
cpBody *bottom_body;
cpShape *bottom_shape;

void create_barriers();
void create_ball();
void setup_world();
void destroy_world();
void advance_simulation(float dt);
