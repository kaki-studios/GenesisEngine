#pragma once
#include "../ecs/coordinator.h"
#include "epa.h"
#include <set>

std::vector<CollisionResult>
CollectCollisionPairsNew(std::set<ECS::Entity> entities,
                         ECS::Coordinator *coordinator);
