#ifdef WITH_PHYSICS
#include <dgNewton/Newton.h>

NewtonWorld *world;
NewtonBody *background_body;
NewtonBody *ball_body;
binocle_sprite *ball_sprite;
bool dragging_ball;
kmVec2 mouse_prev_pos;

void create_barriers() {
  kmMat4 identity;
  kmMat4Identity(&identity);

  kmMat4 bottom_offset;
  kmMat4Translation(&bottom_offset, DESIGN_WIDTH/2, -50, 0);
  NewtonCollision *coll_bottom = NewtonCreateBox(world, DESIGN_WIDTH, 100, 100, 0, &identity.mat[0]);

  kmMat4 top_offset;
  kmMat4Translation(&top_offset, DESIGN_WIDTH/2, DESIGN_HEIGHT + 50, 0);
  NewtonCollision *coll_top = NewtonCreateBox(world, DESIGN_WIDTH, 100, 100, 0, &identity.mat[0]);

  kmMat4 left_offset;
  kmMat4Translation(&left_offset, -50, DESIGN_HEIGHT/2, 0);
  NewtonCollision *coll_left = NewtonCreateBox(world, 100, DESIGN_HEIGHT, 100, 0, &identity.mat[0]);

  kmMat4 right_offset;
  kmMat4Translation(&right_offset, DESIGN_WIDTH + 50, DESIGN_HEIGHT/2, 0);
  NewtonCollision *coll_right = NewtonCreateBox(world, 100, DESIGN_HEIGHT, 100, 0, &identity.mat[0]);

  NewtonCreateDynamicBody(world, coll_bottom, &bottom_offset.mat[0]);
  NewtonCreateDynamicBody(world, coll_top, &top_offset.mat[0]);
  NewtonCreateDynamicBody(world, coll_left, &left_offset.mat[0]);
  NewtonCreateDynamicBody(world, coll_right, &right_offset.mat[0]);

  NewtonDestroyCollision(coll_bottom);
  NewtonDestroyCollision(coll_top);
  NewtonDestroyCollision(coll_left);
  NewtonDestroyCollision(coll_right);
}

NewtonBody *create_background() {
  dFloat points[4][3] =
    {
      {-100.0f, 0.0f,  100.0f},
      { 420.0f, 0.0f,  100.0f},
      { 420.0f, 0.0f, -100.0f},
      {-100.0f, 0.0f, -100.0f},
    };

  // crate a collision tree
  NewtonCollision* const collision = NewtonCreateTreeCollision (world, 0);

  // start building the collision mesh
  NewtonTreeCollisionBeginBuild (collision);

  // add the face one at a time
  NewtonTreeCollisionAddFace (collision, 4, &points[0][0], 3 * sizeof (dFloat), 0);

  // finish building the collision
  NewtonTreeCollisionEndBuild (collision, 1);

  // create a body with a collision and locate at the identity matrix position
  kmMat4 matrix;
  kmMat4Identity(&matrix);
  NewtonBody *body = NewtonCreateDynamicBody(world, collision, &matrix.mat[0]);

  // do no forget to destroy the collision after you not longer need it
  NewtonDestroyCollision(collision);
  return body;
}

static void apply_gravity (const NewtonBody* const body, dFloat timestep, int threadIndex)
{
  // apply gravity force to the body
  dFloat mass;
  dFloat Ixx;
  dFloat Iyy;
  dFloat Izz;

  NewtonBodyGetMass(body, &mass, &Ixx, &Iyy, &Izz);
  float gravityForce[4] = {0, -9.8f * mass, 0.0f, 0.0f};
  NewtonBodySetForce(body, &gravityForce[0]);
  //float torque[4] = {0, -9.8f * mass, 0.0f, 0.0f};
  //NewtonBodySetTorque(body, &torque[0]);
}

NewtonBody* create_ball()
{
  kmMat4 offset;
  kmMat4Translation(&offset, 8, 8, 0);
  // crate a collision sphere
  NewtonCollision* const collision = NewtonCreateBox(world, 16.0f, 16.0f, 16.0f, 0, &offset.mat[0]);

  // create a dynamic body with a sphere shape, and
  kmMat4 matrix;
  kmMat4Identity(&matrix);
  kmMat4 trans;
  kmMat4Translation(&trans, 160, 50, 0);
  kmMat4Multiply(&matrix, &trans, &matrix);
  //matrix.m_posit.m_y = 50.0f;
  NewtonBody* const body = NewtonCreateDynamicBody(world, collision, &matrix.mat[0]);

  // set the force callback for applying the force and torque
  NewtonBodySetForceAndTorqueCallback(body, apply_gravity);

  // set the mass for this body
  dFloat mass = 1.0f;
  NewtonBodySetMassProperties(body, mass, collision);

  // set the linear damping to zero
  NewtonBodySetLinearDamping (body, 0.1f);

  // do no forget to destroy the collision after you not longer need it
  NewtonDestroyCollision(collision);
  return body;
}

void setup_world() {
  world = NewtonCreate();
  //background_body = create_background();
  create_barriers();
  ball_body = create_ball();
  NewtonInvalidateCache(world);
}

void destroy_world() {
  NewtonDestroyAllBodies(world);
  NewtonDestroy(world);
}

void advance_simulation(float dt) {
  if (dt <= 0) return;

  NewtonUpdate(world, dt);
  kmMat4 matrix;
  NewtonBodyGetMatrix(ball_body, matrix.mat);
  kmVec3 pos;
  kmMat4ExtractTranslationVec3(&matrix, &pos);
  binocle_log_info("ball x:%f y:%f z:%f", pos.x, pos.y, pos.z);
}
#endif