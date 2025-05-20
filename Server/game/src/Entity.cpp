#include <cmath>
#include <limits>
#include "Entity.h"
#include "ZoneManager.h"
#include "../../common/include/Log.h"

void Entity::MoveTo(float newX, float newY, float newZ) {

    LOG_DEBUG("Entity::MoveTo called: " + std::to_string(id) + " from (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ") to (" + std::to_string(newX) + ", " + std::to_string(newY) + ", " + std::to_string(newZ) + ")");
    float oldX = x, oldY = y;
    int32_t oldZone = zoneId;
    x = newX;
    y = newY;
    z = newZ;
    int32_t newZone = ZoneManager::CalculateZoneId(x, y);
    if (zoneManager && newZone != oldZone) {
        zoneManager->RemoveEntityFromZone(oldZone, id);
        zoneManager->AddEntityToZone(newZone, shared_from_this());
        zoneId = newZone;
    }
}