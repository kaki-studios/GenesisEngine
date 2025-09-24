#include "../ecs/coordinator.h"
#include "../ecs/system.h"
#include "collision/collision.h"
#include <vector>

struct DebugMarker {};

// to be called from rigidbody
class DebugCollisions : public ECS::System {
public:
  void Init(ECS::Coordinator *coordinator);
  void SetCollisions(std::vector<CollisionResult> collisions, int iter);
  void ClearCollisions();

private:
  ECS::Coordinator *coordinator;
};
