#include "../core/app.h"
#include <bgfx/bgfx.h>

struct PosColorVertex {
  float m_x;
  float m_y;
  float m_z;
  uint32_t m_abgr;

  static void init() {
    ms_decl.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .end();
  };

  static bgfx::VertexLayout ms_decl;
};

class TestRenderer {
  App *app;

  bgfx::VertexBufferHandle vbh;
  bgfx::IndexBufferHandle ibh;
  bgfx::ProgramHandle program;
  float rotation;

  const uint16_t *cubeTriList;
  PosColorVertex *cubeVertices;

public:
  TestRenderer(App *app);
  ~TestRenderer();
  void Update(float dt);
};
