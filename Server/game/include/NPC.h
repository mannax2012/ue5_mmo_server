#pragma once
#include "Entity.h"

// NPC entity
class NPC : public Entity {
public:
    int32_t npcTypeId;
    int32_t hp;
    int32_t maxHp;
    NPC() { type = EntityType::NPC; }
    void Update(float deltaTime) override;
};
