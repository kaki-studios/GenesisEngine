#include "cube_renderer.h"
#include <SDL3/SDL_touch.h>
#include <bgfx/bgfx.h>
#include <bgfx/embedded_shader.h>
#include <bx/math.h>
#include <cstdint>

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

bgfx::VertexLayout PosColorVertex::ms_decl;
const bgfx::EmbeddedShader k_vs = BGFX_EMBEDDED_SHADER(v_simple);
const bgfx::EmbeddedShader k_fs = BGFX_EMBEDDED_SHADER(f_simple);

CubeRenderer::CubeRenderer(int width, int height) {
  bgfx::ShaderHandle vsh =
      bgfx::createEmbeddedShader(&k_vs, bgfx::getRendererType(), "f_simple.sc");
  bgfx::ShaderHandle fsh =
      bgfx::createEmbeddedShader(&k_fs, bgfx::getRendererType(), "v_simple.sc");

  program = bgfx::createProgram(vsh, fsh, true);

  cubeTriList = new uint16_t[6]{0, 1, 3, 1, 2, 3};
  cubeVertices = new PosColorVertex[4]{
      {0.5f, 0.5f, 0.0f, 0xff0000ff},
      {0.5f, -0.5f, 0.0f, 0xff0000ff},
      {-0.5f, -0.5f, 0.0f, 0xff00ff00},
      {-0.5f, 0.5f, 0.0f, 0xff00ff00},
  };
  PosColorVertex::init();
  vbh = bgfx::createVertexBuffer(
      bgfx::makeRef(cubeVertices, sizeof(PosColorVertex) * 4),
      PosColorVertex::ms_decl);
  ibh =
      bgfx::createIndexBuffer(bgfx::makeRef(cubeTriList, sizeof(uint16_t) * 6));
  // Set view rectangle for 0th view
  bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));

  // Clear the view rect
  bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x443355FF, 1.0f,
                     0);
}
void CubeRenderer::Update() {
  const bx::Vec3 at = {0.0f, 0.0f, 0.0f};
  const bx::Vec3 eye = {0.0f, 0.0f, 10.0f};
  float view[16];
  bx::mtxLookAt(view, eye, at);
  float proj[16];
  bx::mtxProj(proj, 60.0f, float(width) / float(height), 0.1f, 100.0f,
              bgfx::getCaps()->homogeneousDepth);
  bgfx::setViewTransform(0, view, proj);
  bgfx::setViewRect(0, 0, 0, width, height);
  bgfx::touch(0);
  float mtx[16];
  bx::mtxRotateY(mtx, 0.0f);
  mtx[12] = 0.0f;
  mtx[13] = 0.0f;
  mtx[14] = 0.0f;

  bgfx::setTransform(mtx);
  bgfx::setVertexBuffer(0, vbh);
  bgfx::setIndexBuffer(ibh);
  bgfx::setState(BGFX_STATE_DEFAULT);
  bgfx::submit(0, program);
  bgfx::frame();
}

CubeRenderer::~CubeRenderer() {
  delete[] cubeTriList;
  delete[] cubeVertices;
}
