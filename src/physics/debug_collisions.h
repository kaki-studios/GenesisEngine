#include "../ecs/coordinator.h"
#include "../ecs/system.h"
#include "cuboid/collision.h"
#include <vector>

struct DebugMarker {};

// to be called from rigidbody
class DebugCollisions : public ECS::System {
public:
  void Init(ECS::Coordinator *coordinator);
  void SetCollisions(std::vector<CollisionInfo> collisions);
  void ClearCollisions();

private:
  ECS::Coordinator *coordinator;
};
