#include "rigidbody.h"
#include "glm/ext/quaternion_common.hpp"
#include "glm/ext/quaternion_geometric.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/matrix.hpp"
#include "rendering/cube_renderer.h"
#include <iostream>

glm::mat3 ComputeCuboidInvInertia(glm::vec3 dims, float density) {
  float volume = dims.x * dims.y * dims.z;
  float x_sqr = dims.x * dims.x;
  float y_sqr = dims.y * dims.y;
  float z_sqr = dims.z * dims.z;
  float mass = volume * density;

  glm::mat3 invInertia = {
      (12.0f) / (mass * (y_sqr + z_sqr)), 0.0f, 0.0f, 0.0f,
      (12.0f) / (mass * (x_sqr + z_sqr)), 0.0f, 0.0f, 0.0f,
      (12.0f) / (mass * (y_sqr + z_sqr)),
  };
  return invInertia;
}

void printMat3(const glm::mat3 &mat) {
  for (int row = 0; row < 3; ++row) {
    std::cout << "[ ";
    for (int col = 0; col < 3; ++col) {
      std::cout << mat[col][row] << " ";
    }
    std::cout << "]\n";
  }
}

Rigidbody CreateCuboidRB(glm::vec3 halfExtents, float density,
                         glm::vec3 angularVelocity) {
  glm::vec3 dims = 2.0f * halfExtents;

  glm::mat3 invInertia = ComputeCuboidInvInertia(dims, density);
  float volume = dims.x * dims.y * dims.z;
  float invMass = 1.0f / (volume * density);

  return Rigidbody{
      .prevPosition = glm::vec3(0),
      .prevRotation = glm::vec3(0),
      .density = density,
      .invMass = invMass,
      .linearVelocity = glm::vec3(0),
      .angularVelocity = angularVelocity,
      .extForce = glm::vec3(0),
      .extTorque = glm::vec3(0),
      .invInertia = invInertia,
      .restitution = 0.0f,
      .friction = 0.0f,
  };
}

void RigidbodySystem::Init(App *app) {
  this->app = app;

  ECS::Signature signature;
  signature.set(app->coordinator.GetComponentType<Transform>());
  signature.set(app->coordinator.GetComponentType<Rigidbody>());
  // no need for Cuboid, yet
  app->coordinator.SetSystemSignature<RigidbodySystem>(signature);
}

const int NUM_SUBSTEPS = 1;

void RigidbodySystem::Update(double dt) {
  // should collect collision pairs here
  double h = dt / NUM_SUBSTEPS;
  for (int i = 0; i < NUM_SUBSTEPS; i++) {
    for (auto &entity : mEntities) {
      auto &transform = app->coordinator.GetComponent<Transform>(entity);
      // std::cout << "-------entity: " << entity << " -------" << std::endl;

      // std::cout << transform.position.x << ", " << transform.position.y << ",
      // "
      //           << transform.position.z << ", " << entity << std::endl;
      auto &rb = app->coordinator.GetComponent<Rigidbody>(entity);
      // integrate positions and velocities
      rb.prevPosition = transform.position;
      rb.linearVelocity += float(h) * rb.extForce * rb.invMass;
      // std::cout << h << ", " << entity << std::endl;
      transform.position += float(h) * rb.linearVelocity;

      // std::cout << transform.position.x << ", " << transform.position.y << ",
      // "
      //           << transform.position.z << ", " << entity << std::endl;

      // integrate rotations and angular velocities
      rb.prevRotation = transform.rotation;
      glm::mat3 R = glm::mat3_cast(transform.rotation);
      // glm::mat3 worldInvInertia = R * rb.invInertia * glm::transpose(R);
      glm::mat3 worldInvInertia = rb.invInertia;
      glm::mat3 worldInertia = glm::inverse(worldInvInertia);
      // w is omega:
      // w <- w + h*I^-1(T_ext-(w x (Iw)))
      std::cout << "start " << entity << std::endl;
      std::cout << rb.angularVelocity.x << ", " << rb.angularVelocity.y << ", "
                << rb.angularVelocity.z << ", " << entity << std::endl;
      printMat3(worldInvInertia);
      auto cross =
          glm::cross(rb.angularVelocity, (worldInertia * rb.angularVelocity));
      std::cout << cross.length() << std::endl;
      rb.angularVelocity +=
          float(h) * worldInvInertia *
          (rb.extTorque - (glm::cross(rb.angularVelocity,
                                      (worldInertia * rb.angularVelocity))));
      std::cout << rb.angularVelocity.x << ", ";
      // nvim just absolutely fucked my shit up somehow and i lost like 100
      // lines of code cuz my editor was bugging
    }
  }
}
