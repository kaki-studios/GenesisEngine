#include "rigidbody.h"
#include "../../rendering/cube_renderer.h"
#include "../collision/collision.h"
#include "collision.h"
#include "glm/geometric.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/matrix.hpp"
#include <vector>

glm::mat3 ComputeCuboidInvInertia(glm::vec3 dims, float density) {
  float volume = dims.x * dims.y * dims.z;
  float x_sqr = dims.x * dims.x;
  float y_sqr = dims.y * dims.y;
  float z_sqr = dims.z * dims.z;
  float mass = volume * density;

  glm::mat3 invInertia(0.0f);
  invInertia[0][0] = 12.0f / (mass * (y_sqr + z_sqr));
  invInertia[1][1] = 12.0f / (mass * (x_sqr + z_sqr));
  invInertia[2][2] = 12.0f / (mass * (x_sqr + y_sqr));
  return invInertia;
}

Rigidbody CreateSB() {
  Rigidbody rb{};
  rb.density = 1.0f;
  rb.invMass = 0.0f; // infinite mass
  rb.prevRotation = glm::identity<glm::quat>();
  rb.invInertia = glm::mat3(0.0f);
  rb.restitution = 0.5f;
  rb.friction = 0.5f;
  return rb;
}

Rigidbody CreateCuboidRB(glm::vec3 halfExtents, float density) {
  glm::vec3 dims = 2.0f * halfExtents;

  Rigidbody rb{};
  rb.density = density;
  rb.invMass = 1.0f / (dims.x * dims.y * dims.z * density);
  rb.prevRotation = glm::identity<glm::quat>();
  rb.invInertia = ComputeCuboidInvInertia(dims, density);
  rb.restitution = 0.5f;
  rb.friction = 0.5f;
  return rb;
}

void RigidbodySystem::Init(App *app) {
  this->app = app;
  this->debug = app->coordinator.RegisterSystem<DebugCollisions>();
  debug->Init(&app->coordinator);

  ECS::Signature signature;
  signature.set(app->coordinator.GetComponentType<Transform>(), true);
  signature.set(app->coordinator.GetComponentType<Rigidbody>(), true);
  signature.set(app->coordinator.GetComponentType<Cuboid>(), true);
  signature.set(app->coordinator.GetComponentType<Collider>(), true);
  app->coordinator.SetSystemSignature<RigidbodySystem>(signature);
}

// Paper: 20 substeps with a single position iteration each is the most
// accurate use of a fixed time budget (Müller et al. 2020, §3.1).
static const int NUM_SUBSTEPS = 20;
static const int NUM_POS_ITERS = 1;
// The paper runs the velocity pass once. A single Gauss-Seidel sweep cannot
// cancel the normal velocity across several contacts of the same pair (deep
// or resting overlaps), which leaves a large separating velocity behind and
// launches the body. A few sweeps make the pass converge to its fixed point:
// vn = min(e*vnOld, 0) at every contact.
static const int NUM_VEL_ITERS = 4;

void RigidbodySystem::Update(double dt) {
  debug->ClearCollisions();

  for (auto &entity : mEntities)
    app->coordinator.GetComponent<Transform>(entity).rotation =
        glm::normalize(app->coordinator.GetComponent<Transform>(entity).rotation);

  const float h = float(dt / NUM_SUBSTEPS);

  for (int substep = 0; substep < NUM_SUBSTEPS; ++substep) {
    // ---- 1. integrate (Algorithm 2) --------------------------------------
    for (auto &entity : mEntities) {
      auto &t = app->coordinator.GetComponent<Transform>(entity);
      auto &rb = app->coordinator.GetComponent<Rigidbody>(entity);
      if (rb.invMass == 0.0f) {
        continue; // static body: never moves
      }

      rb.prevPosition = t.position;
      rb.prevRotation = t.rotation;

      rb.linearVelocity += rb.extForce * rb.invMass * h;
      t.position += rb.linearVelocity * h;

      glm::mat3 R = glm::mat3_cast(t.rotation);
      glm::mat3 invIWorld = R * rb.invInertia * glm::transpose(R);
      glm::mat3 inertiaWorld = glm::inverse(invIWorld);

      // w <- w + h * I^-1 (tau_ext - w x (I w))
      rb.angularVelocity +=
          h * invIWorld *
          (rb.extTorque - glm::cross(rb.angularVelocity,
                                     inertiaWorld * rb.angularVelocity));

      // q <- q + h/2 * [w, 0] q
      t.rotation += 0.5f * h * glm::quat(0.0f, rb.angularVelocity) * t.rotation;
      t.rotation = glm::normalize(t.rotation);

      // Velocities used to advance the state this substep. The restitution
      // term needs them *before* the PBD velocity update (paper §3.6).
      rb.prevLinearVelocity = rb.linearVelocity;
      rb.prevAngularVelocity = rb.angularVelocity;
    }

    // ---- 2. collision detection, per substep (paper §3.5) ----------------
    std::vector<CollisionResult> collisions =
        CollectCollisionPairsNew(mEntities, &app->coordinator);

    // ---- 3. position solve ----------------------------------------------
    for (int iter = 0; iter < NUM_POS_ITERS; ++iter) {
      for (auto &collision : collisions) {
        SolvePositions(collision, &app->coordinator, h);
      }
    }

    // ---- 4. derive velocities from the position change (paper §3.2) -----
    for (auto &entity : mEntities) {
      auto &t = app->coordinator.GetComponent<Transform>(entity);
      auto &rb = app->coordinator.GetComponent<Rigidbody>(entity);
      if (rb.invMass == 0.0f) {
        rb.linearVelocity = glm::vec3(0.0f);
        rb.angularVelocity = glm::vec3(0.0f);
        continue;
      }

      rb.linearVelocity = (t.position - rb.prevPosition) / h;

      glm::quat dq = t.rotation * glm::inverse(rb.prevRotation);
      if (dq.w < 0.0f) {
        dq = -dq;
      }
      rb.angularVelocity = 2.0f * glm::vec3(dq.x, dq.y, dq.z) / h;
    }

    // ---- 5. velocity solve: friction + restitution (paper §3.6) ----------
    for (int iter = 0; iter < NUM_VEL_ITERS; ++iter) {
      for (auto &collision : collisions) {
        SolveVelocities(collision, &app->coordinator, h);
      }
    }

    debug->SetCollisions(collisions, substep);
  }
}
