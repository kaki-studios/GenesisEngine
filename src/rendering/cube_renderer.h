#include "../core/app.h"
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
  glm::vec3 color;
};

class CameraRetriever : public ECS::System {
public:
  ECS::Entity GetCamera();
  App *app;
};

class CubeRenderer : public ECS::System {
public:
  CubeRenderer() = default;
  void Update();
  void Init(App *app);
  ~CubeRenderer();

private:
  App *app;
  // rendering stuff
  bgfx::VertexBufferHandle cubeVbh;
  // bgfx::IndexBufferHandle cubeIbh;
  bgfx::ProgramHandle program;

  bgfx::UniformHandle u_baseCol;
  bgfx::UniformHandle u_lightDir;

  std::shared_ptr<CameraRetriever> retriever;
};
