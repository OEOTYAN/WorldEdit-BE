#include "CylinderRegion.h"
#include "MC/Level.hpp"
#include "MC/Dimension.hpp"
namespace worldedit {
    CylinderRegion::CylinderRegion(const BoundingBox& region, const int& dim)
        : Region(region, dim) {
        this->selecting = false;
        hasY = false;
        this->regionType = CYLINDER;
    }

    void CylinderRegion::updateBoundingBox() {
        if (hasY) {
            auto range = Level::getDimension(dimensionID)->getHeightRange();
            rendertick = 0;
            auto newRadius = (int)(radius);
            boundingBox.bpos1.x = center.x - newRadius;
            boundingBox.bpos1.y = std::max(minY, static_cast<int>(range.min));
            boundingBox.bpos1.z = center.z - newRadius;
            boundingBox.bpos2.x = center.x + newRadius;
            boundingBox.bpos2.y =
                std::min(maxY, static_cast<int>(range.max) - 1);
            boundingBox.bpos2.z = center.z + newRadius;
        }
    }

    bool CylinderRegion::setY(int y) {
        if (!hasY) {
            minY = y;
            maxY = y;
            hasY = true;
            return true;
        } else if (y < minY) {
            minY = y;
            return true;
        } else if (y > maxY) {
            maxY = y;
            return true;
        }
        return false;
    }

    bool CylinderRegion::setMainPos(const BlockPos& pos, const int& dim) {
        selecting = true;
        hasY = false;
        selectedCenter = false;
        selectedRadius = false;
        dimensionID = dim;
        center = pos;
        radius = 0.5;
        setY(pos.y);
        selectedCenter = true;
        selectedRadius = false;
        updateBoundingBox();
        return true;
    }

    bool CylinderRegion::setVicePos(const BlockPos& pos, const int& dim) {
        if (!selecting || dim != dimensionID || !selectedCenter) {
            return false;
        }
        double disx = pos.x - center.x;
        double disz = pos.z - center.z;
        double dis = sqrt(disx * disx + disz * disz);
        if (dis >= radius) {
            radius = static_cast<double>(dis) + 0.5f;
        }
        selectedRadius = true;
        setY(pos.y);
        updateBoundingBox();
        return true;
    }

    bool CylinderRegion::contains(const BlockPos& pos) {
        if (pos.y < minY || pos.y > maxY) {
            return false;
        }
        double disx = pos.x - center.x;
        double disz = pos.z - center.z;
        return disx * disx + disz * disz <= radius * radius;
    }

    std::pair<std::string, bool> CylinderRegion::shift(const BlockPos& change) {
        center = center + change;
        maxY += change.y;
        minY += change.y;
        updateBoundingBox();
        return {"§aThis region has been shifted", true};
    }

    void CylinderRegion::renderRegion() {
        if (selecting && dimensionID >= 0 && rendertick <= 0) {
            rendertick = 40;
            BlockPos centery = center;
            centery.y = maxY;
            worldedit::spawnCuboidParticle(
                {centery.toVec3(), centery.toVec3() + Vec3::ONE},
                GRAPHIC_COLOR::GREEN, dimensionID);
            centery.y = minY;
            worldedit::spawnCuboidParticle(
                {centery.toVec3(), centery.toVec3() + Vec3::ONE},
                GRAPHIC_COLOR::GREEN, dimensionID);
            if (radius > 1.0f) {
                centery = center;
                centery.y = maxY;
                centery.x = center.x - (int)radius;
                worldedit::spawnCuboidParticle(
                    {centery.toVec3(), centery.toVec3() + Vec3::ONE},
                    GRAPHIC_COLOR::GREEN, dimensionID);
                centery.x = center.x + (int)radius;
                worldedit::spawnCuboidParticle(
                    {centery.toVec3(), centery.toVec3() + Vec3::ONE},
                    GRAPHIC_COLOR::GREEN, dimensionID);
                centery.x = center.x;
                centery.z = center.z - (int)radius;
                worldedit::spawnCuboidParticle(
                    {centery.toVec3(), centery.toVec3() + Vec3::ONE},
                    GRAPHIC_COLOR::GREEN, dimensionID);
                centery.z = center.z + (int)radius;
                worldedit::spawnCuboidParticle(
                    {centery.toVec3(), centery.toVec3() + Vec3::ONE},
                    GRAPHIC_COLOR::GREEN, dimensionID);
                if (minY != maxY) {
                    centery.z = center.z;
                    centery.y = minY;
                    centery.x = center.x - (int)radius;
                    worldedit::spawnCuboidParticle(
                        {centery.toVec3(), centery.toVec3() + Vec3::ONE},
                        GRAPHIC_COLOR::GREEN, dimensionID);
                    centery.x = center.x + (int)radius;
                    worldedit::spawnCuboidParticle(
                        {centery.toVec3(), centery.toVec3() + Vec3::ONE},
                        GRAPHIC_COLOR::GREEN, dimensionID);
                    centery.x = center.x;
                    centery.z = center.z - (int)radius;
                    worldedit::spawnCuboidParticle(
                        {centery.toVec3(), centery.toVec3() + Vec3::ONE},
                        GRAPHIC_COLOR::GREEN, dimensionID);
                    centery.z = center.z + (int)radius;
                    worldedit::spawnCuboidParticle(
                        {centery.toVec3(), centery.toVec3() + Vec3::ONE},
                        GRAPHIC_COLOR::GREEN, dimensionID);
                }
            }
        }
        rendertick--;
    };

}  // namespace worldedit