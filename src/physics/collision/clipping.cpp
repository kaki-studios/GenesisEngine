#include "clipping.h"
#include "collider.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>
#include <vector>

namespace {

// Plane: dot(normal, x) + d = 0, `normal` unit.
struct Plane {
  glm::vec3 normal;
  float d;
  float signedDistance(const glm::vec3 &x) const {
    return glm::dot(normal, x) + d;
  }
};

Plane makePlane(const glm::vec3 &nUnit, const glm::vec3 &pointOnPlane) {
  glm::vec3 n = glm::normalize(nUnit);
  return Plane{n, -glm::dot(n, pointOnPlane)};
}

std::vector<glm::vec3> getWorldFaceVerts(const ICollider &h, int faceIdx) {
  Face f = h.getFace(faceIdx);
  std::vector<glm::vec3> out;
  out.reserve(f.indices.size());
  for (int idx : f.indices)
    out.push_back(h.getVertex(idx));
  return out;
}

// Inward half-space planes of a convex face polygon.
// NOTE: the box face loops in CuboidCollider::getFace are wound *clockwise*
// w.r.t. their outward normal, so cross(edge, nOut) points into the face.
// Verified for every face — do not "fix" the winding without changing this.
std::vector<Plane>
buildInwardSidePlanes(const std::vector<glm::vec3> &faceVerts,
                      const glm::vec3 &nOut) {
  const int N = (int)faceVerts.size();
  std::vector<Plane> planes;
  planes.reserve(N);
  for (int i = 0; i < N; ++i) {
    const glm::vec3 &a = faceVerts[i];
    const glm::vec3 &b = faceVerts[(i + 1) % N];
    glm::vec3 inward = glm::normalize(glm::cross(b - a, nOut));
    planes.push_back(makePlane(inward, a));
  }
  return planes;
}

// Sutherland–Hodgman against one half-space: keeps dot(n, x) + d <= 0.
std::vector<glm::vec3> clipAgainstPlane(const std::vector<glm::vec3> &poly,
                                        const Plane &P, float eps = 1e-6f) {
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
      float t = da / (da - db);
      out.push_back(A + t * (B - A));
    } else if (!inA && inB) {
      float t = da / (da - db);
      out.push_back(A + t * (B - A));
      out.push_back(B);
    }
  }
  return out;
}

std::vector<glm::vec3> clipByPlanes(std::vector<glm::vec3> poly,
                                    const std::vector<Plane> &planes) {
  for (const Plane &pl : planes) {
    poly = clipAgainstPlane(poly, pl);
    if (poly.empty())
      break;
  }
  return poly;
}

// Face whose outward normal is most aligned with dir.
int findFaceMostAligned(const ICollider &h, const glm::vec3 &dir) {
  int best = 0;
  float bestDot = -std::numeric_limits<float>::infinity();
  const int N = h.getFaceCount();
  for (int i = 0; i < N; ++i) {
    float d = glm::dot(h.getFace(i).normal, dir);
    if (d > bestDot) {
      bestDot = d;
      best = i;
    }
  }
  return best;
}

// Reduce to <= 4 well spread points.
std::vector<glm::vec3> reduceToFour(const std::vector<glm::vec3> &pts) {
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

  int i2 = i0;
  float a2 = -1.0f;
  for (int i = 0; i < (int)pts.size(); ++i) {
    float a = glm::length(glm::cross(pts[i1] - pts[i0], pts[i] - pts[i0]));
    if (a > a2) {
      a2 = a;
      i2 = i;
    }
  }

  int i3 = i0;
  float sBest = -1.0f;
  for (int i = 0; i < (int)pts.size(); ++i) {
    float s = glm::length(pts[i] - pts[i0]) + glm::length(pts[i] - pts[i1]) +
              glm::length(pts[i] - pts[i2]);
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

} // namespace

ContactManifold buildContactManifold(const ICollider &A, const ICollider &B,
                                     const CollisionResult &epa) {
  ContactManifold m{};
  // EPA's outward face normal of the Minkowski difference (A - B) points from
  // A towards B (see epa.cpp).
  glm::vec3 nAB = glm::normalize(epa.normal);

  // Reference face: the face of either body that points most towards the other.
  int aRef = findFaceMostAligned(A, nAB);
  int bRef = findFaceMostAligned(B, -nAB);
  float aAlign = glm::dot(A.getFace(aRef).normal, nAB);
  float bAlign = glm::dot(B.getFace(bRef).normal, -nAB);

  bool refIsA = (aAlign >= bAlign);
  const ICollider &REF = refIsA ? A : B;
  const ICollider &INC = refIsA ? B : A;
  int refIdx = refIsA ? aRef : bRef;
  glm::vec3 nRefOut = REF.getFace(refIdx).normal;
  // incident face is the one most opposed to the reference face
  int incIdx = findFaceMostAligned(INC, -nRefOut);

  std::vector<glm::vec3> refVerts = getWorldFaceVerts(REF, refIdx);
  std::vector<glm::vec3> incVerts = getWorldFaceVerts(INC, incIdx);
  if (refVerts.empty() || incVerts.empty())
    return m;

  std::vector<Plane> sidePlanes = buildInwardSidePlanes(refVerts, nRefOut);
  std::vector<glm::vec3> clipped = clipByPlanes(incVerts, sidePlanes);
  if (clipped.empty())
    return m;

  // Keep only points on or behind the reference face, projected onto its plane.
  Plane refPlane = makePlane(nRefOut, refVerts[0]);
  std::vector<glm::vec3> contacts;
  contacts.reserve(clipped.size());
  for (const glm::vec3 &p : clipped) {
    float sd = refPlane.signedDistance(p);
    if (sd <= 1e-3f)
      contacts.push_back(p - sd * refPlane.normal);
  }
  if (contacts.empty())
    return m;

  contacts = reduceToFour(contacts);

  // Direction leading from the reference face to the incident body's surface.
  glm::vec3 toIncident = refIsA ? -nAB : nAB;
  Face incFace = INC.getFace(incIdx);
  Plane incPlane = makePlane(incFace.normal, incVerts[0]);
  float cosAngle = glm::dot(incPlane.normal, toIncident);
  if (std::abs(cosAngle) < 1e-4f)
    return m; // faces nearly perpendicular: no usable depth

  m.normal = nAB;
  for (const glm::vec3 &v : contacts) {
    // Depth from the reference point to the incident plane, along toIncident.
    float depth = -incPlane.signedDistance(v) / cosAngle;
    if (depth < -1e-4f)
      continue;
    depth = glm::max(depth, 0.0f);

    glm::vec3 pA, pB;
    if (refIsA) {
      pA = v;
      pB = v + toIncident * depth;
    } else {
      pB = v;
      pA = v + toIncident * depth;
    }
    // Guarantee the invariant the solver relies on: (pA - pB) . nAB >= 0.
    if (glm::dot(pA - pB, nAB) < -1e-4f)
      continue;
    m.points.push_back({pA, pB});
  }

  if (m.points.empty()) {
    // Degenerate clipping: fall back to EPA's barycentric contact pair.
    m.points.push_back({epa.contactA, epa.contactB});
  }

  return m;
}
