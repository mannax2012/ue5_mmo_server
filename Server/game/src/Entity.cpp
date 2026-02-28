#include <cmath>
#include <limits>
#include "Entity.h"
#include "ZoneManager.h"
#include "Player.h"
#include "../../common/include/Log.h"

void Entity::MoveTo(float newX, float newY, float newZ) {

    LOG_DEBUG_EXT("Entity::MoveTo called: " + std::to_string(id) + " from (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ") to (" + std::to_string(newX) + ", " + std::to_string(newY) + ", " + std::to_string(newZ) + ")");
    float oldX = x, oldY = y;
    int32_t oldZone = zoneId;
    x = newX;
    y = newY;
    z = newZ;

    ZoneManager* zm = ZoneManager::Get();
    int32_t newZone = zm ? zm->CalculateZoneId(x, y) : 0;
    if (zoneManager && newZone != oldZone) {
        LOG_DEBUG("Entity::MoveTo zone change: " + std::to_string(id) + " from zone " + std::to_string(oldZone) + " to zone " + std::to_string(newZone));
        zoneManager->RemoveEntityFromZone(oldZone, id);
        zoneManager->AddEntityToZone(newZone, shared_from_this());
        zoneId = newZone;
    }
    LOG_DEBUG_EXT("Entity::MoveTo completed: " + std::to_string(id) + " now at (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ") in zone " + std::to_string(zoneId));
   if (zoneManager) {
    auto nearbyZones = zoneManager->GetNearbyZones(x, y, z);
    for (int32_t zId : nearbyZones) {
        const auto& players = zoneManager->GetPlayersInZone(zId);
        for (const auto& playerPair : players) {
            const auto& playerPtr = playerPair.second;
            if (playerPtr && playerPtr->session) {
                S_Move movePacket{};
                movePacket.header.packetId = PACKET_S_MOVE;
                movePacket.entityId = id;
                movePacket.entityType = static_cast<int8_t>(type);
                movePacket.shardId = zoneId;
                movePacket.x = x;
                movePacket.y = y;
                movePacket.z = z;
                movePacket.yaw = 0.0f; // Set actual yaw if available
                if (zoneManager->server) {
                    zoneManager->server->sendToClient(&movePacket, sizeof(movePacket), playerPtr->session->clientSock);
                }
            }
        }
    }
}
    

}