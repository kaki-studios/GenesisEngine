#include "component_array.h"
#include "entity_manager.h"
#include <memory>
#include <typeindex>
#include <unordered_map>
namespace ECS {
class ComponentManager {
public:
  template <typename T> void RegisterComponent() {
    std::type_index typeName(typeid(T));

    assert(mComponentTypes.find(typeName) == mComponentTypes.end() &&
           "Registering component type more than once");
    mComponentTypes.insert({typeName, mNextComponentType});
    mComponentArrays.insert({typeName, std::make_shared<ComponentArray<T>>()});
    ++mNextComponentType;
  }
  template <typename T> ComponentType GetComponentType() {
    std::type_index typeName(typeid(T));
    assert(mComponentTypes.find(typeName) != mComponentTypes.end() &&
           "Component not registered before use");
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
  // Keyed by std::type_index: typeid(T).name() pointers are not guaranteed to
  // be identical across translation units.
  std::unordered_map<std::type_index, ComponentType> mComponentTypes{};

  std::unordered_map<std::type_index, std::shared_ptr<IComponentArray>>
      mComponentArrays{};

  // The component type to be assigned to the next registered component
  ComponentType mNextComponentType{};

  template <typename T> std::shared_ptr<ComponentArray<T>> GetComponentArray() {
    std::type_index typeName(typeid(T));
    assert(mComponentTypes.find(typeName) != mComponentTypes.end() &&
           "Component not registered before use.");
    return std::static_pointer_cast<ComponentArray<T>>(
        mComponentArrays.at(typeName));
  }
};
} // namespace ECS
