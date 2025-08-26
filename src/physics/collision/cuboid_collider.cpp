#include "cuboid_collider.h"
#include "physics/collision/collider.h"
#include <iostream>
#include <vector>

CuboidCollider::CuboidCollider(const glm::vec3 &hExt) : halfExtents(hExt) {
  type = ColliderType::CuboidColliderType;
}
CuboidCollider::CuboidCollider(glm::vec3 &&hExt) : halfExtents(hExt) {}

// maximizes dot product
glm::vec3 const CuboidCollider::support(const glm::vec3 direction) const {
  glm::vec3 localDir = glm::inverse(glm::normalize(rotation)) * direction;

  glm::vec3 localSupport =
      glm::vec3(localDir.x >= 0 ? halfExtents.x : -halfExtents.x,
                localDir.y >= 0 ? halfExtents.y : -halfExtents.y,
                localDir.z >= 0 ? halfExtents.z : -halfExtents.z);

  return position + rotation * localSupport;
}

int const CuboidCollider::getFaceCount() const { return 6; }

Face const CuboidCollider::getFace(int i) const {
  // ccw
  static const int faceIndices[6][4] = {
      {1, 5, 7, 3}, // +X
      {0, 2, 6, 4}, // -X
      {7, 3, 2, 6}, // +Y
      {5, 4, 0, 1}, // -Y
      {4, 5, 7, 6}, // +Z
      {0, 2, 3, 1}  // -Z
  };
  static const glm::vec3 normals[6] = {{1, 0, 0},  {-1, 0, 0}, {0, 1, 0},
                                       {0, -1, 0}, {0, 0, 1},  {0, 0, -1}};

  Face f;
  f.normal = rotation * normals[i];
  f.indices.assign(&faceIndices[i][0], &faceIndices[i][4]);
  return f;
}

int const CuboidCollider::getVertexCount() const { return 8; }

glm::vec3 const CuboidCollider::getVertex(int i) const {
  glm::vec3 local((i & 1 ? halfExtents.x : -halfExtents.x),
                  (i & 2 ? halfExtents.y : -halfExtents.y),
                  (i & 4 ? halfExtents.z : -halfExtents.z));
  return position + rotation * local;
}
