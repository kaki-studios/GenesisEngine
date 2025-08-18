#include "collision.h"
#include "../../rendering/cube_renderer.h"
#include "ecs/entity_manager.h"
#include "glm/common.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "glm/geometric.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/matrix.hpp"
#include "physics/collision/epa.h"
#include "rigidbody.h"
#include <ECS.h>
#include <cstdio>
#include <iostream>
#include <set>
#include <vector>

// super simple broadphase, TODO make AABB instead of sphere
bool BroadPhaseIsColliding(OBB o1, OBB o2) {
  float r1 = glm::length(o1.halfExtents);
  float r2 = glm::length(o2.halfExtents);

  float d = glm::length(o1.position - o2.position);

  return (r1 + r2) >= d;
}

// Helper function to get OBB axes (local coordinate system)
std::vector<glm::vec3> getOBBAxes(const OBB &obb) {
  glm::mat3 rotMatrix = glm::mat3_cast(obb.rotation);
  return {
      rotMatrix[0], // X axis
      rotMatrix[1], // Y axis
      rotMatrix[2]  // Z axis
  };
}

// Helper function to get OBB vertices
std::vector<glm::vec3> getOBBVertices(const OBB &obb) {
  std::vector<glm::vec3> vertices;
  glm::mat3 rotMatrix = glm::mat3_cast(obb.rotation);

  // Generate all 8 vertices of the OBB
  for (int i = 0; i < 8; ++i) {
    glm::vec3 vertex =
        glm::vec3((i & 1) ? obb.halfExtents.x : -obb.halfExtents.x,
                  (i & 2) ? obb.halfExtents.y : -obb.halfExtents.y,
                  (i & 4) ? obb.halfExtents.z : -obb.halfExtents.z);

    // Transform to world space
    vertex = rotMatrix * vertex + obb.position;
    vertices.push_back(vertex);
  }

  return vertices;
}

// Project OBB onto an axis and return min/max projection values
std::pair<float, float> projectOBB(const OBB &obb, const glm::vec3 &axis) {
  std::vector<glm::vec3> vertices = getOBBVertices(obb);

  float minProj = std::numeric_limits<float>::max();
  float maxProj = -std::numeric_limits<float>::max();

  for (const auto &vertex : vertices) {
    float projection = glm::dot(vertex, axis);
    minProj = std::min(minProj, projection);
    maxProj = std::max(maxProj, projection);
  }

  return {minProj, maxProj};
}

// Check if two projection intervals overlap and return overlap amount
float getOverlap(const std::pair<float, float> &proj1,
                 const std::pair<float, float> &proj2) {
  float overlap1 = proj1.second - proj2.first;
  float overlap2 = proj2.second - proj1.first;
  return std::min(overlap1, overlap2);
}

// Find the contact point between two OBBs
glm::vec3 findContactPoint(const OBB &o1, const OBB &o2,
                           const glm::vec3 &normal, float penetration) {
  // Get vertices of both OBBs
  std::vector<glm::vec3> vertices1 = getOBBVertices(o1);
  std::vector<glm::vec3> vertices2 = getOBBVertices(o2);

  // Find the vertex of o1 that penetrates deepest into o2
  float maxPenetration = -std::numeric_limits<float>::max();
  glm::vec3 contactPoint = o1.position;

  for (const auto &vertex : vertices1) {
    float distance = glm::dot(vertex - o2.position, normal);
    if (distance > maxPenetration) {
      maxPenetration = distance;
      contactPoint = vertex;
    }
  }

  // Adjust contact point to be on the surface
  contactPoint -= normal * (penetration * 0.5f);

  return contactPoint;
}

CollisionResult SAT(OBB o1, OBB o2) {
  CollisionResult info;
  info.penetration = 0.0f;
  info.normal = glm::vec3(0.0f);
  // info.contact = glm::vec3(0.0f);

  // if (glm::length(o1.rotation) != 1.0f) {
  //   std::printf("o1 rotation: %f\n", glm::length(o1.rotation));
  // }
  // if (glm::length(o2.rotation) != 1.0f) {
  //   std::printf("o2 rotation: %f\n", glm::length(o2.rotation));
  // }

  // Get all potential separating axes
  std::vector<glm::vec3> axes1 = getOBBAxes(o1);
  std::vector<glm::vec3> axes2 = getOBBAxes(o2);

  std::vector<glm::vec3> testAxes;

  // Add face normals from both OBBs
  testAxes.insert(testAxes.end(), axes1.begin(), axes1.end());
  testAxes.insert(testAxes.end(), axes2.begin(), axes2.end());

  // Add cross products of edges (15 possible combinations)
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      glm::vec3 crossAxis = glm::cross(axes1[i], axes2[j]);
      float length = glm::length(crossAxis);

      // Skip nearly parallel axes
      if (length > 1e-6f) {
        testAxes.push_back(crossAxis / length);
      }
    }
  }

  float minOverlap = std::numeric_limits<float>::max();
  glm::vec3 separatingAxis(0.0f);
  bool collision = true;

  // Test all axes for separation
  for (const auto &axis : testAxes) {
    glm::vec3 normalizedAxis = glm::normalize(axis);

    // Project both OBBs onto the axis
    auto proj1 = projectOBB(o1, normalizedAxis);
    auto proj2 = projectOBB(o2, normalizedAxis);

    // Check for separation
    if (proj1.second < proj2.first || proj2.second < proj1.first) {
      collision = false;
      break;
    }

    // Calculate overlap
    float overlap = getOverlap(proj1, proj2);

    // if (overlap > 100.0f) {
    //   std::cout << "something wrong" << std::endl;
    // }
    if (overlap < minOverlap) {
      minOverlap = overlap;
      separatingAxis = normalizedAxis;

      // Ensure normal points from o1 to o2
      glm::vec3 centerDiff = o2.position - o1.position;
      if (glm::dot(separatingAxis, centerDiff) < 0) {
        separatingAxis = -separatingAxis;
      }
    }
  }

  if (collision && minOverlap != std::numeric_limits<float>::max()) {
    info.penetration = minOverlap * 0.5f;
    info.normal = separatingAxis;
    // info.contact = findContactPoint(o1, o2, separatingAxis, minOverlap);
  }

  return info;
}

// inverse of stiffness and has units meters/Newton
const float COLLISION_COMPLIANCE = 1e-6;

void SolvePositions(CollisionResult collisionInfo,
                    ECS::Coordinator *coordinator, float h) {
  ECS::Entity e1 = collisionInfo.bodyA;
  ECS::Entity e2 = collisionInfo.bodyB;
  auto &rb1 = coordinator->GetComponent<Rigidbody>(e1);
  auto &rb2 = coordinator->GetComponent<Rigidbody>(e2);
  auto &t1 = coordinator->GetComponent<Transform>(e1);
  auto &t2 = coordinator->GetComponent<Transform>(e2);
  if (rb1.invMass == 0 && rb2.invMass == 0) {
    return;
  }

  glm::vec3 r1 = collisionInfo.contactA - t1.position;

  // std::cout << "Contact Point A: " << collisionInfo.contactA.x << ", "
  //           << collisionInfo.contactA.y << ", " << collisionInfo.contactA.z
  //           << "\n";
  glm::vec3 r2 = collisionInfo.contactB - t2.position;
  //
  // std::cout << "Contact Point B: " << collisionInfo.contactB.x << ", "
  //           << collisionInfo.contactB.y << ", " << collisionInfo.contactB.z
  //           << "\n";

  // glm::vec3 r1 = collisionInfo.contactA;
  // glm::vec3 r2 = collisionInfo.contactB;

  // std::cout << "r1: " << r1.x << ", " << r1.y << ", " << r1.z << "\n";
  // std::cout << "r2: " << r2.x << ", " << r2.y << ", " << r2.z << "\n";

  // generalized inverse masses
  glm::mat3 R1 = glm::mat3_cast(t1.rotation);
  glm::mat3 globalInvInertia1 = R1 * rb1.invInertia * glm::transpose(R1);

  float gim1 = rb1.invMass + glm::dot(glm::cross(r1, collisionInfo.normal),
                                      globalInvInertia1 *
                                          glm::cross(r1, collisionInfo.normal));

  glm::mat3 R2 = glm::mat3_cast(t2.rotation);
  glm::mat3 globalInvInertia2 = R2 * rb2.invInertia * glm::transpose(R2);

  float gim2 = rb2.invMass + glm::dot(glm::cross(r2, collisionInfo.normal),
                                      globalInvInertia2 *
                                          glm::cross(r2, collisionInfo.normal));

  if (gim1 == 0 && gim2 == 0) {
    std::cout << "total inverse mass == 0\n";
    return;
  }
  // lagrangeMultiplier updates
  float aHat = COLLISION_COMPLIANCE / (h * h);
  float dl =
      (-collisionInfo.penetration - aHat * collisionInfo.lagrangeMultiplier) /
      (gim1 + gim2 + aHat);
  // std::cout << "dl: " << dl << "\n";
  collisionInfo.lagrangeMultiplier += dl;
  // std::cout << "lambda" << collisionInfo.lagrangeMultiplier << "\n";

  glm::vec3 positionalImpulse = dl * collisionInfo.normal;

  t1.position += positionalImpulse * rb1.invMass;
  t2.position -= positionalImpulse * rb2.invMass;

  t1.rotation +=
      0.5f *
      glm::quat(0.0, globalInvInertia1 * glm::cross(r1, positionalImpulse)) *
      t1.rotation;
  t2.rotation -=
      0.5f *
      glm::quat(0.0, globalInvInertia2 * glm::cross(r2, positionalImpulse)) *
      t2.rotation;

  // idk what to do with this (maybe store??)
  glm::vec3 collisionForce =
      collisionInfo.lagrangeMultiplier * (collisionInfo.normal / (h * h));
  // std::cout << "collisionforce magnitude: " << glm::length(collisionForce)
  //           << " and penetration: " << collisionInfo.penetration <<
  //           std::endl;
}

void SolveVelocities(CollisionResult collisionInfo,
                     ECS::Coordinator *coordinator, float h) {
  auto &rb1 = coordinator->GetComponent<Rigidbody>(collisionInfo.bodyA);
  auto &t1 = coordinator->GetComponent<Transform>(collisionInfo.bodyA);
  auto &rb2 = coordinator->GetComponent<Rigidbody>(collisionInfo.bodyB);
  auto &t2 = coordinator->GetComponent<Transform>(collisionInfo.bodyB);

  if (rb1.invMass == 0 && rb2.invMass == 0) {
    return;
  }

  glm::vec3 r1 = collisionInfo.contactA - t1.position;
  glm::vec3 r2 = collisionInfo.contactB - t2.position;

  glm::vec3 v = (rb1.linearVelocity + glm::cross(rb1.angularVelocity, r1)) -
                (rb2.linearVelocity + glm::cross(rb2.angularVelocity, r2));
  float vn = glm::dot(collisionInfo.normal, v);
  glm::vec3 vt = v - collisionInfo.normal * vn;

  if (glm::length(vt) < 1e-6f) {
    std::cout << "really small tangential velocity" << std::endl;
    return;
  }
  glm::vec3 frictionDirection = -glm::normalize(vt);

  float friction = (rb1.friction + rb2.friction) / 2;
  glm::vec3 fn =
      (collisionInfo.lagrangeMultiplier * collisionInfo.normal) / (h * h);

  glm::vec3 deltaV = -(glm::normalize(vt)) *
                     glm::min(h * friction * glm::length(fn), glm::length(vt));
  // maybe apply damping

  glm::mat3 R1 = glm::mat3_cast(t1.rotation);
  glm::mat3 globalInvInertia1 = R1 * rb1.invInertia * glm::transpose(R1);
  if (glm::length(deltaV) < 1e-5f) {
    return;
  }

  glm::vec3 dir = glm::normalize(deltaV);

  float gim1 = rb1.invMass + glm::dot(glm::cross(r1, dir),
                                      globalInvInertia1 * glm::cross(r1, dir));

  glm::mat3 R2 = glm::mat3_cast(t2.rotation);
  glm::mat3 globalInvInertia2 = R2 * rb2.invInertia * glm::transpose(R2);

  float gim2 = rb2.invMass + glm::dot(glm::cross(r2, dir),
                                      globalInvInertia2 * glm::cross(r2, dir));
  if (gim1 == 0 && gim2 == 0) {
    return;
  }

  glm::vec3 p = deltaV / (gim1 + gim2);

  rb1.linearVelocity += p * rb1.invMass;
  rb2.linearVelocity -= p * rb2.invMass;

  rb1.angularVelocity += globalInvInertia1 * glm::cross(r1, p);
  rb2.angularVelocity -= globalInvInertia2 * glm::cross(r2, p);
}
