#include "rigidbody.h"
#include "../../rendering/cube_renderer.h"
#include "../collision/collision.h"
#include "collision.h"
#include "glm/ext/quaternion_common.hpp"
#include "glm/ext/quaternion_geometric.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/matrix.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <vector>

void printMat3(const glm::mat3 &mat) {
  for (int row = 0; row < 3; ++row) {
    std::cout << "[ ";
    for (int col = 0; col < 3; ++col) {
      std::cout << mat[col][row] << " ";
    }
    std::cout << "]\n";
  }
}
glm::mat3 ComputeCuboidInvInertia(glm::vec3 dims, float density) {
  float volume = dims.x * dims.y * dims.z;
  float x_sqr = dims.x * dims.x;
  float y_sqr = dims.y * dims.y;
  float z_sqr = dims.z * dims.z;
  float mass = volume * density;

  glm::mat3 invInertia = {
      (12.0f) / (mass * (y_sqr + z_sqr)), 0.0f, 0.0f, 0.0f,
      (12.0f) / (mass * (x_sqr + z_sqr)), 0.0f, 0.0f, 0.0f,
      (12.0f) / (mass * (y_sqr + x_sqr)),
  };
  return invInertia;
}

Rigidbody CreateSB() {
  return Rigidbody{
      .prevPosition = glm::vec3(0),
      .prevRotation = glm::vec3(0),
      .density = 0.0f,
      .invMass = 0.0f, // this makes it unmovable, implies infinite mass
      .linearVelocity = glm::vec3(0),
      .angularVelocity = glm::vec3(0),
      .extForce = glm::vec3(0),
      .invInertia = glm::mat3(0),
      .restitution = 0.5f,
      .friction = 0.5f,
  };
}

Rigidbody CreateCuboidRB(glm::vec3 halfExtents, float density) {
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
      .angularVelocity = glm::vec3(0),
      .extForce = glm::vec3(0),
      .extTorque = glm::vec3(0),
      .invInertia = invInertia,
      .restitution = 0.5f,
      .friction = 0.5f,
  };
}

void RigidbodySystem::Init(App *app) {
  this->app = app;
  this->debug = app->coordinator.RegisterSystem<DebugCollisions>();
  debug->Init(&app->coordinator);

  ECS::Signature signature;
  signature.set(app->coordinator.GetComponentType<Transform>(), true);
  signature.set(app->coordinator.GetComponentType<Rigidbody>(), true);
  signature.set(app->coordinator.GetComponentType<Cuboid>(), true);
  signature.set(app->coordinator.GetComponentType<Collider>(), true);
  app->coordinator.SetSystemSignature<RigidbodySystem>(signature);
}

const int NUM_SUBSTEPS = 20;
const int NUM_POS_ITERS = 2;

void RigidbodySystem::Update(double dt) {
  debug->ClearCollisions();
  // collect collisions
  for (auto &entity : mEntities) {
    auto &t = app->coordinator.GetComponent<Transform>(entity);
    t.rotation = glm::normalize(t.rotation);
  }
  std::vector<CollisionResult> collisions =
      CollectCollisionPairsNew(mEntities, &app->coordinator);
  if (collisions.size()) {
    std::cout << collisions.size() << std::endl;
  }
  for (auto &collision : collisions) {
    //   auto &rb1 = app->coordinator.GetComponent<Rigidbody>(collision.bodyA);
    //   auto &t1 = app->coordinator.GetComponent<Transform>(collision.bodyA);
    //   auto &rb2 = app->coordinator.GetComponent<Rigidbody>(collision.bodyB);
    //   auto &t2 = app->coordinator.GetComponent<Transform>(collision.bodyB);
    //   glm::vec3 r1 = collision.contact - t1.position;
    //   glm::vec3 r2 = collision.contact - t2.position;
    //   collision.p1 = t1.position + glm::mat3_cast(t1.rotation) * r1;
    //   collision.p2 = t2.position + glm::mat3_cast(t2.rotation) * r2;
    //   collision.p1hat = rb1.prevPosition + glm::mat3_cast(rb1.prevRotation) *
    //   r1; collision.p2hat = rb2.prevPosition +
    //   glm::mat3_cast(rb2.prevRotation)
    //   * r1;

    // collision.penetration =
    //     glm::dot(collision.p2 - collision.p1, collision.normal);

    // std::cout << "bodyA: " << collision.bodyA << "\n";
    // std::cout << "bodyB: " << collision.bodyB << "\n";
  }
  double h = dt / NUM_SUBSTEPS;
  for (int i = 0; i < NUM_SUBSTEPS; i++) {
    for (auto &entity : mEntities) {
      auto &transform = app->coordinator.GetComponent<Transform>(entity);
      auto &rb = app->coordinator.GetComponent<Rigidbody>(entity);
      // integrate positions and velocities
      rb.prevPosition = transform.position;
      rb.linearVelocity += float(h) * rb.extForce * rb.invMass;
      transform.position += float(h) * rb.linearVelocity;

      transform.rotation = glm::normalize(transform.rotation);

      // integrate rotations and angular velocities
      rb.prevRotation = transform.rotation;
      glm::mat3 R = glm::mat3_cast(transform.rotation);
      glm::mat3 worldInvInertia = R * (rb.invInertia * glm::transpose(R));

      if (glm::determinant(worldInvInertia) < 1e-6f) {
        // probably a staticbody
        continue;
      }
      glm::mat3 worldInertia = glm::inverse(worldInvInertia);
      // w is omega:
      // w <- w + h*I^-1(T_ext-(w x (Iw)))
      rb.angularVelocity +=
          float(h) * worldInvInertia *
          (rb.extTorque - (glm::cross(rb.angularVelocity,
                                      (worldInertia * rb.angularVelocity))));

      transform.rotation += float(h) * 0.5f *
                            glm::quat(0.0, rb.angularVelocity) *
                            transform.rotation;
      transform.rotation = glm::normalize(transform.rotation);
    }
    for (int i = 0; i < NUM_POS_ITERS; i++) {
      for (auto &collisionInfo : collisions) {
        // ECS::Entity bodyA = collisionInfo.bodyA;
        // ECS::Entity bodyB = collisionInfo.bodyB;

        // auto &t1 =
        //     app->coordinator.GetComponent<Transform>(collisionInfo.bodyA);
        // auto &c1 =
        // app->coordinator.GetComponent<Cuboid>(collisionInfo.bodyA); auto &t2
        // =
        //     app->coordinator.GetComponent<Transform>(collisionInfo.bodyB);
        // auto &c2 =
        // app->coordinator.GetComponent<Cuboid>(collisionInfo.bodyB); OBB o1 =
        // OBB(t1.position, t1.rotation, c1.halfExtents); OBB o2 =
        // OBB(t2.position, t2.rotation, c2.halfExtents); collisionInfo =
        // SAT(o1, o2); collisionInfo.bodyA = bodyA; collisionInfo.bodyB =
        // bodyB;
        /// std::cout << "collision penetration: " << collisionInfo.penetration
        //<< std::endl;

        SolvePositions(collisionInfo, &app->coordinator, h);
      }
    }
    for (auto &entity : mEntities) {
      auto &transform = app->coordinator.GetComponent<Transform>(entity);
      auto &rb = app->coordinator.GetComponent<Rigidbody>(entity);

      rb.linearVelocity =
          (transform.position - rb.prevPosition) * float(1.0 / h);
      glm::quat dq = transform.rotation * glm::inverse(rb.prevRotation);
      rb.angularVelocity = 2.0f * glm::vec3(dq.x, dq.y, dq.z) * float(1.0 / h);

      rb.angularVelocity = dq.w >= 0 ? rb.angularVelocity : -rb.angularVelocity;
      // std::printf("entity %d position: %f,%f,%f\n", entity,
      //             transform.position.x, transform.position.y,
      //             transform.position.z);
    }
    for (auto &collision : collisions) {
      // SolveVelocities(collision, &app->coordinator, h);
    }
    debug->SetCollisions(collisions);
  }
}
