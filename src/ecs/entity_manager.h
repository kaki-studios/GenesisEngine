// based on https://austinmorlan.com/posts/entity_component_system/

#pragma once
#include <array>
#include <bitset>
#include <cstdint>
#include <queue>
namespace ECS {
using Entity = std::uint32_t;
const Entity MAX_ENTITIES = 5000;
using ComponentType = std::uint8_t;

const ComponentType MAX_COMPONENTS = 32;
// A simple type alias
using Signature = std::bitset<MAX_COMPONENTS>;

class EntityManager {
public:
  EntityManager();
  Entity CreateEntity();
  void DestroyEntity(Entity);
  void SetSignature(Entity, Signature);
  Signature GetSignature(Entity);

private:
  // queue of unused entity ids
  std::queue<Entity> mAvailableEntities{};
  // index = entity id
  std::array<Signature, MAX_ENTITIES> mSignatures{};
  // used to keep limits on how many exist
  uint32_t mLivingEntityCount{};
};

} // namespace ECS
