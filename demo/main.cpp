#include "bgfx/defines.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_float.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include <Engine.h>
#include <Physics.h>
#include <Rendering.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_timer.h>
#include <array>
#include <bgfx/bgfx.h>
#include <cstdlib>
#include <iostream>
#include <ostream>

int main(void) {
  std::cout << "Starting EngineDemo..." << std::endl;
  App app(1920, 1080);

  std::array<ECS::Entity, 2> entities;
  // these should be somewhere else
  app.coordinator.RegisterComponent<Transform>();
  app.coordinator.RegisterComponent<Cuboid>();
  app.coordinator.RegisterComponent<Rigidbody>();

  auto cubeRenderer = app.coordinator.RegisterSystem<CubeRenderer>();
  cubeRenderer->Init(&app);
  auto rbSystem = app.coordinator.RegisterSystem<RigidbodySystem>();
  rbSystem->Init(&app);

  // initialize entities
  for (int i = 0; i < entities.size(); i++) {
    entities[i] = app.coordinator.CreateEntity();
    glm::vec3 halfExtents = glm::vec3(1.0f, 5.0f, 0.2f);
    app.coordinator.AddComponent(
        entities[i], Transform{
                         .position = glm::vec3(i * 5.0f, 10.0f, 0.0f),
                         .rotation = glm::identity<glm::quat>(),
                     });

    app.coordinator.AddComponent(entities[i],
                                 Cuboid{
                                     .halfExtents = halfExtents,
                                     .color = glm::vec3(1.0f, 0.5f * i, 0.0f),
                                 });
    app.coordinator.AddComponent(entities[i],
                                 CreateCuboidRB(halfExtents, 1.0f));
    app.coordinator.GetComponent<Rigidbody>(entities[i]).angularVelocity =
        glm::vec3(1.0, 0.1, 0.0);
    app.coordinator.GetComponent<Rigidbody>(entities[i]).linearVelocity =
        glm::vec3(-float((i * 2) - 1), 0.0f, 0.0f);
    // gravity
    app.coordinator.GetComponent<Rigidbody>(entities[i]).extForce =
        glm::vec3(0.0f, -9.81f, 0.0f);
  }
  // ground
  ECS::Entity ground = app.coordinator.CreateEntity();
  app.coordinator.AddComponent(ground,
                               Transform{
                                   .position = glm::vec3(0.0f, -2.0f, 0.0f),
                                   .rotation = glm::identity<glm::quat>(),
                               });
  app.coordinator.AddComponent(ground, CreateSB());
  app.coordinator.AddComponent(
      ground, Cuboid{
                  .halfExtents = glm::vec3(100.0f, 0.5f, 100.0f),
                  .color = glm::vec3(0.0f, 0.0f, 1.0f),
              });

  std::cout << "App starting..." << std::endl;
  bgfx::setDebug(BGFX_DEBUG_STATS);
  // these should be in app
  double now, last, deltaTime;
  now = SDL_GetTicks();
  last = now;
  deltaTime = 0.0;
  double accumulator = 0.0;
  const double H = 0.01;

  while (!app.ShouldClose()) {
    deltaTime = (now - last) / 1000.0;
    last = now;
    now = SDL_GetTicks();
    app.Update();
    accumulator += deltaTime;
    while (accumulator >= H) {
      rbSystem->Update(H);
      accumulator -= H;
    }

    cubeRenderer->Update();

    bgfx::frame();
  }
  std::cout << "Out" << std::endl;

  return 0;
}
