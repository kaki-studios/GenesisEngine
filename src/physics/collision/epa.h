#pragma once
#include "../ecs/entity_manager.h"
#include "gjk.h"
#include <glm/common.hpp>
#include <glm/ext.hpp>

struct CollisionResult {
  float penetration;
  glm::vec3 normal;
  glm::vec3 contactA;
  glm::vec3 contactB;
  ECS::Entity bodyA;
  ECS::Entity bodyB;
  bool valid; // if epa failed, false
  float lagrangeMultiplier;
  float restitutionCoeff;

  CollisionResult() : penetration(0.0f), normal(glm::vec3(0)), valid(false) {}
};

struct EPAResult {
  float penetration;
  glm::vec3 normal;
  ECS::Entity bodyA;
  ECS::Entity bodyB;
  bool valid; // false if EPA failed
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

CollisionResult EPA(const Collider &a, const Collider &b,
                    const Simplex &simplex);
