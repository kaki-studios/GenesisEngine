#include "cube_renderer.h"
#include <../ecs/system.h>
#include <bgfx/bgfx.h>
#include <bgfx/defines.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// shader hell
#include "bgfx/embedded_shader.h"
#include "glm/ext/quaternion_transform.hpp"
#include "shaders/generated/essl/f_simple.sc.bin.h"
#include "shaders/generated/essl/v_simple.sc.bin.h"
#include "shaders/generated/glsl/f_simple.sc.bin.h"
#include "shaders/generated/glsl/v_simple.sc.bin.h"
#include "shaders/generated/spirv/f_simple.sc.bin.h"
#include "shaders/generated/spirv/v_simple.sc.bin.h"
#if defined(_WIN32)
#include "shaders/generated/dx11/f_simple.sc.bin.h"
#include "shaders/generated/dx11/v_simple.sc.bin.h"
#endif //  defined(_WIN32)
#if __APPLE__
#include "shaders/generated/mtl/f_simple.sc.bin.h"
#include "shaders/generated/mtl/v_simple.sc.bin.h"
#endif // __APPLE__
#if !defined(_WIN32)
#undef BGFX_EMBEDDED_SHADER_DXBC
#define BGFX_EMBEDDED_SHADER_DXBC(...)
#endif

struct VertPosCol {
  float x;
  float y;
  float z;
  uint32_t abgr;
  static void init() {
    ms_decl.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .end();
  };

  static bgfx::VertexLayout ms_decl;
};

bgfx::VertexLayout VertPosCol::ms_decl;

/* crude ascii visualization
5----------6
|\         |\
| \        | \
|  1-------+--2
7__|_______8  |
 \ |        \ |
  \|         \|
   3----------4
*/
const VertPosCol vertList[] = {
    {-1.0f, -1.0f, -1.0f, 0xffff00ff}, {1.0f, -1.0f, -1.0f, 0xff00ffff},
    {-1.0f, 1.0f, -1.0f, 0xffffff00},  {1.0f, 1.0f, -1.0f, 0xff0000ff},
    {-1.0f, -1.0f, 1.0f, 0xff000000},  {1.0f, -1.0f, 1.0f, 0xff880088},
    {-1.0f, 1.0f, 1.0f, 0xff00ff88},   {1.0f, 1.0f, 1.0f, 0xff008888},

};

// indices for a cuboid
const uint16_t triList[] = {
    // front side
    1,
    4,
    2,
    1,
    3,
    4,
    // left side
    1,
    5,
    3,
    3,
    5,
    7,
    // right side
    4,
    8,
    6,
    4,
    6,
    2,
    // top side
    1,
    6,
    5,
    1,
    2,
    6,
    // bottom side
    3,
    8,
    4,
    3,
    7,
    8,
    // back side
    7,
    5,
    6,
    7,
    6,
    8,

};

const bgfx::EmbeddedShader k_vs = BGFX_EMBEDDED_SHADER(v_simple);
const bgfx::EmbeddedShader k_fs = BGFX_EMBEDDED_SHADER(f_simple);

void CubeRenderer::Init(App *app) {
  this->app = app;
  VertPosCol::init();

  cubeVbh = bgfx::createVertexBuffer(bgfx::makeRef(vertList, sizeof(vertList)),
                                     VertPosCol::ms_decl);
  cubeIbh = bgfx::createIndexBuffer(bgfx::makeRef(triList, sizeof(triList)));
  bgfx::ShaderHandle vsh =
      bgfx::createEmbeddedShader(&k_vs, bgfx::getRendererType(), "v_simple");

  bgfx::ShaderHandle fsh =
      bgfx::createEmbeddedShader(&k_fs, bgfx::getRendererType(), "f_simple");

  program = bgfx::createProgram(vsh, fsh, true);

  int width, height;
  app->GetWindowDims(&width, &height);
  // Set view rectangle for 0th view
  bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));

  // Clear the view rect
  bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355FF, 1.0f,
                     0);

  ECS::Signature signature;
  signature.set(app->coordinator.GetComponentType<Transform>());
  signature.set(app->coordinator.GetComponentType<Cuboid>());
  app->coordinator.SetSystemSignature<CubeRenderer>(signature);
}

glm::mat4 ProjectionMatrix(float fovy, float aspect, float near, float far,
                           bool homogeneousDepth) {
  return homogeneousDepth ? glm::perspectiveNO(fovy, aspect, near, far)
                          : glm::perspectiveZO(fovy, aspect, near, far);
}

void CubeRenderer::Update(float dt) {

  const glm::vec3 center = {0.0f, 0.0f, 0.0f};
  const glm::vec3 eye = {0.0f, 0.75f, -2.0f};
  int width, height;
  if (!app->GetWindowDims(&width, &height)) {
    std::cout << SDL_GetError() << std::endl;
  }
  glm::mat4 projection =
      ProjectionMatrix(60.0f, float(width) / float(height), 0.1f, 100.0f,
                       bgfx::getCaps()->homogeneousDepth);
  glm::mat4 view = glm::lookAt(eye, center, glm::vec3(0.0f, 1.0f, 0.0f));

  bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355FF, 1.0f,
                     0);

  bgfx::setViewTransform(0, glm::value_ptr(view), glm::value_ptr(projection));
  bgfx::setViewRect(0, 0, 0, (uint16_t)width, (uint16_t)height);
  if (mEntities.empty()) {
    // submit empty primitive
    bgfx::touch(0);
  }

  for (auto const &entity : mEntities) {

    auto &transform = app->coordinator.GetComponent<Transform>(entity);
    auto &cuboid = app->coordinator.GetComponent<Cuboid>(entity);
    // TODO doesn't work
    transform.rotation =
        glm::rotate(transform.rotation, 40.0f * dt, {0.0f, 1.0f, 0.0f});

    glm::mat4 transformMat = glm::translate(glm::mat4(1), transform.position) *
                             glm::mat4_cast(transform.rotation) *
                             // scaling a cuboid from (-1,-1,-1) by its
                             // halfExtents gives the correct extents
                             glm::scale(glm::mat4(1), cuboid.halfExtents);
    bgfx::setTransform(glm::value_ptr(transformMat));
    bgfx::setVertexBuffer(0, cubeVbh);
    bgfx::setIndexBuffer(cubeIbh);
    bgfx::setState(BGFX_STATE_DEFAULT);
    bgfx::submit(0, program);
  }
}
