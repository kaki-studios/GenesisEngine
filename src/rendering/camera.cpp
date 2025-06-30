#include "camera.h"
#include "cube_renderer.h"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keycode.h>
#include <glm/common.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/trigonometric.hpp>
#include <glm/vec3.hpp>

float wrapFloat(float value, float min, float max) {
  float range = max - min;
  return min + glm::mod(value - min, range);
}

void CameraSystem::Init(App *app) {
  this->app = app;

  ECS::Signature signature;
  signature.set(app->coordinator.GetComponentType<Camera>(), true);
  signature.set(app->coordinator.GetComponentType<Transform>(), true);
  app->coordinator.SetSystemSignature<CameraSystem>(signature);
}

void CameraSystem::Update(float dt) {
  for (auto &entity : mEntities) {

    auto &camera = app->coordinator.GetComponent<Camera>(entity);
    auto &transform = app->coordinator.GetComponent<Transform>(entity);
    auto inputState = app->GetInputState();

    // rotation
    camera.pitch -= inputState.mouseDeltaY * camera.moveSpeed * 0.001;
    camera.yaw -= inputState.mouseDeltaX * camera.moveSpeed * 0.001;
    // camera.pitch = glm::mod(camera.pitch, glm::pi<float>());
    camera.yaw = wrapFloat(camera.yaw, 0, 2 * glm::pi<float>());
    camera.pitch = wrapFloat(camera.pitch, -0.5 * glm::pi<float>(),
                             0.5 * glm::pi<float>());

    transform.rotation = glm::angleAxis(camera.yaw, glm::vec3(0, 1, 0)) *
                         glm::angleAxis(camera.pitch, glm::vec3(1, 0, 0));

    // movement
    glm::vec3 forwardXZ =
        glm::angleAxis(camera.yaw, glm::vec3(0, 1, 0)) * glm::vec3(0, 0, -1);
    forwardXZ.y = 0;
    forwardXZ = glm::normalize(forwardXZ);
    glm::vec3 rightXZ =
        glm::normalize(glm::cross(forwardXZ, glm::vec3(0, 1, 0)));

    glm::vec3 moveDir = glm::vec3(0.0f);

    if (inputState.keysDown[SDLK_W])
      moveDir += forwardXZ;
    if (inputState.keysDown[SDLK_A])
      moveDir -= rightXZ;
    if (inputState.keysDown[SDLK_S])
      moveDir -= forwardXZ;
    if (inputState.keysDown[SDLK_D])
      moveDir += rightXZ;
    if (inputState.keysDown[SDLK_SPACE])
      moveDir += glm::vec3(0, 1, 0);
    if (inputState.keysDown[SDLK_LSHIFT])
      moveDir -= glm::vec3(0, 1, 0);
    if (glm::length(moveDir) > 0.0f) {
      transform.position += glm::normalize(moveDir) * camera.moveSpeed * dt;
    }
  }
}
