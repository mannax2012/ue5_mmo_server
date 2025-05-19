#include "ZoneManager.h"
#include "Entity.h"
int ZoneManager::zoneSize = 100;
void ZoneManager::AddEntityToZone(int32_t zoneId, std::shared_ptr<Entity> entity) {
    zoneEntities[zoneId].insert(entity->id);
}

void ZoneManager::RemoveEntityFromZone(int32_t zoneId, int32_t entityId) {
    auto it = zoneEntities.find(zoneId);
    if (it != zoneEntities.end()) {
        it->second.erase(entityId);
        if (it->second.empty()) {
            zoneEntities.erase(it);
        }
    }
}

const std::set<int32_t>& ZoneManager::GetEntitiesInZone(int32_t zoneId) const {
    static const std::set<int32_t> emptySet;
    auto it = zoneEntities.find(zoneId);
    if (it != zoneEntities.end()) {
        return it->second;
    }
    return emptySet;
}

// Get nearby zones based on the entity's position
// This function returns a vector of zone IDs that are within a 3x3 grid around the entity's position
// The center zone is the one the entity is currently in, and the surrounding zones are also included
std::vector<int32_t> ZoneManager::GetNearbyZones(float x, float y, float z) const {

    int zx = static_cast<int>(x) / GetZoneSize();
    int zy = static_cast<int>(y) / GetZoneSize();
    std::vector<int32_t> result;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            int zoneId = (zx + dx) + 1000 * (zy + dy);
            if (zoneEntities.count(zoneId)) {
                result.push_back(zoneId);
            }
        }
    }
    return result;
}



int32_t ZoneManager::CalculateZoneId(float x, float y) {

    return static_cast<int32_t>(x) / GetZoneSize() + 1000 * (static_cast<int32_t>(y) / GetZoneSize());
}

void ZoneManager::SetZoneSize(int size) {
    zoneSize = size;
}

int ZoneManager::GetZoneSize() {
    return zoneSize;
}
