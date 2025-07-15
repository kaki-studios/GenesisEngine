#include <glm/common.hpp>
#include <glm/ext.hpp>
#include "gjk.h"
#include "../../ecs/coordinator.h"

struct CollisionResult {
    float penetration;
    glm::vec3 normal;
    glm::vec3 contactA;
    glm::vec3 contactB;
    ECS::Entity bodyA;
    ECS::Entity bodyB;
};


CollisionResult EPA(const Collider *a, const Collider *b, Simplex s);