#include "collider.h"

class CuboidCollider : public ICollider {
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 halfExtents;
    public:
    CuboidCollider(glm::vec3& position, glm::quat& rotation, glm::vec3& halfExtents);
    glm::vec3 const support(const glm::vec3 direction) const override;

};