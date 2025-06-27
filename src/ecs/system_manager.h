#include "system.h"
#include <assert.h>
#include <memory>
#include <unordered_map>

namespace ECS {
class SystemManager {
public:
  template <typename T> std::shared_ptr<T> RegisterSystem() {
    const char *typeName = typeid(T).name();
    assert(mSystems.find(typeName) == mSystems.end() &&
           "Registering a system more than once");

    auto system = std::make_shared<T>();
    mSystems.insert({typeName, system});
    return system;
  }
  template <typename T> void SetSignature(Signature signature) {
    const char *typeName = typeid(T).name();
    assert(mSystems.find(typeName) != mSystems.end() &&
           "System used before registered");
    mSignatures.insert({typeName, signature});
  }
  // non generic methods
  void EntityDestroyed(Entity entity);
  void EntitySignatureChanged(Entity entity, Signature entitySignature);

private:
  // Map from system type string pointer to a signature
  std::unordered_map<const char *, Signature> mSignatures{};

  // Map from system type string pointer to a system pointer
  std::unordered_map<const char *, std::shared_ptr<System>> mSystems{};
};
} // namespace ECS
