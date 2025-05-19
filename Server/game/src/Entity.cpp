#include "Entity.h"
#include "ZoneManager.h"

void Entity::MoveTo(float newX, float newY, float newZ, void* zoneManagerPtr) {
    float oldX = x, oldY = y;
    int32_t oldZone = zoneId;
    x = newX;
    y = newY;
    z = newZ;
    int32_t newZone = ZoneManager::CalculateZoneId(x, y);
    if (zoneManagerPtr && newZone != oldZone) {
        auto* zoneManager = static_cast<ZoneManager*>(zoneManagerPtr);
        zoneManager->RemoveEntityFromZone(oldZone, id);
        zoneManager->AddEntityToZone(newZone, shared_from_this());
        zoneId = newZone;
    }
}
