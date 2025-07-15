#include "gjk.h"

bool NearestSimplex(Simplex& simplex, glm::vec3& direction) {
switch (simplex.size) {
        case 2: { // Line segment
            const glm::vec3& A = simplex[0].point;
            const glm::vec3& B = simplex[1].point;
            glm::vec3 AB = B - A;
            glm::vec3 AO = -A;

            if (glm::dot(AB, AO) > 0) {
                direction = glm::cross(glm::cross(AB, AO), AB);
            } else {
                simplex.size = 1;  // Keep only A
                direction = AO;
            }
            return false;
        }

        case 3: { // Triangle
            const glm::vec3& A = simplex[0].point;
            const glm::vec3& B = simplex[1].point;
            const glm::vec3& C = simplex[2].point;

            glm::vec3 AB = B - A;
            glm::vec3 AC = C - A;
            glm::vec3 AO = -A;

            glm::vec3 ABC = glm::cross(AB, AC);

            glm::vec3 ABperp = glm::cross(ABC, AB);
            if (glm::dot(ABperp, AO) > 0) {
                simplex[2] = simplex[1];
                simplex[1] = simplex[0];
                simplex.size = 2;
                direction = glm::cross(glm::cross(AB, AO), AB);
                return false;
            }

            glm::vec3 ACperp = glm::cross(AC, ABC);
            if (glm::dot(ACperp, AO) > 0) {
                simplex[1] = simplex[0];
                simplex.size = 2;
                direction = glm::cross(glm::cross(AC, AO), AC);
                return false;
            }

            direction = ABC * (glm::dot(ABC, AO) > 0 ? 1.0f : -1.0f);
            return false;
        }

        case 4: { // Tetrahedron
            const SupportPoint A = simplex[0];
            const SupportPoint B = simplex[1];
            const SupportPoint C = simplex[2];
            const SupportPoint D = simplex[3];

            glm::vec3 AO = -A.point;

            // Triangle faces
            glm::vec3 AB = B.point - A.point;
            glm::vec3 AC = C.point - A.point;
            glm::vec3 AD = D.point - A.point;

            glm::vec3 ABC = glm::cross(AB, AC);
            glm::vec3 ACD = glm::cross(AC, AD);
            glm::vec3 ADB = glm::cross(AD, AB);

            // Check which face the origin is outside of
            if (glm::dot(ABC, AO) > 0) {
                simplex[1] = simplex[0];
                simplex[2] = B;
                simplex[3] = C;
                simplex.size = 3;
                direction = ABC;
                return false;
            }

            if (glm::dot(ACD, AO) > 0) {
                simplex[1] = simplex[0];
                simplex[2] = C;
                simplex[3] = D;
                simplex.size = 3;
                direction = ACD;
                return false;
            }

            if (glm::dot(ADB, AO) > 0) {
                simplex[1] = simplex[0];
                simplex[2] = D;
                simplex[3] = B;
                simplex.size = 3;
                direction = ADB;
                return false;
            }

            // Origin is inside tetrahedron
            return true;
        }

        default:
            return false;
    }

}

SupportPoint Support(const Collider* a, const Collider* b, const glm::vec3& dir) {
    glm::vec3 suppA = a->inner->support(dir);
    glm::vec3 suppB = b->inner->support(-dir);
    return SupportPoint{
        suppA - suppB,  // Minkowski Difference
        suppA,
        suppB
    };
}


const int MAX_GJK_ITERS = 100;

GJKResult GJKIntersect(const Collider* a, const Collider* b) {
    glm::vec3 initialDirection = glm::vec3(1.0f,0.0f,0.0f);
    SupportPoint first = Support(a, b, initialDirection);
    Simplex simplex;
    simplex.push_front(first);
    
    if(glm::length(first.point) < 0.0001f) {
        return {true, simplex};
    }
    glm::vec3 direction = -first.point;

    for (int i = 0; i<MAX_GJK_ITERS;i++) {
        SupportPoint newPoint = Support(a,b,direction);

        if (glm::dot(newPoint.point, direction) < 0) {
            return {false, simplex};
        }
        simplex.push_front(newPoint);

        bool containsOrigin = NearestSimplex(simplex, direction);

        if (containsOrigin) {
            return {true, simplex};
        }
    }
    return {false, simplex};
}