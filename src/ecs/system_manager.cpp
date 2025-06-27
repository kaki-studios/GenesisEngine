#include "system_manager.h"

namespace ECS {
void SystemManager::EntityDestroyed(Entity entity) {
  for (auto const &pair : mSystems) {
    auto const &system = pair.second;
    system->mEntities.erase(entity);
  }
}
void SystemManager::EntitySignatureChanged(Entity entity,
                                           Signature entitySignature) {
  // Notify each system that an entity's signature changed
  for (auto const &pair : mSystems) {

    auto const &type = pair.first;
    auto const &system = pair.second;
    auto const &systemSignature = mSignatures[type];
    // entity signature matched system signature
    if ((entitySignature & systemSignature) == systemSignature) {
      system->mEntities.insert(entity);
    } else {
      system->mEntities.erase(entity);
    }
  }
}
} // namespace ECS
