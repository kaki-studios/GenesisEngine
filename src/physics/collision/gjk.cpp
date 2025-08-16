#include "gjk.h"
#include "../rendering/cube_renderer.h"
#include "glm/geometric.hpp"
#include <iostream>

bool Line(Simplex &simplex, glm::vec3 &direction) {
  SupportPoint a = simplex[0];
  SupportPoint b = simplex[1];

  glm::vec3 ab = b.point - a.point;
  glm::vec3 ao = -a.point;

  if (glm::dot(ab, ao) > 0) {
    direction = glm::cross(glm::cross(ab, ao), ab);
    if (glm::length(direction) < 0.0001f) {
      // find perpendicular direction
      glm::vec3 perp = glm::vec3(0, 1, 0);
      if (abs(glm::dot(ab, perp)) > 0.9f) {
        perp = glm::vec3(1, 0, 0);
      }
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

  if (glm::dot(abc, ao) > 0) {
    return Triangle(simplex = {a, b, c}, direction);
  }
  if (glm::dot(acd, ao) > 0) {
    return Triangle(simplex = {a, c, d}, direction);
  }
  if (glm::dot(adb, ao) > 0) {
    return Triangle(simplex = {a, d, b}, direction);
  }
  return true;
}

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
  return SupportPoint{suppA - suppB, // Minkowski Difference
                      suppA, suppB};
}

const int MAX_GJK_ITERS = 200;

GJKResult GJKIntersect(const Collider *aCol, const Collider *bCol) {
  glm::vec3 initialDirection = glm::vec3(1.0f, 0.0f, 0.0f);
  SupportPoint first = Support(aCol, bCol, initialDirection);
  Simplex simplex;
  simplex.push_front(first);

  glm::vec3 direction = -first.point;

  for (int i = 0; i < MAX_GJK_ITERS; i++) {
    SupportPoint newPoint = Support(aCol, bCol, direction);

    if (glm::dot(newPoint.point, direction) <= 0) {
      return {false, simplex};
    }
    simplex.push_front(newPoint);

    if (NearestSimplex(simplex, direction)) {
      return {true, simplex};
    }
    if (glm::length(direction) < 0.0001f) {
      // very close to origin
      // std::cout << "degenerate direction!!\n";
      return {true, simplex};
    }
    direction = glm::normalize(direction);
  }
  std::cout << "gjk didn't converge, returning false" << std::endl;
  return {false, simplex};
}
