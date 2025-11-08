#include "../collision/epa.h"
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

// CollisionInfo SAT(OBB o1, OBB o2);

// caller must ensure that all entites have the following components:
// Transform
// Rigidbody
// Cuboid
std::vector<CollisionResult>
CollectCollisionPairs(std::set<ECS::Entity> entities,
                      ECS::Coordinator *coordinator);
void SolvePositions(CollisionResult &collisionInfo,
                    ECS::Coordinator *coordinator, float h);
void SolveVelocities(CollisionResult &collisionInfo,
                     ECS::Coordinator *coordinator, float h);
