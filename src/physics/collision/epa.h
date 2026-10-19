#pragma once
#include "../ecs/entity_manager.h"
#include "gjk.h"
#include <glm/common.hpp>
#include <glm/ext.hpp>

// A single contact between two bodies.
//
// Conventions (must stay consistent across clipping/solver, see
// Müller et al. 2020, "Detailed Rigid Body Simulation with XPBD", §3.5):
//   normal      unit vector pointing from body A towards body B
//   penetration (pA - pB) . normal, positive while the bodies overlap
//   contactA/B  contact offsets in the bodies' local frames (relative to the
//               centre of mass), so they can be re-evaluated every substep
struct CollisionResult {
  bool valid = false;
  ECS::Entity bodyA = 0;
  ECS::Entity bodyB = 0;
  glm::vec3 normal = glm::vec3(0.0f);
  float penetration = 0.0f;
  glm::vec3 contactA = glm::vec3(0.0f);
  glm::vec3 contactB = glm::vec3(0.0f);
  // XPBD multipliers, zeroed at the start of every substep (paper §3.5)
  float lagrangeN = 0.0f;
  float lagrangeT = 0.0f;
  float restitutionCoeff = 0.0f;
  float frictionCoeff = 0.0f;
};

struct EPAFace {
  int vertices[3];
  glm::vec3 normal; // pointing outward
  float distance;   // distance from origin
  EPAFace(int a, int b, int c) {
    vertices[0] = a;
    vertices[1] = b;
    vertices[2] = c;
    // calculate later
    normal = glm::vec3(0);
    distance = 0;
  }
};

// Minimum translation vector of the Minkowski difference A - B.
// normal is unit and points from A towards B, penetration >= 0.
CollisionResult EPA(const Collider &a, const Collider &b,
                    const Simplex &simplex);
