#pragma once
#include <vector>
#include <unordered_map>
#include <cstdint>

// Basic Mob structure
struct Mob {
    int32_t id;
    float x, y, z; // Position
    int32_t typeId; // Mob type/template
    int32_t hp;
    int32_t maxHp;
    int32_t level;
    // Add more stats as needed
};

// Mob system for managing all mobs in the world
class MobSystem {
public:
    // Spawn a new mob and return its ID
    int32_t SpawnMob(int32_t typeId, float x, float y, float z, int32_t level);
    // Remove a mob by ID
    void RemoveMob(int32_t mobId);
    // Update all mobs (AI, movement, etc)
    void Update(float deltaTime);
    // Get mob by ID
    Mob* GetMob(int32_t mobId);
    // Get all mobs
    const std::unordered_map<int32_t, Mob>& GetAllMobs() const { return mobs; }
private:
    std::unordered_map<int32_t, Mob> mobs;
    int32_t nextMobId = 1;
};
