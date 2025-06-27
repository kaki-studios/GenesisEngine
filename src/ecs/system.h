#pragma once
#include "entity_manager.h"
#include <set>
namespace ECS {
class System {
public:
  std::set<Entity> mEntities;
};
} // namespace ECS
