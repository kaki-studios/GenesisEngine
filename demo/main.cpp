#include "bgfx/defines.h"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include <Engine.h>
#include <Rendering.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_timer.h>
#include <array>
#include <bgfx/bgfx.h>
#include <iostream>
#include <ostream>

int main(void) {
  std::cout << "Starting EngineDemo..." << std::endl;
  App app(1920, 1080);

  std::array<ECS::Entity, 2> entities;
  app.coordinator.RegisterComponent<Transform>();
  app.coordinator.RegisterComponent<Cuboid>();
  auto cubeRenderer = app.coordinator.RegisterSystem<CubeRenderer>();
  cubeRenderer->Init(&app);
  // initialize entities
  for (int i = 0; i < entities.size(); i++) {
    entities[i] = app.coordinator.CreateEntity();
    app.coordinator.AddComponent(
        entities[i], Transform{
                         .position = glm::vec3(i * 5.0f, 0.0f, 0.0f),
                         .rotation = glm::quat(),
                     });

    app.coordinator.AddComponent(entities[i],
                                 Cuboid{
                                     .halfExtents = glm::vec3(1.0f, 1.0f, 1.0f),
                                 });
  }

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
    cubeRenderer->Update(deltaTime);

    bgfx::frame();
  }
  std::cout << "Out" << std::endl;

  return 0;
}
