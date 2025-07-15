#include "cuboid_collider.h"

CuboidCollider::CuboidCollider(glm::vec3& pos, glm::quat& rot, glm::vec3& hExt):
position(pos),rotation(rot),halfExtents(hExt) {}


//maximizes dot product
glm::vec3 const CuboidCollider::support(const glm::vec3 direction) const {
    glm::vec3 localDir = glm::inverse(rotation) * direction;

    glm::vec3 localSupport = glm::vec3(
        localDir.x >= 0 ? halfExtents.x : -halfExtents.x,
        localDir.y >= 0 ? halfExtents.y : -halfExtents.y,
        localDir.z >= 0 ? halfExtents.z : -halfExtents.z
    );

    return position + rotation * localSupport;
}