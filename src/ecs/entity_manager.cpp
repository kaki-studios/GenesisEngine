#include "entity_manager.h"

namespace ECS {

EntityManager::EntityManager() {
  // init the queue with all possible entity ids
  for (Entity entity = 0; entity < MAX_ENTITIES; ++entity)
    mAvailableEntities.push(entity);
}

Entity EntityManager::CreateEntity() {
  assert(!mAvailableEntities.empty() && "Too many entities");
  Entity id = mAvailableEntities.front();
  mAvailableEntities.pop();
  // Clear any stale signature left behind by a recycled id. Without this a
  // respawned entity inherits the dead one's signature and gets pushed into
  // systems it has no components for.
  mSignatures[id].reset();
  return id;
}

void EntityManager::DestroyEntity(Entity entity) {
  assert(entity < MAX_ENTITIES && "Entity out of range");
  mSignatures[entity].reset();
  mAvailableEntities.push(entity);
}

void EntityManager::SetSignature(Entity entity, Signature signature) {
  assert(entity < MAX_ENTITIES && "Entity out of range.");
  mSignatures[entity] = signature;
}

Signature EntityManager::GetSignature(Entity entity) {
  assert(entity < MAX_ENTITIES && "Entity out of range");
  return mSignatures[entity];
}

} // namespace ECS
