#include "../ecs/coordinator.h"
#include "../ecs/entity_manager.h"
#include "../include/ECS.h"
#include "glm/ext/quaternion_float.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <vector>

// Oriented bounding box
#pragma once
struct OBB {
  glm::vec3 position;
  glm::quat rotation;
  glm::vec3 halfExtents;

  OBB(glm::vec3 pos, glm::quat rot, glm::vec3 hExts)
      : position(pos), rotation(rot), halfExtents(hExts) {}
};

struct CollisionInfo {
  ECS::Entity bodyA;
  ECS::Entity bodyB;

  glm::vec3 contact;
  glm::vec3 normal;
  float penetration;

  float lagrangeMultiplier; // lambda
  //
  glm::vec3 p1;
  glm::vec3 p2;

  glm::vec3 p1hat;
  glm::vec3 p2hat;
};

CollisionInfo SAT(OBB o1, OBB o2);

// caller must ensure that all entites have the following components:
// Transform
// Rigidbody
// Cuboid
std::vector<CollisionInfo> CollectCollisionPairs(std::set<ECS::Entity> entities,
                                                 ECS::Coordinator *coordinator);
void SolvePositions(CollisionInfo collisionInfo, ECS::Coordinator *coordinator,
                    float h);
void SolveVelocities(CollisionInfo collisionInfo, ECS::Coordinator *coordinator,
                     float h);
