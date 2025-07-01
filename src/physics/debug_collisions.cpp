#include "debug_collisions.h"
#include "../ecs/coordinator.h"
#include "../ecs/entity_manager.h"
#include "../rendering/cube_renderer.h"
#include "glm/ext/quaternion_geometric.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/gtc/quaternion.hpp"
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

void DebugCollisions::SetCollisions(std::vector<CollisionInfo> collisions) {
  for (auto &collision : collisions) {
    ECS::Entity entity = coordinator->CreateEntity();
    coordinator->AddComponent(entity, DebugMarker{});
    coordinator->AddComponent(
        entity,
        Cuboid{
            .halfExtents = glm::vec3(0.1, 10 * collision.penetration, 0.1),
            .color = glm::vec3(1.0, 0.0, 0.0),
        });
    glm::quat dir = glm::quatLookAt(collision.normal, glm::vec3(0.0, 1.0, 0.0));
    coordinator->AddComponent(entity, Transform{
                                          .position = collision.contact,
                                          .rotation = glm::normalize(dir),
                                      });
  }
}
