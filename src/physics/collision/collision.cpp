#include "collision.h"
#include "../cuboid/rigidbody.h"


std::vector<CollisionResult> CollectCollisionPairsNew(std::set<ECS::Entity> entities, ECS::Coordinator *coordinator) {
    std::vector<CollisionResult> collisions;
//loop through all pairs
  for (auto i = entities.begin(); i != entities.end(); ++i) {
    auto j = i;
    j++; // start from i+1
    for (; j != entities.end(); ++j) {
        ECS::Entity e1 = *i;
        ECS::Entity e2 = *j;
        auto rb1 = coordinator->GetComponent<Rigidbody>(e1);
        auto rb2 = coordinator->GetComponent<Rigidbody>(e2);
        auto c1 = coordinator->GetComponent<Collider>(e1);
        auto c2 = coordinator->GetComponent<Collider>(e2);

    }
  }
    return collisions;
}