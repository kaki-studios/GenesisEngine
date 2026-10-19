#include "collision.h"
#include "../cuboid/rigidbody.h"
#include "../rendering/cube_renderer.h"
#include "clipping.h"
#include "glm/geometric.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

namespace {

// Sphere-sphere broad phase on the bounding radii of the colliders' boxes.
bool BroadPhase(const glm::vec3 &p1, const glm::vec3 &h1, const glm::vec3 &p2,
                const glm::vec3 &h2) {
  float r1 = glm::length(h1);
  float r2 = glm::length(h2);
  return glm::length(p1 - p2) <= (r1 + r2);
}

} // namespace

std::vector<CollisionResult>
CollectCollisionPairsNew(std::set<ECS::Entity> entities,
                         ECS::Coordinator *coordinator) {
  std::vector<CollisionResult> collisions;

  for (auto i = entities.begin(); i != entities.end(); ++i) {
    auto j = i;
    ++j;
    for (; j != entities.end(); ++j) {
      ECS::Entity e1 = *i;
      ECS::Entity e2 = *j;

      auto &rb1 = coordinator->GetComponent<Rigidbody>(e1);
      auto &rb2 = coordinator->GetComponent<Rigidbody>(e2);
      if (rb1.invMass == 0.0f && rb2.invMass == 0.0f)
        continue; // two static bodies can never resolve

      auto &t1 = coordinator->GetComponent<Transform>(e1);
      auto &t2 = coordinator->GetComponent<Transform>(e2);
      auto &cube1 = coordinator->GetComponent<Cuboid>(e1);
      auto &cube2 = coordinator->GetComponent<Cuboid>(e2);
      if (!BroadPhase(t1.position, cube1.halfExtents, t2.position,
                      cube2.halfExtents))
        continue;

      auto &c1 = coordinator->GetComponent<Collider>(e1);
      auto &c2 = coordinator->GetComponent<Collider>(e2);
      // The collider stores world pose; refresh it every substep.
      c1.inner->position = t1.position;
      c1.inner->rotation = t1.rotation;
      c2.inner->position = t2.position;
      c2.inner->rotation = t2.rotation;

      GJKResult gjk = GJKIntersect(&c1, &c2);
      if (!gjk.colliding)
        continue;

      // epa.normal points from body A towards body B.
      CollisionResult epa = EPA(c1, c2, gjk.simplex);
      if (!epa.valid)
        continue;

      ContactManifold m = buildContactManifold(*c1.inner, *c2.inner, epa);
      if (m.points.empty())
        continue;

      glm::mat3 R1 = glm::mat3_cast(t1.rotation);
      glm::mat3 R2 = glm::mat3_cast(t2.rotation);
      float restitution = rb1.restitution * rb2.restitution;
      float friction = 0.5f * (rb1.friction + rb2.friction);

      for (auto &point : m.points) {
        // Local contact offsets, evaluated fresh at every solver pass.
        glm::vec3 localA = glm::transpose(R1) * (point.positionA - t1.position);
        glm::vec3 localB = glm::transpose(R2) * (point.positionB - t2.position);

        float penetration =
            glm::dot(point.positionA - point.positionB, m.normal);
        if (penetration <= 0.0f)
          continue;

        CollisionResult c;
        c.valid = true;
        c.bodyA = e1;
        c.bodyB = e2;
        c.normal = m.normal;
        c.contactA = localA;
        c.contactB = localB;
        c.penetration = penetration;
        c.restitutionCoeff = restitution;
        c.frictionCoeff = friction;
        collisions.push_back(c);
      }
    }
  }
  return collisions;
}
