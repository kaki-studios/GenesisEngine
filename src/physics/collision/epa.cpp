#include "epa.h"
#include <iostream>
#include <algorithm>

static const int EPA_MAX_ITERS = 64;
static const float TOLERANCE = 1e-6f;

//helpers
static void calculateFaceProperties(Face& face, const std::vector<SupportPoint>& vertices) {
    const glm::vec3& a = vertices[face.vertices[0]].point;
    const glm::vec3& b = vertices[face.vertices[1]].point;
    const glm::vec3& c = vertices[face.vertices[2]].point;

    glm::vec3 ab = b-a;
    glm::vec3 ac = c-a;
    glm::vec3 normal = glm::normalize(glm::cross(ab,ac));

//normal has to point away from origin
    if(glm::dot(normal,a)<0) {
        normal = -normal;
        //maintain winding order
        std::swap(face.vertices[1], face.vertices[2]);
    }

    face.normal = normal;
    face.distance = glm::dot(normal, a);
}

static int findClosestFace(const std::vector<Face>& faces) {
    int closestIndex = 0;
    float minDistance = faces[0].distance;
    for (size_t i = 1; i< faces.size(); ++i ) {
        if (faces[i].distance < minDistance) {
            minDistance = faces[i].distance;
            closestIndex = static_cast<int>(i);
        }
    }
    return closestIndex;
}

static bool isPointInFrontOfFace(const glm::vec3& point, const Face& face, const std::vector<SupportPoint>& vertices) {
    const glm::vec3& facePoint = vertices[face.vertices[0]].point;
    glm::vec3 toPoint = point - facePoint;
    return glm::dot(toPoint, face.normal) > TOLERANCE;
}

static std::vector<std::pair<int, int>> findHoleEdges(
    const std::vector<Face>& faces, const glm::vec3& newPoint,
    const std::vector<SupportPoint>& vertices) {
    std::vector<std::pair<int, int>> edges;
    std::vector<std::pair<int, int>> allEdges;
    for (const auto& face : faces) {
        if(isPointInFrontOfFace(newPoint, face, vertices)) {
            allEdges.push_back({face.vertices[0],face.vertices[1]});
            allEdges.push_back({face.vertices[1],face.vertices[2]});
            allEdges.push_back({face.vertices[2],face.vertices[0]});
        }
    }
    //find edges that appear only once (boundary edges)
    for(size_t i = 0; i<allEdges.size();++i) {
        bool isUnique = true;
        for (size_t j = 0; j<allEdges.size(); ++j) {
            if(i!=j) {
                if ((allEdges[i].first == allEdges[j].second && 
                allEdges[j].first == allEdges[i].second
                )) {
                    isUnique = false;
                    break;
                }
            }
        }
        if (isUnique) {
            edges.push_back(allEdges[i]);
        }
    }
    return edges;
}

static std::vector<Face> createInitialTetrahedron() {
    std::vector<Face> faces;
    faces.emplace_back(0,1,2);
    faces.emplace_back(0,3,1);
    faces.emplace_back(1,3,2);
    faces.emplace_back(2,3,0);
    return faces;
}

static bool validateSimplex(const Simplex& simplex) {
    if(simplex.size!=4) return false;

    // Check if points are coplanar
    glm::vec3 v1 = simplex[1].point - simplex[0].point;
    glm::vec3 v2 = simplex[2].point - simplex[0].point;
    glm::vec3 v3 = simplex[3].point - simplex[0].point;
    
    glm::vec3 normal = glm::cross(v1, v2);
    float volume = std::abs(glm::dot(normal, v3));
    
    return volume > TOLERANCE;
}

//calculates JUST the contact points (contactA and contactB)
static void calculateContactPoint(CollisionResult& res, const Face& closestFace, const std::vector<SupportPoint>& vertices) {
    const SupportPoint& sp0 = vertices[closestFace.vertices[0]];
    const SupportPoint& sp1 = vertices[closestFace.vertices[1]];
    const SupportPoint& sp2 = vertices[closestFace.vertices[2]];

    glm::vec3 p0 = sp0.point;
    glm::vec3 p1 = sp1.point;
    glm::vec3 p2 = sp2.point;

    glm::vec3 normal = closestFace.normal;
    glm::vec3 closestPoint = closestFace.distance * normal;

    glm::vec3 v0 = p1-p0;
    glm::vec3 v1 = p2-p0;
    glm::vec3 v2 = closestPoint - p0;

    float dot00 = glm::dot(v0, v0);
    float dot01 = glm::dot(v0, v1);
    float dot02 = glm::dot(v0, v2);
    float dot11 = glm::dot(v1, v1);
    float dot12 = glm::dot(v1, v2);
    
    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    float w = 1.0f - u - v;
    
    // Clamp barycentric coordinates to ensure they're within the triangle
    if (u < 0) { u = 0; v = glm::clamp(v, 0.0f, 1.0f); w = 1.0f - v; }
    if (v < 0) { v = 0; u = glm::clamp(u, 0.0f, 1.0f); w = 1.0f - u; }
    if (w < 0) { w = 0; u = glm::clamp(u / (u + v), 0.0f, 1.0f); v = 1.0f - u; }

    std::cout << "u + v + w: " << u+v+w <<"\n";
    
    // Calculate contact points using barycentric coordinates
    glm::vec3 contactA = w * sp0.suppA + u * sp1.suppA + v * sp2.suppA;
    glm::vec3 contactB = w * sp0.suppB + u * sp1.suppB + v * sp2.suppB;

    res.contactA = contactA;
    res.contactB = contactB;
}

CollisionResult EPA(const Collider& a, const Collider& b, const Simplex& simplex) {
    if(!validateSimplex(simplex)) {
        return CollisionResult();
    }

    std::vector<SupportPoint> vertices(simplex.points.begin(), simplex.points.end());
    std::vector<Face> faces = createInitialTetrahedron();

    for (auto& face: faces) {
        calculateFaceProperties(face, vertices);
    }

    for (int iteration = 0; iteration < EPA_MAX_ITERS; ++iteration) {
        int closestFaceIndex = findClosestFace(faces);
        const Face& closestFace = faces[closestFaceIndex];

        SupportPoint newSupportPoint = Support(&a, &b, closestFace.normal);
        float supportDistance = glm::dot(newSupportPoint.point, closestFace.normal);

//difference always positive, no need for abs()
        if(supportDistance - closestFace.distance < TOLERANCE) {
            //we've found the minimum penetration
            CollisionResult result;
            calculateContactPoint(result, closestFace, vertices);
            result.normal = closestFace.normal;
            result.penetration = supportDistance;
            result.valid = true;
            return result;
        }
        std::vector<std::pair<int, int>> holeEdges = findHoleEdges(faces, newSupportPoint.point, vertices);
        //remove faces that can "see" the support point
        faces.erase(
            std::remove_if(
                faces.begin(), faces.end(),
                [&](const Face& face) {
                    return isPointInFrontOfFace(newSupportPoint.point, face,vertices);
                }

            ),
            faces.end()
        );

        int newVertexIndex = static_cast<int>(vertices.size());
        vertices.push_back(newSupportPoint); //newVertexIndex now points to newSupportPoint

        for (const auto& edge : holeEdges) {
            Face newFace(edge.first, edge.second, newVertexIndex);
            calculateFaceProperties(newFace, vertices);
            faces.push_back(newFace);
        }
    }
    std::cout  << "EPA failed to converge\n";
    return CollisionResult();

}