#include "bgfx/defines.h"
#include <Engine.h>
#include <Rendering.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_timer.h>
#include <bgfx/bgfx.h>
#include <iostream>
#include <ostream>

int main(void) {
  std::cout << "Starting EngineDemo..." << std::endl;
  App app(1920, 1080);
  CubeRenderer renderer(&app);
  std::cout << "App starting..." << std::endl;
  bgfx::setDebug(BGFX_DEBUG_TEXT);
  // these should be in app
  while (!app.ShouldClose()) {
    app.Update();
    renderer.Update();
  }
  std::cout << "Out" << std::endl;

  return 0;
}
