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
  bgfx::setDebug(BGFX_DEBUG_STATS);
  // these should be in app
  float now, last, deltaTime;
  now = SDL_GetTicks();
  last = now;
  deltaTime = 0.0f;

  while (!app.ShouldClose()) {
    deltaTime = (now - last) / 1000.0f;
    last = now;
    now = SDL_GetTicks();
    app.Update();
    renderer.Update(deltaTime);
  }
  std::cout << "Out" << std::endl;

  return 0;
}
