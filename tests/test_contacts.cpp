// Headless checks for the XPBD contact solver.
//
// Build from the repo root:
//   clang++ -std=c++17 -O1 -I<path-to-glm> -Isrc -Isrc/include -Isrc/ecs \
//     tests/test_contacts.cpp src/physics/collision/gjk.cpp \
//     src/physics/collision/epa.cpp src/physics/collision/clipping.cpp \
//     src/physics/collision/collision.cpp \
//     src/physics/collision/cuboid_collider.cpp \
//     src/physics/cuboid/collision.cpp src/ecs/coordinator.cpp \
//     src/ecs/entity_manager.cpp src/ecs/system_manager.cpp -o test_contacts
//
// Covered: resting contact, resting rotated contact, deep initial overlap,
// contact normal orientation, two-box stacking and Coulomb friction.
#include "../../src/physics/collision/collision.h"
#include "../../src/physics/collision/cuboid_collider.h"
#include "../../src/physics/cuboid/collision.h"
#include "../../src/physics/cuboid/rigidbody.h"
#include "../../src/ecs/coordinator.h"
#include "../../src/rendering/cube_renderer.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>

// Same body factories as rigidbody.cpp, duplicated here to avoid linking the
// whole app/render stack into a headless test.
Rigidbody CreateSBTest() {
  Rigidbody rb{};
  rb.invMass = 0.0f;
  rb.prevRotation = glm::identity<glm::quat>();
  rb.invInertia = glm::mat3(0.0f);
  rb.friction = 0.5f;
  rb.restitution = 0.5f;
  return rb;
}

Rigidbody CreateCuboidRBTest(glm::vec3 halfExtents, float density) {
  glm::vec3 d = 2.0f * halfExtents;
  Rigidbody rb{};
  rb.density = density;
  rb.invMass = 1.0f / (d.x * d.y * d.z * density);
  rb.prevRotation = glm::identity<glm::quat>();
  float m = d.x * d.y * d.z * density;
  rb.invInertia = glm::mat3(0.0f);
  rb.invInertia[0][0] = 12.0f / (m * (d.y * d.y + d.z * d.z));
  rb.invInertia[1][1] = 12.0f / (m * (d.x * d.x + d.z * d.z));
  rb.invInertia[2][2] = 12.0f / (m * (d.x * d.x + d.y * d.y));
  rb.friction = 0.5f;
  rb.restitution = 0.5f;
  return rb;
}

namespace {

struct World {
  ECS::Coordinator coord;
  ECS::Entity ground;
  ECS::Entity box;
};

// One simulate step: mirrors RigidbodySystem::Update for two bodies.
void Step(World &w, float dt, int substeps = 20) {
  auto &groundT = w.coord.GetComponent<Transform>(w.ground);
  auto &groundRb = w.coord.GetComponent<Rigidbody>(w.ground);
  auto &boxT = w.coord.GetComponent<Transform>(w.box);
  auto &boxRb = w.coord.GetComponent<Rigidbody>(w.box);
  (void)groundRb;
  (void)groundT;

  const float h = dt / substeps;
  for (int s = 0; s < substeps; ++s) {
    // integrate
    for (ECS::Entity e : {w.ground, w.box}) {
      auto &t = w.coord.GetComponent<Transform>(e);
      auto &rb = w.coord.GetComponent<Rigidbody>(e);
      if (rb.invMass == 0.0f)
        continue;
      rb.prevPosition = t.position;
      rb.prevRotation = t.rotation;
      rb.linearVelocity += rb.extForce * rb.invMass * h;
      t.position += rb.linearVelocity * h;
      glm::mat3 R = glm::mat3_cast(t.rotation);
      glm::mat3 invIw = R * rb.invInertia * glm::transpose(R);
      glm::mat3 Iw = glm::inverse(invIw);
      rb.angularVelocity +=
          h * invIw *
          (rb.extTorque -
           glm::cross(rb.angularVelocity, Iw * rb.angularVelocity));
      t.rotation += 0.5f * h * glm::quat(0.0f, rb.angularVelocity) * t.rotation;
      t.rotation = glm::normalize(t.rotation);
      rb.prevLinearVelocity = rb.linearVelocity;
      rb.prevAngularVelocity = rb.angularVelocity;
    }

    std::set<ECS::Entity> ents{w.ground, w.box};
    auto collisions = CollectCollisionPairsNew(ents, &w.coord);
    for (auto &c : collisions)
      SolvePositions(c, &w.coord, h);

    for (ECS::Entity e : {w.ground, w.box}) {
      auto &t = w.coord.GetComponent<Transform>(e);
      auto &rb = w.coord.GetComponent<Rigidbody>(e);
      if (rb.invMass == 0.0f)
        continue;
      rb.linearVelocity = (t.position - rb.prevPosition) / h;
      glm::quat dq = t.rotation * glm::inverse(rb.prevRotation);
      if (dq.w < 0.0f)
        dq = -dq;
      rb.angularVelocity = 2.0f * glm::vec3(dq.x, dq.y, dq.z) / h;
    }

    for (int it = 0; it < 4; ++it)
      for (auto &c : collisions)
        SolveVelocities(c, &w.coord, h);
  }
}

World MakeWorld(glm::vec3 boxPos, glm::quat boxRot = glm::identity<glm::quat>(),
                glm::vec3 boxHalf = glm::vec3(1.0f)) {
  World w;
  w.coord.Init();
  w.coord.RegisterComponent<Transform>();
  w.coord.RegisterComponent<Rigidbody>();
  w.coord.RegisterComponent<Cuboid>();
  w.coord.RegisterComponent<Collider>();

  // ground: static box, top face at y = 0
  w.ground = w.coord.CreateEntity();
  w.coord.AddComponent(w.ground,
                       Transform{glm::vec3(0.0f, -0.5f, 0.0f),
                                 glm::identity<glm::quat>()});
  w.coord.AddComponent(w.ground, CreateSBTest());
  w.coord.AddComponent(w.ground,
                       Cuboid{glm::vec3(20.0f, 0.5f, 20.0f), glm::vec3(0)});
  w.coord.AddComponent(
      w.ground, Collider(CuboidCollider(glm::vec3(20.0f, 0.5f, 20.0f))));

  w.box = w.coord.CreateEntity();
  w.coord.AddComponent(w.box, Transform{boxPos, boxRot});
  auto rb = CreateCuboidRBTest(boxHalf, 1.0f);
  rb.extForce = glm::vec3(0.0f, -9.81f / rb.invMass, 0.0f);
  w.coord.AddComponent(w.box, rb);
  w.coord.AddComponent(w.box, Cuboid{boxHalf, glm::vec3(1)});
  w.coord.AddComponent(w.box, Collider(CuboidCollider(boxHalf)));
  return w;
}

// Deepest penetration of the box bottom below the ground top plane (y=0).
float SinkDepth(World &w, glm::vec3 boxHalf) {
  auto &t = w.coord.GetComponent<Transform>(w.box);
  glm::mat3 R = glm::mat3_cast(t.rotation);
  float lowest = 1e9f;
  for (int i = 0; i < 8; ++i) {
    glm::vec3 l((i & 1 ? boxHalf.x : -boxHalf.x),
                (i & 2 ? boxHalf.y : -boxHalf.y),
                (i & 4 ? boxHalf.z : -boxHalf.z));
    lowest = std::min(lowest, (t.position + R * l).y);
  }
  return -lowest; // > 0 means sunk below y=0
}

// Three-body version of Step.
void Step3(World &w, ECS::Entity a, ECS::Entity b, ECS::Entity c, float dt,
           int frames, int substeps = 20) {
  std::vector<ECS::Entity> es{a, b, c};
  const float h = dt / substeps;
  for (int f = 0; f < frames; ++f) {
    for (int s = 0; s < substeps; ++s) {
      for (ECS::Entity e : es) {
        auto &t = w.coord.GetComponent<Transform>(e);
        auto &rb = w.coord.GetComponent<Rigidbody>(e);
        if (rb.invMass == 0.0f)
          continue;
        rb.prevPosition = t.position;
        rb.prevRotation = t.rotation;
        rb.linearVelocity += rb.extForce * rb.invMass * h;
        t.position += rb.linearVelocity * h;
        glm::mat3 R = glm::mat3_cast(t.rotation);
        glm::mat3 invIw = R * rb.invInertia * glm::transpose(R);
        glm::mat3 Iw = glm::inverse(invIw);
        rb.angularVelocity +=
            h * invIw *
            (rb.extTorque -
             glm::cross(rb.angularVelocity, Iw * rb.angularVelocity));
        t.rotation +=
            0.5f * h * glm::quat(0.0f, rb.angularVelocity) * t.rotation;
        t.rotation = glm::normalize(t.rotation);
        rb.prevLinearVelocity = rb.linearVelocity;
        rb.prevAngularVelocity = rb.angularVelocity;
      }
      std::set<ECS::Entity> ents{es.begin(), es.end()};
      auto collisions = CollectCollisionPairsNew(ents, &w.coord);
      for (auto &col : collisions)
        SolvePositions(col, &w.coord, h);
      for (ECS::Entity e : es) {
        auto &t = w.coord.GetComponent<Transform>(e);
        auto &rb = w.coord.GetComponent<Rigidbody>(e);
        if (rb.invMass == 0.0f)
          continue;
        rb.linearVelocity = (t.position - rb.prevPosition) / h;
        glm::quat dq = t.rotation * glm::inverse(rb.prevRotation);
        if (dq.w < 0.0f)
          dq = -dq;
        rb.angularVelocity = 2.0f * glm::vec3(dq.x, dq.y, dq.z) / h;
      }
      for (int it = 0; it < 4; ++it)
        for (auto &col : collisions)
          SolveVelocities(col, &w.coord, h);
    }
  }
}

} // namespace

int main() {
  // 1. Dropped box must come to rest on the ground, not sink through it.
  {
    World w = MakeWorld(glm::vec3(0.0f, 3.0f, 0.0f));
    for (int i = 0; i < 240; ++i)
      Step(w, 1.0f / 60.0f);
    float sink = SinkDepth(w, glm::vec3(1.0f));
    auto &t = w.coord.GetComponent<Transform>(w.box);
    auto &rb = w.coord.GetComponent<Rigidbody>(w.box);
    printf("[drop]      y=%.4f sink=%.4f |v|=%.5f\n", t.position.y, sink,
           glm::length(rb.linearVelocity));
    assert(std::abs(t.position.y - 1.0f) < 0.1f && "box did not rest at y=1");
    assert(sink < 0.05f && "box sank into the ground");
    assert(glm::length(rb.linearVelocity) < 0.1f && "box never settled");
  }

  // 2. Rotated box must be pushed out, not dragged sideways into the ground.
  {
    glm::quat rot = glm::angleAxis(glm::radians(40.0f), glm::vec3(0, 0, 1));
    World w = MakeWorld(glm::vec3(0.0f, 2.0f, 0.0f), rot);
    for (int i = 0; i < 240; ++i)
      Step(w, 1.0f / 60.0f);
    float sink = SinkDepth(w, glm::vec3(1.0f));
    auto &t = w.coord.GetComponent<Transform>(w.box);
    printf("[rotated]   y=%.4f sink=%.4f\n", t.position.y, sink);
    assert(sink < 0.05f && "rotated box tunneled into the ground");
  }

  // 3. Box starting already overlapped must be pushed apart, never amplify.
  {
    World w = MakeWorld(glm::vec3(0.0f, 0.6f, 0.0f));
    float before = SinkDepth(w, glm::vec3(1.0f));
    Step(w, 1.0f / 60.0f);
    float after = SinkDepth(w, glm::vec3(1.0f));
    printf("[overlap]   sink %.4f -> %.4f\n", before, after);
    assert(after <= before + 1e-3f && "penetration increased: solver sign bug");

    // And it must settle at rest instead of being launched.
    for (int i = 0; i < 240; ++i)
      Step(w, 1.0f / 60.0f);
    auto &t = w.coord.GetComponent<Transform>(w.box);
    auto &rb = w.coord.GetComponent<Rigidbody>(w.box);
    float rest = SinkDepth(w, glm::vec3(1.0f));
    printf("[overlap2]  y=%.4f sink=%.4f |v|=%.5f\n", t.position.y, rest,
           glm::length(rb.linearVelocity));
    printf("   resty=%.4f\n", t.position.y); assert(std::abs(t.position.y - 1.0f) < 0.05f && "overlap case never settled");
    assert(glm::length(rb.linearVelocity) < 0.1f && "energy injected by solver");
  }

  // 4. Contact normal must point from A (ground) towards B (box).
  {
    World w = MakeWorld(glm::vec3(0.0f, 0.9f, 0.0f));
    std::set<ECS::Entity> ents{w.ground, w.box};
    auto cols = CollectCollisionPairsNew(ents, &w.coord);
    printf("[manifold]  contacts=%zu\n", cols.size());
    assert(!cols.empty() && "no contacts found for overlapping box");
    for (auto &c : cols) {
      printf("            n=(%.2f,%.2f,%.2f) pen=%.4f\n", c.normal.x,
             c.normal.y, c.normal.z, c.penetration);
      assert(c.normal.y > 0.9f && "normal does not point A->B (upwards)");
      assert(c.penetration > 0.0f && "non-positive penetration");
      assert(c.bodyA == w.ground && c.bodyB == w.box);
    }
  }

  // 5. A stack must not sink or drift apart (sequential Gauss-Seidel).
  {
    World w = MakeWorld(glm::vec3(0.0f, 1.0f, 0.0f));
    // second box on top
    ECS::Entity top = w.coord.CreateEntity();
    glm::vec3 he(1.0f);
    w.coord.AddComponent(top, Transform{glm::vec3(0.0f, 3.0f, 0.0f),
                                        glm::identity<glm::quat>()});
    auto rb = CreateCuboidRBTest(he, 1.0f);
    rb.extForce = glm::vec3(0.0f, -9.81f / rb.invMass, 0.0f);
    w.coord.AddComponent(top, rb);
    w.coord.AddComponent(top, Cuboid{he, glm::vec3(1)});
    w.coord.AddComponent(top, Collider(CuboidCollider(he)));

    Step3(w, w.ground, w.box, top, 1.0f / 60.0f, 400);
    auto &tb = w.coord.GetComponent<Transform>(w.box);
    auto &tt = w.coord.GetComponent<Transform>(top);
    printf("[stack]     lower=%.4f upper=%.4f\n", tb.position.y, tt.position.y);
    assert(tb.position.y > 0.95f && "lower box sank");
    assert(tt.position.y > tb.position.y + 1.9f && "upper box sank into lower");
  }

  // 6. Friction must stop a box sliding down a slope-free floor with push.
  {
    World w = MakeWorld(glm::vec3(0.0f, 1.0f, 0.0f));
    auto &rb = w.coord.GetComponent<Rigidbody>(w.box);
    rb.linearVelocity = glm::vec3(5.0f, 0.0f, 0.0f);
    for (int i = 0; i < 180; ++i)
      Step(w, 1.0f / 60.0f);
    auto &t = w.coord.GetComponent<Transform>(w.box);
    printf("[friction]  x=%.3f vx=%.3f\n", t.position.x, rb.linearVelocity.x);
    assert(t.position.x < 5.0f && "friction did not slow the box down");
  }

  printf("all contact solver checks passed\n");
  return 0;
}
