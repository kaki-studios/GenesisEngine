#include "cube_renderer.h"
#include "bx/math.h"
#include <SDL3/SDL_touch.h>
#include <bgfx/bgfx.h>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>

bgfx::VertexLayout PosColorVertex::ms_decl;

bgfx::ShaderHandle loadShader(const char *_name) {
  char *data = new char[2048];
  std::ifstream file;
  size_t fileSize;
  file.open(_name, std::ios::binary);
  if (file.fail()) {
    std::cerr << "couldn't find shader " << _name << " in directory "
              << std::filesystem::current_path() << std::endl;
    return bgfx::ShaderHandle{0};
  }
  if (file.is_open()) {
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    file.read(data, fileSize);
    file.close();
  }
  const bgfx::Memory *mem = bgfx::copy(data, fileSize + 1);
  delete[] data;
  mem->data[mem->size - 1] = '\0';

  std::cerr << "memSize" << mem->size << std::endl;
  bgfx::ShaderHandle handle = bgfx::createShader(mem);
  bgfx::setName(handle, _name);
  return handle;
}

CubeRenderer::CubeRenderer(int width, int height) {
  bgfx::ShaderHandle vsh = loadShader("v_simple.bin");
  bgfx::ShaderHandle fsh = loadShader("f_simple.bin");

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
