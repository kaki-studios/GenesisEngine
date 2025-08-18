#pragma once
#include <glm/common.hpp>
#include <glm/ext.hpp>
#include <memory>

enum ColliderType {
  CuboidColliderType = 0,
  ColliderCount,
};

struct Face {
  glm::vec3 normal;
  std::vector<int> indices;
};

class ICollider {
public:
  virtual const glm::vec3 support(const glm::vec3) const = 0;
  ColliderType getType() const { return type; }

  virtual const int getFaceCount() const = 0;
  virtual const Face getFace(int faceIndex) const = 0;

  virtual const int getVertexCount() const = 0;
  virtual const glm::vec3 getVertex(int index) const = 0;

  virtual ~ICollider() = default;
  // TODO maybe store these as pointers to the actual Transform component
  glm::vec3 position;
  glm::quat rotation;
  ColliderType type;
};

class Collider {
public:
  // shared because multiple references to a single Collider might exist (in
  // ECS)
  std::shared_ptr<ICollider> inner;

  // cursed but works
  template <typename T,
            typename = std::enable_if_t<std::is_base_of_v<ICollider, T>>>
  Collider(T &&collider)
      : inner(std::make_shared<std::decay_t<T>>(std::forward<T>(collider))) {}

  Collider(const Collider &) = default;
  Collider(Collider &&) = default;
  Collider &operator=(const Collider &) = default;
  Collider &operator=(Collider &&) = default;
  Collider() = default;
};
