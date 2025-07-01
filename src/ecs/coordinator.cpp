#include "coordinator.h"
#include "ecs/entity_manager.h"
#include <memory>

namespace ECS {
void Coordinator::Init() {
  mEntityManager = std::make_unique<EntityManager>();
  mComponentManager = std::make_unique<ComponentManager>();
  mSystemManager = std::make_unique<SystemManager>();
}
void Coordinator::FreeSystems() { mSystemManager->FreeSystems(); }
Signature Coordinator::GetEntitySignature(Entity entity) {
  return mEntityManager->GetSignature(entity);
}
Entity Coordinator::CreateEntity() { return mEntityManager->CreateEntity(); }
void Coordinator::DestroyEntity(Entity entity) {
  mEntityManager->DestroyEntity(entity);
  mComponentManager->EntityDestroyed(entity);
  mSystemManager->EntityDestroyed(entity);
}
} // namespace ECS
