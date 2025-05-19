#pragma once
#include <unordered_map>
#include <memory>
#include "Entity.h"

// MobManager manages all entities (players, NPCs, mobs)
class MobManager {
public:
    // Add entity (ownership transferred)
    void AddEntity(std::shared_ptr<Entity> entity);
    // Remove entity by ID
    void RemoveEntity(int32_t id);
    // Get entity by ID
    std::shared_ptr<Entity> GetEntity(int32_t id);
    // Update all entities
    void Update(float deltaTime);
    // Get all entities
    const std::unordered_map<int32_t, std::shared_ptr<Entity>>& GetAllEntities() const { return entities; }
private:
    std::unordered_map<int32_t, std::shared_ptr<Entity>> entities;
};
