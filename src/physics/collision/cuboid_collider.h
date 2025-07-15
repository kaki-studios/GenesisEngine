#include "collider.h"

class CuboidCollider : public ICollider {
    glm::vec3 halfExtents;
    public:
    CuboidCollider(const glm::vec3& halfExtents);
    CuboidCollider(glm::vec3&& halfExtents);
    glm::vec3 const support(const glm::vec3 direction) const override;

};