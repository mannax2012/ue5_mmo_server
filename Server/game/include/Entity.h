#pragma once
#include <cstdint>
#include <memory>
#include "../../common/include/BaseServer.h"
enum EntityType { PLAYER, NPC, MOB, ITEM };

// Base class for all entities in the world
class Entity : public std::enable_shared_from_this<Entity> {
public:
    int32_t id;
    float x, y, z;
    EntityType type;
    int32_t zoneId = -1; // Track which zone the entity is in
    virtual ~Entity() = default;
    virtual void Update(float deltaTime) = 0;

    // Move the entity to a new position and handle zone changes
    virtual void MoveTo(float newX, float newY, float newZ);

    static int32_t CalculateZoneId(float x, float y);
};
