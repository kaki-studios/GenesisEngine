#include "entity_manager.h"
#include <assert.h>
#include <unordered_map>

namespace ECS {

// unfortunately some virtual inheritance
class IComponentArray {
public:
  virtual ~IComponentArray() = default;
  virtual void EntityDestroyed(Entity) = 0;
};
// implementation has to be in the same file as it's a template
template <typename T> class ComponentArray : public IComponentArray {
public:
  void InsertData(Entity entity, T compenent) {
    assert(mEntityToIndexMap.find(entity) == mEntityToIndexMap.end() &&
           "Component added to same entity more than once");

    size_t newIndex = mSize;
    mEntityToIndexMap[entity] = newIndex;
    mIndexToEntityMap[newIndex] = entity;
    mComponentArray[newIndex] = compenent;
    ++mSize;
  }
  void RemoveData(Entity entity) {
    assert(mEntityToIndexMap.find(entity) != mEntityToIndexMap.end() &&
           "Removing non-existsent component");
    // Copy element at end into deleted element's place to maintain density
    size_t indexOfRemovedEntity = mEntityToIndexMap[entity];
    size_t indexOfLastElement = mSize - 1;
    mComponentArray[indexOfRemovedEntity] = mComponentArray[indexOfLastElement];
    // Update map to point to moved spot
    Entity entityOfLastElement = mIndexToEntityMap[indexOfLastElement];
    mEntityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
    mIndexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

    mEntityToIndexMap.erase(entity);
    mIndexToEntityMap.erase(indexOfLastElement);

    --mSize;
  }
  T &GetData(Entity entity) {
    assert(mEntityToIndexMap.find(entity) != mEntityToIndexMap.end() &&
           "Retrieving non-existent component.");

    // Return a reference to the entity's component
    return mComponentArray[mEntityToIndexMap[entity]];
  }
  void EntityDestroyed(Entity entity) override {
    if (mEntityToIndexMap.find(entity) != mEntityToIndexMap.end()) {
      // Remove the entity's component if it existed
      RemoveData(entity);
    }
  }

private:
  // packed array of components
  std::array<T, MAX_ENTITIES> mComponentArray;
  std::unordered_map<Entity, size_t> mEntityToIndexMap;
  std::unordered_map<size_t, Entity> mIndexToEntityMap;
  // size of the valid entries in the array
  size_t mSize;
};

} // namespace ECS
