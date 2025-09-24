#include "debug_collisions.h"
#include "../ecs/coordinator.h"
#include "../ecs/entity_manager.h"
#include "../rendering/cube_renderer.h"
#include "glm/ext/quaternion_geometric.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/gtc/quaternion.hpp"
#include <iostream>
#include <vector>

void DebugCollisions::ClearCollisions() {
  std::vector<ECS::Entity> toBeDeleted;
  for (auto &entity : mEntities) {
    toBeDeleted.push_back(entity);
  }
  for (auto &entity : toBeDeleted) {
    coordinator->DestroyEntity(entity);
  }
}

void DebugCollisions::Init(ECS::Coordinator *coordinator) {
  this->coordinator = coordinator;

  ECS::Signature signature;
  coordinator->RegisterComponent<DebugMarker>();
  signature.set(coordinator->GetComponentType<DebugMarker>(), true);
  coordinator->SetSystemSignature<DebugCollisions>(signature);
}

void DebugCollisions::SetCollisions(std::vector<CollisionResult> collisions,
                                    int iter) {
  if (collisions.size() > 0) {
    std::cout << "collision count: " << collisions.size() << "\n";
  }
  for (auto &collision : collisions) {
    auto t1 = coordinator->GetComponent<Transform>(collision.bodyA);
    auto t2 = coordinator->GetComponent<Transform>(collision.bodyB);
    ECS::Entity entity = coordinator->CreateEntity();
    const float strecthFactor = 10.0f;
    glm::vec3 contact = (collision.contactA + collision.contactB) * 0.5f;
    // std::cout << "Contact Point: " << contact.x << ", " << contact.y << ", "
    // << contact.z << "\n"; std::cout << "Between entity " << collision.bodyA
    // << " and entity " << collision.bodyB << "\n"; std::cout << "Entity 1 pos:
    // " << t1.position.x << ", " << t1.position.y << ", " << t1.position.z <<
    // "\n"; std::cout << "Entity 2 pos: " << t2.position.x << ", " <<
    // t2.position.y << ", " << t2.position.z << "\n";

    glm::vec3 p1 = t1.position + (t1.rotation * collision.contactA);
    glm::vec3 p2 = t2.position + (t2.rotation * collision.contactB);
    float penetration = glm::abs(glm::dot(p2 - p1, collision.normal));

    glm::vec3 pos = p1 + p2 / 2.0f;
    coordinator->AddComponent(entity, DebugMarker{});
    coordinator->AddComponent(
        entity,
        Cuboid{
            .halfExtents = glm::vec3(0.1, 0.1, penetration * strecthFactor),
            .color =
                glm::vec3((float(iter) / 20.0f), float(20 - iter) / 20.0f, 0.0),
        });
    glm::quat dir = glm::quatLookAt(collision.normal, glm::vec3(0.0, 1.0, 0.0));
    coordinator->AddComponent(entity, Transform{
                                          .position = pos,
                                          .rotation = glm::normalize(dir),
                                      });
  }
}
