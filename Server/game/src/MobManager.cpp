#include "MobManager.h"

void MobManager::AddEntity(std::shared_ptr<Entity> entity) {
    entities[entity->id] = std::move(entity);
}

void MobManager::RemoveEntity(int32_t id) {
    entities.erase(id);
}

std::shared_ptr<Entity> MobManager::GetEntity(int32_t id) {
    auto it = entities.find(id);
    if (it != entities.end()) {
        return it->second;
    }
    return nullptr;
}

void MobManager::Update(float deltaTime) {
    for (auto& pair : entities) {
        if (pair.second) {
            pair.second->Update(deltaTime);
        }
    }
}
