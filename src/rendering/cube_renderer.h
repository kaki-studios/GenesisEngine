#include "../core/app.h"
#include "ECS.h"
#include "glm/fwd.hpp"
#include <bgfx/bgfx.h>
#include <glm/common.hpp>
#include <glm/gtc/quaternion.hpp>

struct Transform {
  glm::vec3 position;
  glm::quat rotation;
};

struct Cuboid {
  glm::vec3 halfExtents;
};

class CubeRenderer : public ECS::System {
public:
  CubeRenderer() = default;
  void Update(float dt);
  void Init(App *app);

private:
  App *app;
  // rendering stuff
  bgfx::VertexBufferHandle cubeVbh;
  bgfx::IndexBufferHandle cubeIbh;
  bgfx::ProgramHandle program;
};
