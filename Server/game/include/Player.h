#pragma once
#include "Entity.h"
#include <string>
#include <memory>
#include <vector>
#include "../../common/include/MySQLClient.h"

// Player entity
class Player : public Entity {
public:
    std::string name;
    int32_t accountId;
    Player();
    void Update(float deltaTime) override;
    void InitFromDB(MySQLClient& mysql, int32_t charId, const std::string& name, int32_t accountId, ZoneManager* zm);
    void SaveToDB(MySQLClient& mysql);
};
