#include "Player.h"
#include "../../common/include/MySQLClient.h"
#include <vector>
#include <string>
#include "ZoneManager.h"

Player::Player() { type = EntityType::PLAYER; }

void Player::InitFromDB(MySQLClient& mysql, int32_t charId, const std::string& name, int32_t accountId) {
    id = charId;
    this->name = name;
    this->accountId = accountId;
    x = 0; y = 0; z = 0;
    std::string posQuery = "SELECT x, y, z FROM characters WHERE id=" + std::to_string(charId);
    std::vector<std::vector<std::string>> posResult;
    if (mysql.query(posQuery, posResult) && !posResult.empty() && posResult[0].size() == 3) {
        x = std::stof(posResult[0][0]);
        y = std::stof(posResult[0][1]);
        z = std::stof(posResult[0][2]);
    }
    zoneId = ZoneManager::CalculateZoneId(x, y);
}

void Player::Update(float deltaTime) {
    // TODO: Implement player-specific update logic (input, cooldowns, etc)
}
