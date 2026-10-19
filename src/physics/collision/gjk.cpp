#include "gjk.h"
#include "glm/geometric.hpp"
#include <algorithm>
#include <cmath>

namespace {

bool Line(Simplex &simplex, glm::vec3 &direction) {
  SupportPoint a = simplex[0];
  SupportPoint b = simplex[1];

  glm::vec3 ab = b.point - a.point;
  glm::vec3 ao = -a.point;

  if (glm::dot(ab, ao) > 0) {
    direction = glm::cross(glm::cross(ab, ao), ab);
    if (glm::length(direction) < 1e-6f) {
      // ab and ao are parallel: pick any perpendicular
      glm::vec3 perp(0.0f, 1.0f, 0.0f);
      if (std::abs(glm::dot(ab, perp)) > 0.9f)
        perp = glm::vec3(1.0f, 0.0f, 0.0f);
      direction = glm::cross(ab, perp);
    }
  } else {
    simplex = {a};
    direction = ao;
  }
  return false;
}

bool Triangle(Simplex &simplex, glm::vec3 &direction) {
  SupportPoint a = simplex[0];
  SupportPoint b = simplex[1];
  SupportPoint c = simplex[2];

  glm::vec3 ab = b.point - a.point;
  glm::vec3 ac = c.point - a.point;
  glm::vec3 ao = -a.point;

  glm::vec3 abc = glm::cross(ab, ac);

  if (glm::dot(glm::cross(abc, ac), ao) > 0) {
    if (glm::dot(ac, ao) > 0) {
      simplex = {a, c};
      direction = glm::cross(glm::cross(ac, ao), ac);
    } else {
      return Line(simplex = {a, b}, direction);
    }
  } else {
    if (glm::dot(glm::cross(ab, abc), ao) > 0) {
      return Line(simplex = {a, b}, direction);
    } else {
      if (glm::dot(abc, ao) > 0) {
        direction = abc;
      } else {
        simplex = {a, c, b};
        direction = -abc;
      }
    }
  }
  return false;
}

bool Tetrahedron(Simplex &simplex, glm::vec3 &direction) {
  SupportPoint a = simplex[0];
  SupportPoint b = simplex[1];
  SupportPoint c = simplex[2];
  SupportPoint d = simplex[3];

  glm::vec3 ab = b.point - a.point;
  glm::vec3 ac = c.point - a.point;
  glm::vec3 ad = d.point - a.point;
  glm::vec3 ao = -a.point;

  glm::vec3 abc = glm::cross(ab, ac);
  glm::vec3 acd = glm::cross(ac, ad);
  glm::vec3 adb = glm::cross(ad, ab);

  // Degenerate tetrahedron (coplanar support points): fall back to the
  // triangle that carries the origin, otherwise the face tests below are
  // meaningless and we would report a bogus containment.
  if (glm::length(abc) < 1e-8f || glm::length(acd) < 1e-8f ||
      glm::length(adb) < 1e-8f) {
    return Triangle(simplex = {a, b, c}, direction);
  }

  if (glm::dot(abc, ao) > 0)
    return Triangle(simplex = {a, b, c}, direction);
  if (glm::dot(acd, ao) > 0)
    return Triangle(simplex = {a, c, d}, direction);
  if (glm::dot(adb, ao) > 0)
    return Triangle(simplex = {a, d, b}, direction);
  return true;
}

} // namespace

bool NearestSimplex(Simplex &simplex, glm::vec3 &direction) {
  switch (simplex.size) {
  case 2:
    return Line(simplex, direction);
  case 3:
    return Triangle(simplex, direction);
  case 4:
    return Tetrahedron(simplex, direction);
  }
  return false;
}

SupportPoint Support(const Collider *aCol, const Collider *bCol,
                     const glm::vec3 &dir) {
  glm::vec3 suppA = aCol->inner->support(dir);
  glm::vec3 suppB = bCol->inner->support(-dir);
  return SupportPoint{suppA - suppB, // Minkowski difference A - B
                      suppA, suppB};
}

static const int MAX_GJK_ITERS = 64;

GJKResult GJKIntersect(const Collider *aCol, const Collider *bCol) {
  // Seed along the line between centres: far better than a fixed +X axis.
  glm::vec3 initialDirection = bCol->inner->position - aCol->inner->position;
  if (glm::length(initialDirection) < 1e-6f)
    initialDirection = glm::vec3(1.0f, 0.0f, 0.0f);

  SupportPoint first = Support(aCol, bCol, initialDirection);
  Simplex simplex;
  simplex.push_front(first);

  glm::vec3 direction = -first.point;
  if (glm::length(direction) < 1e-6f)
    direction = initialDirection;

  for (int i = 0; i < MAX_GJK_ITERS; i++) {
    SupportPoint newPoint = Support(aCol, bCol, direction);

    // No progress along the search direction: the origin is outside.
    if (glm::dot(newPoint.point, direction) < 0.0f)
      return {false, simplex};

    // A repeated support point means the simplex cannot grow any further.
    for (int k = 0; k < simplex.size; ++k) {
      if (glm::length(simplex[k].point - newPoint.point) < 1e-6f) {
        return {true, simplex};
      }
    }

    simplex.push_front(newPoint);

    if (NearestSimplex(simplex, direction))
      return {true, simplex};

    if (glm::length(direction) < 1e-6f)
      return {true, simplex};

    direction = glm::normalize(direction);
  }
  // Ran out of iterations: report a hit so EPA gets a chance to classify it.
  return {true, simplex};
}
