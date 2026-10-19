#pragma once
#include "../collision/epa.h"
#include "../ecs/coordinator.h"

// Inverse stiffness in m/N. 0 = perfectly rigid contact (paper §3.5).
constexpr float COLLISION_COMPLIANCE = 0.0f;
constexpr float FRICTION_COMPLIANCE = 0.0f;

// Contact solver, following Müller et al. 2020 §3.5 and §3.6.
//
// Contact convention (shared with the narrow phase):
//   normal      unit vector from body A towards body B
//   penetration (contactPointA - contactPointB) . normal, > 0 while penetrating
//   contactA/B  local offsets from each body's centre of mass
//
// Both functions expect the caller to have reset lagrangeN / lagrangeT at the
// start of every substep.
void SolvePositions(CollisionResult &collisionInfo,
                    ECS::Coordinator *coordinator, float h);
void SolveVelocities(CollisionResult &collisionInfo,
                     ECS::Coordinator *coordinator, float h);
