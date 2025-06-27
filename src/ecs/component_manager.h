#include "component_array.h"
#include "entity_manager.h"
#include <memory>
#include <unordered_map>
namespace ECS {
class ComponentManager {
public:
  template <typename T> void RegisterComponent() {
    const char *typeName = typeid(T).name();

    assert(mComponentTypes.find(typeName) == mComponentTypes.end() &&
           "Registering component type more than once");
    // add to component type map
    mComponentTypes.insert({typeName, mNextComponentType});
    // add to component array map
    mComponentArrays.insert({typeName, std::make_shared<ComponentArray<T>>()});
    ++mNextComponentType;
  }
  template <typename T> ComponentType GetComponentType() {
    const char *typeName = typeid(T).name();
    assert(mComponentTypes.find(typeName) != mComponentTypes.end() &&
           "Component not registered before use");
    // return this component's type, used for creating signatures
    return mComponentTypes[typeName];
  }
  template <typename T> void AddComponent(Entity entity, T component) {
    GetComponentArray<T>()->InsertData(entity, component);
  }
  template <typename T> void RemoveComponent(Entity entity) {
    GetComponentArray<T>()->RemoveData(entity);
  }
  template <typename T> T &GetComponent(Entity entity) {
    return GetComponentArray<T>()->GetData(entity);
  }
  void EntityDestroyed(Entity entity) {
    // Notify each component array that an entity has been destroyed
    // If it has a component for that entity, it will remove it
    for (auto const &pair : mComponentArrays) {
      auto const &component = pair.second;

      component->EntityDestroyed(entity);
    }
  }

private:
  // Map from type string pointer to a component type
  std::unordered_map<const char *, ComponentType> mComponentTypes{};

  // Map from type string pointer to a component array
  std::unordered_map<const char *, std::shared_ptr<IComponentArray>>
      mComponentArrays{};

  // The component type to be assigned to the next registered component -
  // starting at 0
  ComponentType mNextComponentType{};

  // Convenience function to get the statically casted pointer to the
  // ComponentArray of type T.
  template <typename T> std::shared_ptr<ComponentArray<T>> GetComponentArray() {
    const char *typeName = typeid(T).name();

    assert(mComponentTypes.find(typeName) != mComponentTypes.end() &&
           "Component not registered before use.");

    return std::static_pointer_cast<ComponentArray<T>>(
        mComponentArrays[typeName]);
  }
};
} // namespace ECS
