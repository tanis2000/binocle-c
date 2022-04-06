//#ifdef WITH_PHYSICS
#include <chipmunk/chipmunk.h>
#include "binocle_sprite.h"
#include "constants.h"
#include "binocle_log.h"
#include "physics.h"

physics_state_t physics_state;

void create_barriers() {
  physics_state.bottom_body = cpSpaceGetStaticBody(physics_state.world);
  cpBodySetPosition(physics_state.bottom_body, cpv(DESIGN_WIDTH/2, -50));
  physics_state.bottom_shape = cpBoxShapeNew(physics_state.bottom_body, DESIGN_WIDTH, 100, 0);
  cpSpaceAddShape(physics_state.world, physics_state.bottom_shape);
//  cpBodySetPosition(bottom_body, cpv(DESIGN_WIDTH/2, -50));
//  cpBodySetPosition(bottom_body, cpv(0, 0));
  cpShapeSetFriction(physics_state.bottom_shape, 1);

//  kmMat4 identity;
//  kmMat4Identity(&identity);
//
//  kmMat4 bottom_offset;
//  kmMat4Translation(&bottom_offset, DESIGN_WIDTH/2, -50, 0);
//  NewtonCollision *coll_bottom = NewtonCreateBox(world, DESIGN_WIDTH, 100, 100, 0, &identity.mat[0]);
//
//  kmMat4 top_offset;
//  kmMat4Translation(&top_offset, DESIGN_WIDTH/2, DESIGN_HEIGHT + 50, 0);
//  NewtonCollision *coll_top = NewtonCreateBox(world, DESIGN_WIDTH, 100, 100, 0, &identity.mat[0]);
//
//  kmMat4 left_offset;
//  kmMat4Translation(&left_offset, -50, DESIGN_HEIGHT/2, 0);
//  NewtonCollision *coll_left = NewtonCreateBox(world, 100, DESIGN_HEIGHT, 100, 0, &identity.mat[0]);
//
//  kmMat4 right_offset;
//  kmMat4Translation(&right_offset, DESIGN_WIDTH + 50, DESIGN_HEIGHT/2, 0);
//  NewtonCollision *coll_right = NewtonCreateBox(world, 100, DESIGN_HEIGHT, 100, 0, &identity.mat[0]);
//
//  NewtonCreateDynamicBody(world, coll_bottom, &bottom_offset.mat[0]);
//  NewtonCreateDynamicBody(world, coll_top, &top_offset.mat[0]);
//  NewtonCreateDynamicBody(world, coll_left, &left_offset.mat[0]);
//  NewtonCreateDynamicBody(world, coll_right, &right_offset.mat[0]);
//
//  NewtonDestroyCollision(coll_bottom);
//  NewtonDestroyCollision(coll_top);
//  NewtonDestroyCollision(coll_left);
//  NewtonDestroyCollision(coll_right);
}

//NewtonBody *create_background() {
//  dFloat points[4][3] =
//    {
//      {-100.0f, 0.0f,  100.0f},
//      { 420.0f, 0.0f,  100.0f},
//      { 420.0f, 0.0f, -100.0f},
//      {-100.0f, 0.0f, -100.0f},
//    };
//
//  // crate a collision tree
//  NewtonCollision* const collision = NewtonCreateTreeCollision (world, 0);
//
//  // start building the collision mesh
//  NewtonTreeCollisionBeginBuild (collision);
//
//  // add the face one at a time
//  NewtonTreeCollisionAddFace (collision, 4, &points[0][0], 3 * sizeof (dFloat), 0);
//
//  // finish building the collision
//  NewtonTreeCollisionEndBuild (collision, 1);
//
//  // create a body with a collision and locate at the identity matrix position
//  kmMat4 matrix;
//  kmMat4Identity(&matrix);
//  NewtonBody *body = NewtonCreateDynamicBody(world, collision, &matrix.mat[0]);
//
//  // do no forget to destroy the collision after you not longer need it
//  NewtonDestroyCollision(collision);
//  return body;
//}

//static void apply_gravity (const NewtonBody* const body, dFloat timestep, int threadIndex)
//{
//  // apply gravity force to the body
//  dFloat mass;
//  dFloat Ixx;
//  dFloat Iyy;
//  dFloat Izz;
//
//  NewtonBodyGetMass(body, &mass, &Ixx, &Iyy, &Izz);
//  float gravityForce[4] = {0, -9.8f * mass, 0.0f, 0.0f};
//  NewtonBodySetForce(body, &gravityForce[0]);
//  //float torque[4] = {0, -9.8f * mass, 0.0f, 0.0f};
//  //NewtonBodySetTorque(body, &torque[0]);
//}

void create_ball()
{
  physics_state.ball_body = cpBodyNew(1, cpMomentForBox(1, 16.0f, 16.0f));
  cpSpaceAddBody(physics_state.world, physics_state.ball_body);
  physics_state.ball_shape = cpBoxShapeNew(physics_state.ball_body, 16.0f, 16.0f, 0);
  cpSpaceAddShape(physics_state.world, physics_state.ball_shape);
  cpBodySetPosition(physics_state.ball_body, cpv(160, 50));
  cpShapeSetFriction(physics_state.ball_shape, 0.7f);
}

physics_state_t setup_world() {
  physics_state = (physics_state_t){0};
  cpVect gravity = cpv(0, -9.8f);
  physics_state.world = cpSpaceNew();
  cpSpaceSetGravity(physics_state.world, gravity);
  //background_body = create_background();
  create_barriers();
  create_ball();
  return physics_state;
}

void destroy_world() {
  cpShapeFree(physics_state.ball_shape);
  cpBodyFree(physics_state.ball_body);
  cpSpaceFree(physics_state.world);
}

void advance_simulation(float dt) {
  if (dt <= 0) return;

  cpVect pos = cpBodyGetPosition(physics_state.ball_body);
  cpVect vel = cpBodyGetVelocity(physics_state.ball_body);
  cpBB bb = cpShapeGetBB(physics_state.ball_shape);
  binocle_log_info("ball x:%5.2f y:%5.2f - velocity x:%5.2f y:%5.2f", pos.x, pos.y, vel.x, vel.y);
  binocle_log_info("bb t:%5.2f l:%5.2f r:%5.2f b:%5.2f", bb.t, bb.l, bb.r, bb.b);

//  pos = cpBodyGetPosition(bottom_body);
//  vel = cpBodyGetVelocity(bottom_body);
//  bb = cpShapeGetBB(bottom_shape);
//  binocle_log_info("coll x:%5.2f y:%5.2f - velocity x:%5.2f y:%5.2f", pos.x, pos.y, vel.x, vel.y);
//  binocle_log_info("bb coll t:%5.2f l:%5.2f r:%5.2f b:%5.2f", bb.t, bb.l, bb.r, bb.b);

  cpSpaceStep(physics_state.world, dt);
}
//#endif