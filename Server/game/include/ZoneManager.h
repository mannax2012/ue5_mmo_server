#pragma once
#include <unordered_map>
#include <vector>
#include <set>
#include <memory>

class Entity;

// ZoneManager splits the world into zones for network culling
class ZoneManager {
public:
    // Assign entity to a zone
    void AddEntityToZone(int32_t zoneId, std::shared_ptr<Entity> entity);
    // Remove entity from a zone
    void RemoveEntityFromZone(int32_t zoneId, int32_t entityId);
    // Get all entities in a zone
    const std::set<int32_t>& GetEntitiesInZone(int32_t zoneId) const;
    // Get zones near a position (for AOI/network culling)
    std::vector<int32_t> GetNearbyZones(float x, float y, float z) const;
    // Calculate zone id from position
    static int32_t CalculateZoneId(float x, float y);
    // Set zone size from config
    static void SetZoneSize(int size);
    static int GetZoneSize();
private:
    static int zoneSize;
    std::unordered_map<int32_t, std::set<int32_t>> zoneEntities; // zoneId -> entity IDs
};
