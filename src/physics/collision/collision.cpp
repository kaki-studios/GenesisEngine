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
        auto t1 = coordinator->GetComponent<Transform>(e1);
        auto t2 = coordinator->GetComponent<Transform>(e2);
        if (rb1.invMass == 0 && rb2.invMass == 0) {
            continue;
        }
        c1.inner->position = t1.position;
        c1.inner->rotation = t1.rotation;

        c2.inner->position = t2.position;
        c2.inner->rotation = t2.rotation;
        //TODO broadphase
        GJKResult gjk = GJKIntersect(&c1, &c2);
        if (!gjk.colliding) {
            continue;
        }
        CollisionResult collision = EPA(&c1, &c2,  gjk.simplex);
        collision.bodyA = e1;
        collision.bodyB = e2;
        if(collision.penetration > 0)
            collisions.push_back(collision);
    }
  }
    return collisions;
}