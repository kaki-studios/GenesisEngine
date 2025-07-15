#pragma once
#include <glm/common.hpp>
#include <glm/ext.hpp>
#include <memory>

class ICollider {
    public:
    virtual const glm::vec3 support(const glm::vec3) const = 0;
    virtual ~ICollider() = default;
};

class Collider {
    public:
    //shared because multiple references to a single Collider might exist (in ECS)
    std::shared_ptr<ICollider> inner;

//cursed but works
    template<typename T, typename = std::enable_if_t<std::is_base_of_v<ICollider, T>>>
    Collider(T&& collider)
        : inner(std::make_shared<std::decay_t<T>>(std::forward<T>(collider))) {}


 Collider(const Collider&) = default;
    Collider(Collider&&) = default;
    Collider& operator=(const Collider&) = default;
    Collider& operator=(Collider&&) = default;
};