#include "bgfx/defines.h"
#include <Engine.h>
#include <Rendering.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <bgfx/bgfx.h>
#include <iostream>
#include <ostream>

int main(void) {
  std::cout << "starting engine demo" << std::endl;
  App app(800, 600);
  CubeRenderer renderer(800, 600);
  std::cout << "app started" << std::endl;
  bgfx::setDebug(BGFX_DEBUG_TEXT);
  while (!app.ShouldClose()) {
    app.Update();
    renderer.Update();
  }
  std::cout << "Out" << std::endl;

  return 0;
}
