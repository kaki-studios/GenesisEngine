#include "collider.h"
#include "epa.h"
#include "glm/geometric.hpp"
#include <glm/common.hpp>
#include <glm/vec3.hpp>

// NOTE: dot(n,x)=d, where x is a point on the plane
struct Plane {
  glm::vec3 normal;
  float d;
  inline float signedDistance(const glm::vec3 &x) const {
    return glm::dot(normal, x) + d;
  }
};

struct ContactPoint {
  glm::vec3 positionA;
  glm::vec3 positionB;
};

struct ContactManifold {
  glm::vec3 normal; // incident -> reference (same direction as EPA normal after
                    // choosing reference)
  std::vector<ContactPoint> points; // up to 4
};

ContactManifold buildContactManifold(const ICollider &A, const ICollider &B,
                                     const CollisionResult &epa);
