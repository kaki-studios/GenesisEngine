#pragma once
#include "collider.h"
#include "epa.h"
#include <glm/vec3.hpp>
#include <vector>

struct ContactPoint {
  glm::vec3 positionA; // contact point on body A, world space
  glm::vec3 positionB; // contact point on body B, world space
};

struct ContactManifold {
  // Unit vector pointing from body A towards body B. For every point
  // (positionA - positionB) . normal >= 0 while penetrating.
  glm::vec3 normal;
  std::vector<ContactPoint> points; // up to 4
};

// A and B are passed in the same order as they were fed to GJK/EPA, so
// epa.normal points A -> B.
ContactManifold buildContactManifold(const ICollider &A, const ICollider &B,
                                     const CollisionResult &epa);
