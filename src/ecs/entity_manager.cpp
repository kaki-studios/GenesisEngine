#include "entity_manager.h"
#include <assert.h>

namespace ECS {

EntityManager::EntityManager() {
  // init the queue with all possible entity ids
  for (Entity entity = 0; entity < MAX_ENTITIES; ++entity) {
    mAvailableEntities.push(entity);
  }
}

Entity EntityManager::CreateEntity() {
  assert(mLivingEntityCount < MAX_ENTITIES && "Too many entities");
  // take available id from front of queue
  Entity id = mAvailableEntities.front();
  mAvailableEntities.pop();
  ++mLivingEntityCount;

  return id;
}
void EntityManager::DestroyEntity(Entity entity) {
  assert(entity < MAX_ENTITIES && "Entity out of range");
  // invalidate the destroyed entity's signature
  mSignatures[entity].reset();

  // put the entity id as available at the back of the queue
  mAvailableEntities.push(entity);
  --mLivingEntityCount;
}
void EntityManager::SetSignature(Entity entity, Signature signature) {
  assert(entity < MAX_ENTITIES && "Entity out of range.");

  // Put this entity's signature into the array
  mSignatures[entity] = signature;
}
Signature EntityManager::GetSignature(Entity entity) {
  assert(entity < MAX_ENTITIES && "Entity out of range");

  // get the signature
  return mSignatures[entity];
}
} // namespace ECS
