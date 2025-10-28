#include "clipping.h"
#include "collider.h"
#include "glm/geometric.hpp"
#include "physics/collision/epa.h"
#include <iostream>
#include <utility>
#include <vector>

// NOTE: check TODO.md
// NOTE: check:
// https://github.com/felipeek/raw-physics/blob/master/src/physics/clipping.cpp

inline Plane makePlane(const glm::vec3 &nUnit, const glm::vec3 &pointOnPlane) {
  return Plane{glm::normalize(nUnit),
               glm::dot(glm::normalize(nUnit), pointOnPlane)};
}

// Collect world-space vertices of a face (CCW when seen along +face.normal)
inline std::vector<glm::vec3> getWorldFaceVerts(const ICollider &h,
                                                int faceIdx) {
  Face f = h.getFace(faceIdx);
  std::vector<glm::vec3> out;
  out.reserve(f.indices.size());
  for (int idx : f.indices) {
    auto vert = h.getVertex(idx);
    out.push_back(vert);
    std::cout << "vertex " << idx << " for face: " << faceIdx << ": (" << vert.x
              << "), (" << vert.y << "), (" << vert.z << ")\n";
  }
  return out;
}

// Build inward side-planes for a convex face polygon.
// Requires: faceVerts CCW w.r.t. outward normal nOut.
inline std::vector<Plane>
buildInwardSidePlanes(const std::vector<glm::vec3> &faceVerts,
                      const glm::vec3 &nOut) {
  const int N = (int)faceVerts.size();
  std::vector<Plane> planes;
  planes.reserve(N);
  for (int i = 0; i < N; ++i) {
    const glm::vec3 &a = faceVerts[i];
    const glm::vec3 &b = faceVerts[(i + 1) % N];
    glm::vec3 edge = b - a;
    glm::vec3 inward =
        glm::normalize(glm::cross(edge, nOut)); // points into the face
    planes.push_back(makePlane(inward, a));
  }
  return planes;
}

static bool isPointInPlane(const Plane &plane, glm::vec3 pos) {
  return (glm::dot(pos, plane.normal) + plane.d <= 0.0f);
}

// Sutherland–Hodgman: clip polygon against a single half-space (dot(n,x) <= d)
inline std::vector<glm::vec3>
clipAgainstPlane(const std::vector<glm::vec3> &poly, const Plane &P,
                 float eps = 1e-6f) {
  std::vector<glm::vec3> out;
  if (poly.empty())
    return out;
  const int N = (int)poly.size();
  out.reserve(N + 2);

  for (int i = 0; i < N; ++i) {
    const glm::vec3 &A = poly[i];
    const glm::vec3 &B = poly[(i + 1) % N];
    float da = P.signedDistance(A);
    float db = P.signedDistance(B);
    bool inA = (da <= eps);
    bool inB = (db <= eps);

    if (inA && inB) {
      out.push_back(B);
    } else if (inA && !inB) {
      // out.push_back(A);
      float t = da / (da - db + 1e-30f);
      out.push_back(A + t * (B - A));
    } else if (!inA && inB) {
      float t = da / (da - db + 1e-30f);
      out.push_back(A + t * (B - A));
      out.push_back(B);
    }
  }
  return out;
}

inline std::vector<glm::vec3> clipByPlanes(std::vector<glm::vec3> poly,
                                           const std::vector<Plane> &planes) {
  for (const Plane &pl : planes) {
    poly = clipAgainstPlane(poly, pl);
    if (poly.empty())
      break;
  }
  return poly;
}

// Find the face whose outward normal is most aligned with dir (max dot).
inline int findFaceMostAligned(const ICollider &h, const glm::vec3 &dir) {
  int best = 0;
  float bestDot = -std::numeric_limits<float>::infinity();
  const int N = h.getFaceCount();
  for (int i = 0; i < N; ++i) {
    glm::vec3 n = h.getFace(i).normal; // world outward
    float d = glm::dot(n, dir);
    if (d > bestDot) {
      bestDot = d;
      best = i;
    }
  }
  return best;
}

// Reduce to ≤4 points (keeps them spread; simple heuristic).
inline std::vector<glm::vec3> reduceToFour(const std::vector<glm::vec3> &pts) {
  if (pts.size() <= 4)
    return pts;
  glm::vec3 c(0.0f);
  for (auto &p : pts)
    c += p;
  c /= (float)pts.size();

  auto farthestFrom = [&](const glm::vec3 &q) {
    int idx = 0;
    float best = -1.0f;
    for (int i = 0; i < (int)pts.size(); ++i) {
      float d = glm::length(pts[i] - q);
      if (d > best) {
        best = d;
        idx = i;
      }
    }
    return idx;
  };

  int i0 = farthestFrom(c);
  int i1 = farthestFrom(pts[i0]);

  auto triArea2 = [&](int k) {
    return glm::length(glm::cross(pts[i1] - pts[i0], pts[k] - pts[i0]));
  };
  int i2 = i0;
  float a2 = -1.0f;
  for (int i = 0; i < (int)pts.size(); ++i) {
    float a = triArea2(i);
    if (a > a2) {
      a2 = a;
      i2 = i;
    }
  }

  auto sumDist = [&](int k) {
    return glm::length(pts[k] - pts[i0]) + glm::length(pts[k] - pts[i1]) +
           glm::length(pts[k] - pts[i2]);
  };
  int i3 = i0;
  float sBest = -1.0f;
  for (int i = 0; i < (int)pts.size(); ++i) {
    float s = sumDist(i);
    if (s > sBest) {
      sBest = s;
      i3 = i;
    }
  }

  std::vector<glm::vec3> out{pts[i0], pts[i1]};
  if (i2 != i0 && i2 != i1)
    out.push_back(pts[i2]);
  if (i3 != i0 && i3 != i1 && i3 != i2)
    out.push_back(pts[i3]);
  return out;
}

ContactManifold buildContactManifold(const ICollider &A, const ICollider &B,
                                     const CollisionResult &epa) {
  std::cout << "building contact manifold with epa contact point 1: ("
            << epa.contactA.x << ", " << epa.contactA.y << ", "
            << epa.contactA.z << "), and contact point 2: (" << epa.contactB.x
            << ", " << epa.contactB.y << ", " << epa.contactB.z << ")\n";

  ContactManifold m{};

  // EPA normal points B -> A by convention.
  glm::vec3 nEPA = -glm::normalize(epa.normal);
  // if (epa.penetration <= 0.0f)
  //   epa.penetration = -epa.penetration;

  const ICollider *REF = &A;
  const ICollider *INC = &B;

  // Decide reference vs incident:
  // Choose the collider that has a face most aligned with nEPA as reference.
  int aRef = findFaceMostAligned(A, -nEPA);
  int bRef = findFaceMostAligned(
      B, nEPA); // if B is reference, its outward normal aligns with -nEPA

  float aAlign = glm::dot(A.getFace(aRef).normal, -nEPA);
  float bAlign = glm::dot(B.getFace(bRef).normal, nEPA);

  int refIdx = aRef;
  int incIdx = -1;
  glm::vec3 nRefOut =
      A.getFace(aRef).normal; // outward normal of reference face (world)
  glm::vec3 mNormal = nEPA;   // manifold normal (incident -> reference)

  if (bAlign > aAlign) {
    std::cout << "flipping normals\n";
    REF = &B;
    INC = &A;
    refIdx = bRef;
    nRefOut = B.getFace(bRef).normal;
    mNormal = -nEPA; // ensure incident -> reference
  }

  // Incident face is the one most opposed to manifold normal (== aligned with
  // -mNormal). flipped bc don't work
  incIdx = findFaceMostAligned(*INC, mNormal);

  // World-space polygons
  std::vector<glm::vec3> refVerts =
      getWorldFaceVerts(*REF, refIdx); // CCW wrt nRefOut
  std::vector<glm::vec3> incVerts = getWorldFaceVerts(*INC, incIdx);

  // Build inward side planes for reference polygon and clip incident polygon
  std::vector<Plane> sidePlanes = buildInwardSidePlanes(refVerts, nRefOut);
  std::vector<glm::vec3> clipped = clipByPlanes(incVerts, sidePlanes);

  // Project onto the reference face plane (eliminate tiny drift)
  Plane refPlane = makePlane(nRefOut, refVerts[0]);
  const float eps = 1e-5f;

  std::vector<glm::vec3> contacts;
  contacts.reserve(clipped.size());
  for (const glm::vec3 &p : clipped) {
    float sd = refPlane.signedDistance(
        p); // >0 means in front of outward normal (outside)
    std::cout << "sd: " << sd << "\n";
    if (sd <= eps || std::abs(sd) < 0.001f) {
      glm::vec3 proj = p - sd * refPlane.normal;
      contacts.push_back(proj);
      // contacts.push_back(p);
    }
  }

  if (contacts.empty()) {
    // Degenerate case; you can fallback to a single point along the normal.
    // Here we just return empty points but keep normal & penetration.
    std::cout << "no contacts, Degenerate\n";
    m.normal = mNormal;
    m.points.push_back({epa.contactA, epa.contactB});
    return m;
  }

  // Optional: cap to ≤ 4 points for solver stability/perf
  contacts = reduceToFour(contacts);

  m.normal = mNormal;

  // Get the incident face plane
  Face incFace = INC->getFace(incIdx);
  Plane incPlane = makePlane(incFace.normal, incVerts[0]);
  for (auto &v : contacts) {
    // v is the clipped point on reference face

    // Measure distance from v to incident plane along mNormal
    // This is the actual penetration at this contact point
    float distToIncPlane = -incPlane.signedDistance(v);

    // Adjust for the fact we need distance along mNormal, not along incPlane
    // normal
    float cosAngle = glm::dot(mNormal, incPlane.normal);
    float penetration = distToIncPlane / (std::abs(cosAngle) + 1e-6f);
    penetration = std::max(0.0f, penetration);

    glm::vec3 contactA, contactB;
    if (REF == &A) {
      contactA = v;
      contactB = v - mNormal * penetration;
    } else {
      contactB = v;
      contactA = v + mNormal * penetration;
    }
    m.points.push_back({contactA, contactB});
  }

  return m;
}
