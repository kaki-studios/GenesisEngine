// a rigidbody implementation based on XPBD:
// https://matthias-research.github.io/pages/publications/PBDBodies.pdf

#include "../core/app.h"
#include "../debug_collisions.h"
#include "../ecs/system.h"
#include "glm/fwd.hpp"
#include <glm/common.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>
#include <memory>

struct Rigidbody {
  // these are already in Transform
  // glm::vec3 position;
  // glm::quat rotation;

  glm::vec3 prevPosition;
  glm::quat prevRotation;

  float density;
  float invMass;

  glm::vec3 linearVelocity;
  glm::vec3 angularVelocity; // for xpbd we store angVel instead of angMomentum

  glm::vec3 prevLinearVelocity;
  glm::vec3 prevAngularVelocity;

  glm::vec3 extForce;
  glm::vec3 extTorque;

  glm::mat3 invInertia;
  float restitution;
  float friction;
};

class RigidbodySystem : public ECS::System {

public:
  RigidbodySystem() = default;
  void Update(double dt);
  void Init(App *app);

private:
  App *app;
  std::shared_ptr<DebugCollisions> debug;
};

Rigidbody CreateSB();
Rigidbody CreateCuboidRB(glm::vec3 halfExtents, float density);
