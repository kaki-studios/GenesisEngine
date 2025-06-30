#include "bgfx/defines.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_float.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/trigonometric.hpp"
#include <Engine.h>
#include <Physics.h>
#include <Rendering.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_timer.h>
#include <array>
#include <bgfx/bgfx.h>
#include <cstdio>
#include <iostream>
#include <ostream>

glm::quat quat_look_at(const glm::vec3 &position, const glm::vec3 &target,
                       const glm::vec3 &up = glm::vec3(0, 1, 0)) {
  glm::mat4 view = glm::lookAt(position, target, up);
  glm::mat4 model = glm::inverse(view); // camera-to-world

  return glm::quat_cast(model); // convert matrix to quaternion
}

void CreateWalls(App *app) {
  for (int i = 0; i < 4; i++) {
    ECS::Entity wall = app->coordinator.CreateEntity();
    int sign = i % 2 == 0 ? 1 : -1;
    int j = i < 2 ? i : -i;
    glm::vec3 position =
        glm::vec3(((j + 1) % 2) * 25.0f * sign, 12.5f, (j % 2) * 25.0f * sign);
    std::printf("position: %f, %f, %f\n", position.x, position.y, position.z);

    app->coordinator.AddComponent(
        wall, Transform{
                  .position = position,
                  .rotation = glm::angleAxis(
                      glm::radians(90.0f), glm::vec3((i % 2), 0, (i + 1) % 2)),
              });
    app->coordinator.AddComponent(wall, CreateSB());
    app->coordinator.AddComponent(
        wall, Cuboid{
                  .halfExtents = glm::vec3(25.0f, 0.5f, 25.0f),
                  .color = glm::vec3(0.0f, float((i + 1) % 2), float(i % 2)),
              });
  }
}

int main(void) {
  std::cout << "Starting EngineDemo..." << std::endl;
  App app(1920, 1080);

  std::array<ECS::Entity, 2> entities;
  // these should be somewhere else
  app.coordinator.RegisterComponent<Transform>();
  app.coordinator.RegisterComponent<Cuboid>();
  app.coordinator.RegisterComponent<Rigidbody>();
  app.coordinator.RegisterComponent<Camera>();

  auto cameraSystem = app.coordinator.RegisterSystem<CameraSystem>();
  cameraSystem->Init(&app);
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

    auto &rb = app.coordinator.GetComponent<Rigidbody>(entities[i]);
    rb.angularVelocity = glm::vec3(1.0, 0.1, 0.0);
    rb.linearVelocity = glm::vec3(-float((i * 2) - 1), 0.0f, 0.0f);
    // gravity
    rb.extForce = glm::vec3(0.0f, -9.81f / rb.invMass, 0.0f);
  }
  // ground
  CreateWalls(&app);
  for (int i = 0; i < 2; i++) {
    ECS::Entity ground = app.coordinator.CreateEntity();
    app.coordinator.AddComponent(
        ground, Transform{
                    .position = glm::vec3(0.0f, i * 25.0f, 0.0f),
                    .rotation = glm::identity<glm::quat>(),
                });
    app.coordinator.AddComponent(ground, CreateSB());
    app.coordinator.AddComponent(
        ground, Cuboid{
                    .halfExtents = glm::vec3(25.0f, 0.5f, 25.0f),
                    .color = glm::vec3(0.0f, 0.0f, 1.0f),
                });
  }

  // camera
  ECS::Entity camera = app.coordinator.CreateEntity();
  app.coordinator.AddComponent(
      camera,
      Transform{
          .position = glm::vec3(0.0, 1.5, 10.0),
          .rotation = quat_look_at(glm::vec3(0.0, 1.5, 10.0), glm::vec3(0)),
      });
  app.coordinator.AddComponent(camera, Camera{
                                           .fov = 60.0f,
                                           .moveSpeed = 5.0f,
                                           .nearPlane = 0.1f,
                                           .farPlane = 100.0f,
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

    cameraSystem->Update(deltaTime);
    cubeRenderer->Update();

    bgfx::frame();
  }
  std::cout << "Out" << std::endl;

  return 0;
}
