#include "collision.h"
#include "rigidbody.h"
#include "../../rendering/cube_renderer.h" // Transform
#include "glm/geometric.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/matrix.hpp"
#include <cmath>

namespace {

// World-space inverse inertia: I^-1_world = R * I^-1_body * R^T.
glm::mat3 WorldInvInertia(const Transform &t, const Rigidbody &rb) {
  glm::mat3 R = glm::mat3_cast(t.rotation);
  return R * rb.invInertia * glm::transpose(R);
}

// Linearised quaternion update, paper eqs. (8)/(9):
//   q <- q + 1/2 [I^-1 (r x p), 0] q
// r and p must both be in world space.
void ApplyAngularUpdate(glm::quat &q, const glm::mat3 &invInertiaWorld,
                        const glm::vec3 &rWorld, const glm::vec3 &p,
                        float sign) {
  glm::vec3 dw = sign * (invInertiaWorld * glm::cross(rWorld, p));
  q += 0.5f * glm::quat(0.0f, dw) * q;
  q = glm::normalize(q);
}

// Velocity applied at a point by a body's rigid motion.
glm::vec3 PointVelocity(const Rigidbody &rb, const glm::vec3 &rWorld) {
  return rb.linearVelocity + glm::cross(rb.angularVelocity, rWorld);
}

// Generalized inverse mass along a direction (paper eqs. 2-3):
//   w = 1/m + (r x n)^T I^-1 (r x n)
float GeneralizedInvMass(const Rigidbody &rb, const glm::mat3 &invI,
                         const glm::vec3 &rWorld, const glm::vec3 &dir) {
  glm::vec3 rd = glm::cross(rWorld, dir);
  return rb.invMass + glm::dot(rd, invI * rd);
}

// Applies a positional impulse p at world contact arms r1/r2 (eqs. 6-9).
void ApplyPositionalImpulse(Transform &t1, const Rigidbody &rb1,
                            const glm::mat3 &invI1, const glm::vec3 &r1,
                            Transform &t2, const Rigidbody &rb2,
                            const glm::mat3 &invI2, const glm::vec3 &r2,
                            const glm::vec3 &p) {
  t1.position += p * rb1.invMass;
  t2.position -= p * rb2.invMass;
  ApplyAngularUpdate(t1.rotation, invI1, r1, p, +1.0f);
  ApplyAngularUpdate(t2.rotation, invI2, r2, p, -1.0f);
}

// Applies a velocity change deltaV at the contact points (paper eq. 33).
void ApplyVelocityChange(Rigidbody &rb1, const glm::mat3 &invI1,
                         const glm::vec3 &r1, Rigidbody &rb2,
                         const glm::mat3 &invI2, const glm::vec3 &r2,
                         const glm::vec3 &deltaV) {
  glm::vec3 d = deltaV;
  float len = glm::length(d);
  if (len < 1e-8f)
    return;
  d /= len;

  float denom =
      GeneralizedInvMass(rb1, invI1, r1, d) + GeneralizedInvMass(rb2, invI2, r2, d);
  if (denom <= 1e-12f)
    return;

  glm::vec3 p = (len / denom) * d;
  rb1.linearVelocity += p * rb1.invMass;
  rb2.linearVelocity -= p * rb2.invMass;
  rb1.angularVelocity += invI1 * glm::cross(r1, p);
  rb2.angularVelocity -= invI2 * glm::cross(r2, p);
}

} // namespace

// Positional contact constraint, paper §3.5.
//
//   C  = (p1 - p2) . n          p1/p2 = current world contact points
//   dλ = (-C - a~ λ) / (w1 + w2 + a~),  a~ = compliance / h²
//   p  = dλ n
//
// The contact offsets stored in CollisionResult are in body space (so they can
// be re-evaluated every substep); they are rotated into world space here.
void SolvePositions(CollisionResult &c, ECS::Coordinator *coordinator,
                    float h) {
  auto &rb1 = coordinator->GetComponent<Rigidbody>(c.bodyA);
  auto &t1 = coordinator->GetComponent<Transform>(c.bodyA);
  auto &rb2 = coordinator->GetComponent<Rigidbody>(c.bodyB);
  auto &t2 = coordinator->GetComponent<Transform>(c.bodyB);

  if (rb1.invMass == 0.0f && rb2.invMass == 0.0f)
    return;

  glm::vec3 n = glm::normalize(c.normal);
  glm::vec3 r1 = t1.rotation * c.contactA; // world contact arm
  glm::vec3 r2 = t2.rotation * c.contactB;

  glm::mat3 invI1 = WorldInvInertia(t1, rb1);
  glm::mat3 invI2 = WorldInvInertia(t2, rb2);

  // ---- normal constraint ---------------------------------------------------
  {
    glm::vec3 p1 = t1.position + r1;
    glm::vec3 p2 = t2.position + r2;
    float C = glm::dot(p1 - p2, n); // > 0 while penetrating
    if (C > 0.0f) {                 // complementarity: skip if separated
      const float aHat = COLLISION_COMPLIANCE / (h * h);
      float w1 = GeneralizedInvMass(rb1, invI1, r1, n);
      float w2 = GeneralizedInvMass(rb2, invI2, r2, n);
      float denom = w1 + w2 + aHat;
      if (denom > 1e-12f) {
        float dLambda = (-C - aHat * c.lagrangeN) / denom;
        c.lagrangeN += dLambda;

        ApplyPositionalImpulse(t1, rb1, invI1, r1, t2, rb2, invI2, r2,
                               dLambda * n);
      }
    }
  }

  // ---- static friction, paper §3.5 ----------------------------------------
  // Cancels the tangential drift the two contact points picked up since the
  // start of the substep, bounded by Coulomb's cone |λt| <= µs |λn|.
  {
    glm::vec3 dp1 = (t1.position + t1.rotation * c.contactA) -
                    (rb1.prevPosition + rb1.prevRotation * c.contactA);
    glm::vec3 dp2 = (t2.position + t2.rotation * c.contactB) -
                    (rb2.prevPosition + rb2.prevRotation * c.contactB);
    glm::vec3 dp = dp1 - dp2;
    glm::vec3 dpT = dp - n * glm::dot(dp, n);
    float dpTlen = glm::length(dpT);

    if (dpTlen > 1e-8f) {
      glm::vec3 dT = dpT / dpTlen; // direction to cancel the tangential drift
      float w1 = GeneralizedInvMass(rb1, invI1, r1, dT);
      float w2 = GeneralizedInvMass(rb2, invI2, r2, dT);
      float denom = w1 + w2;
      if (denom > 1e-12f) {
        float dLambdaMag = dpTlen / denom;
        // Coulomb limit: |λt - dλ| <= µs |λn| => dλ bounded by the remaining
        // cone budget in the direction of the correction.
        float limit = c.frictionCoeff * std::abs(c.lagrangeN) + c.lagrangeT;
        dLambdaMag = glm::min(dLambdaMag, glm::max(limit, 0.0f));
        c.lagrangeT -= dLambdaMag;

        ApplyPositionalImpulse(t1, rb1, invI1, r1, t2, rb2, invI2, r2,
                               -dLambdaMag * dT);
      }
    }
  }
}

// Velocity pass, paper §3.6: dynamic Coulomb friction then restitution.
void SolveVelocities(CollisionResult &c, ECS::Coordinator *coordinator,
                     float h) {
  auto &rb1 = coordinator->GetComponent<Rigidbody>(c.bodyA);
  auto &t1 = coordinator->GetComponent<Transform>(c.bodyA);
  auto &rb2 = coordinator->GetComponent<Rigidbody>(c.bodyB);
  auto &t2 = coordinator->GetComponent<Transform>(c.bodyB);

  if (rb1.invMass == 0.0f && rb2.invMass == 0.0f)
    return;

  glm::vec3 n = glm::normalize(c.normal);
  glm::vec3 r1 = t1.rotation * c.contactA;
  glm::vec3 r2 = t2.rotation * c.contactB;

  glm::mat3 invI1 = WorldInvInertia(t1, rb1);
  glm::mat3 invI2 = WorldInvInertia(t2, rb2);

  // Relative normal velocity from before this substep's position projection.
  // vn < 0 means the bodies are approaching (it is d/dt of the penetration).
  glm::vec3 vOld =
      (rb1.prevLinearVelocity + glm::cross(rb1.prevAngularVelocity, r1)) -
      (rb2.prevLinearVelocity + glm::cross(r2, rb2.prevAngularVelocity));
  float vnOld = glm::dot(n, vOld);

  // ---- dynamic friction, eq. (30) ----------------------------------------
  // Δv = -(vt/|vt|) min(µd |fn|, |vt|) with |fn| = |λn| / h².
  {
    glm::vec3 v = PointVelocity(rb1, r1) - PointVelocity(rb2, r2);
    glm::vec3 vT = v - n * glm::dot(n, v);
    float vTlen = glm::length(vT);
    if (vTlen > 1e-6f) {
      float fN = std::abs(c.lagrangeN) / (h * h);
      float magnitude = glm::min(c.frictionCoeff * fN, vTlen);
      ApplyVelocityChange(rb1, invI1, r1, rb2, invI2, r2,
                          -(vT / vTlen) * magnitude);
    }
  }

  // ---- restitution, eq. (34) ---------------------------------------------
  // vn < 0 means approaching, so the target normal velocity is
  // min(e*vnOld, 0) <= 0. With e = 0 this simply zeroes the normal velocity,
  // removing the spurious separating speed the position solve derives from
  // penetration depth (paper §3.6, fig. 13) — critical for initially
  // overlapping bodies.
  {
    const float restitutionThreshold = 2.0f * 9.81f * h;
    float e = (-vnOld > restitutionThreshold) ? c.restitutionCoeff : 0.0f;

    glm::vec3 v = PointVelocity(rb1, r1) - PointVelocity(rb2, r2);
    float vn = glm::dot(n, v);
    float targetVn = glm::min(e * vnOld, 0.0f);
    ApplyVelocityChange(rb1, invI1, r1, rb2, invI2, r2, n * (targetVn - vn));
  }
}
