#include "epa.h"
#include "glm/geometric.hpp"
#include <algorithm>
#include <iostream>

static const int EPA_MAX_ITERS = 64;
static const float TOLERANCE = 1e-6f;

namespace {

bool samePoint(const glm::vec3 &a, const glm::vec3 &b) {
  return glm::length(a - b) <= 1e-6f;
}

void calculateEPAFaceProperties(EPAFace &face,
                                const std::vector<SupportPoint> &vertices) {
  const glm::vec3 &a = vertices[face.vertices[0]].point;
  const glm::vec3 &b = vertices[face.vertices[1]].point;
  const glm::vec3 &c = vertices[face.vertices[2]].point;

  glm::vec3 normal = glm::cross(b - a, c - a);
  float len = glm::length(normal);
  if (len <= TOLERANCE) {
    face.normal = glm::vec3(0.0f);
    face.distance = 0.0f;
    return;
  }
  normal /= len;

  // Outward normal: the origin lies inside the Minkowski difference, so a
  // point on the face must have a positive dot product with the outward
  // normal.
  if (glm::dot(normal, a) < 0.0f) {
    normal = -normal;
    std::swap(face.vertices[1], face.vertices[2]);
  }

  face.normal = normal;
  face.distance = glm::dot(normal, a);
}

int findClosestEPAFace(const std::vector<EPAFace> &faces) {
  int closestIndex = -1;
  float minDistance = std::numeric_limits<float>::max();
  for (size_t i = 0; i < faces.size(); ++i) {
    if (faces[i].distance < minDistance) {
      minDistance = faces[i].distance;
      closestIndex = static_cast<int>(i);
    }
  }
  return closestIndex;
}

bool isPointInFrontOfEPAFace(const glm::vec3 &point, const EPAFace &face,
                             const std::vector<SupportPoint> &vertices) {
  const glm::vec3 &facePoint = vertices[face.vertices[0]].point;
  return glm::dot(point - facePoint, face.normal) > TOLERANCE;
}

std::vector<std::pair<int, int>>
findHoleEdges(const std::vector<EPAFace> &faces, const glm::vec3 &newPoint,
              const std::vector<SupportPoint> &vertices) {
  std::vector<std::pair<int, int>> allEdges;
  for (const auto &face : faces) {
    if (isPointInFrontOfEPAFace(newPoint, face, vertices)) {
      allEdges.push_back({face.vertices[0], face.vertices[1]});
      allEdges.push_back({face.vertices[1], face.vertices[2]});
      allEdges.push_back({face.vertices[2], face.vertices[0]});
    }
  }
  std::vector<std::pair<int, int>> edges;
  for (size_t i = 0; i < allEdges.size(); ++i) {
    bool isUnique = true;
    for (size_t j = 0; j < allEdges.size(); ++j) {
      if (i == j)
        continue;
      if (allEdges[i].first == allEdges[j].second &&
          allEdges[i].second == allEdges[j].first) {
        isUnique = false;
        break;
      }
    }
    if (isUnique)
      edges.push_back(allEdges[i]);
  }
  return edges;
}

std::vector<EPAFace> createInitialTetrahedron() {
  std::vector<EPAFace> faces;
  faces.emplace_back(0, 1, 2);
  faces.emplace_back(0, 3, 1);
  faces.emplace_back(1, 3, 2);
  faces.emplace_back(2, 3, 0);
  return faces;
}

bool tetrahedronVolume(const std::vector<SupportPoint> &v, int a, int b, int c,
                       int d) {
  glm::vec3 n = glm::cross(v[b].point - v[a].point, v[c].point - v[a].point);
  return std::abs(glm::dot(n, v[d].point - v[a].point)) > TOLERANCE;
}

// GJK normally hands us a tetrahedron containing the origin. Degenerate cases
// (coplanar / duplicate support points) are repaired here by pulling extra
// support points along the coordinate axes until the hull has volume.
bool ensureTetrahedron(Simplex &simplex, const Collider &a, const Collider &b,
                       std::vector<SupportPoint> &vertices) {
  vertices.assign(simplex.points.begin(), simplex.points.end());
  if (simplex.size == 4 && tetrahedronVolume(vertices, 0, 1, 2, 3))
    return true;
  if (simplex.size < 4)
    return false;

  static const glm::vec3 dirs[6] = {{1, 0, 0},  {-1, 0, 0}, {0, 1, 0},
                                    {0, -1, 0}, {0, 0, 1},  {0, 0, -1}};
  for (const auto &d : dirs) {
    SupportPoint sp = Support(&a, &b, d);
    bool duplicate = false;
    for (const auto &v : vertices) {
      if (samePoint(v.point, sp.point)) {
        duplicate = true;
        break;
      }
    }
    if (duplicate)
      continue;
    vertices.push_back(sp);
    if (tetrahedronVolume(vertices, 0, 1, 2, 3)) {
      simplex.size = 4;
      for (int i = 0; i < 4; ++i)
        simplex.points[i] = vertices[i];
      return true;
    }
    vertices.pop_back();
  }
  return false;
}

// Barycentric contact points on the closest EPA face.
void calculateContactPoint(CollisionResult &res, const EPAFace &face,
                           const std::vector<SupportPoint> &vertices) {
  const SupportPoint &sp0 = vertices[face.vertices[0]];
  const SupportPoint &sp1 = vertices[face.vertices[1]];
  const SupportPoint &sp2 = vertices[face.vertices[2]];

  glm::vec3 p0 = sp0.point;
  glm::vec3 p1 = sp1.point;
  glm::vec3 p2 = sp2.point;
  glm::vec3 closestPoint = face.distance * face.normal;

  glm::vec3 v0 = p1 - p0;
  glm::vec3 v1 = p2 - p0;
  glm::vec3 v2 = closestPoint - p0;

  float dot00 = glm::dot(v0, v0);
  float dot01 = glm::dot(v0, v1);
  float dot02 = glm::dot(v0, v2);
  float dot11 = glm::dot(v1, v1);
  float dot12 = glm::dot(v1, v2);

  float denom = dot00 * dot11 - dot01 * dot01;
  float u = 0.0f, v = 0.0f;
  if (std::abs(denom) > 1e-12f) {
    float invDenom = 1.0f / denom;
    u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    v = (dot00 * dot12 - dot01 * dot02) * invDenom;
  }
  // Clamp to the triangle.
  u = glm::clamp(u, 0.0f, 1.0f);
  v = glm::clamp(v, 0.0f, 1.0f);
  if (u + v > 1.0f) {
    float s = u + v;
    u /= s;
    v /= s;
  }
  float w = 1.0f - u - v;

  res.contactA = w * sp0.suppA + u * sp1.suppA + v * sp2.suppA;
  res.contactB = w * sp0.suppB + u * sp1.suppB + v * sp2.suppB;
}

} // namespace

CollisionResult EPA(const Collider &a, const Collider &b,
                    const Simplex &simplex) {
  CollisionResult result;
  result.valid = false;

  Simplex work = simplex;
  std::vector<SupportPoint> vertices;
  if (!ensureTetrahedron(work, a, b, vertices))
    return result;

  std::vector<EPAFace> faces = createInitialTetrahedron();
  for (auto &face : faces)
    calculateEPAFaceProperties(face, vertices);

  for (int iteration = 0; iteration < EPA_MAX_ITERS; ++iteration) {
    int closestIndex = findClosestEPAFace(faces);
    if (closestIndex < 0)
      return result;
    EPAFace closestFace = faces[closestIndex];

    SupportPoint newSupportPoint = Support(&a, &b, closestFace.normal);
    float supportDistance = glm::dot(newSupportPoint.point, closestFace.normal);

    if (supportDistance - closestFace.distance < TOLERANCE) {
      result.normal = glm::normalize(closestFace.normal);
      result.penetration = glm::max(closestFace.distance, 0.0f);
      calculateContactPoint(result, closestFace, vertices);
      result.valid = true;
      return result;
    }

    std::vector<std::pair<int, int>> holeEdges =
        findHoleEdges(faces, newSupportPoint.point, vertices);
    if (holeEdges.empty())
      return result;

    faces.erase(std::remove_if(faces.begin(), faces.end(),
                               [&](const EPAFace &face) {
                                 return isPointInFrontOfEPAFace(
                                     newSupportPoint.point, face, vertices);
                               }),
                faces.end());

    int newVertexIndex = static_cast<int>(vertices.size());
    vertices.push_back(newSupportPoint);

    for (const auto &edge : holeEdges) {
      EPAFace newFace(edge.first, edge.second, newVertexIndex);
      calculateEPAFaceProperties(newFace, vertices);
      if (glm::length(newFace.normal) < 0.5f)
        continue; // degenerate sliver, drop it
      faces.push_back(newFace);
    }
    if (faces.empty())
      return result;
  }
  return result; // did not converge
}
