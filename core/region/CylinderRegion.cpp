//
// Created by OEOTYAN on 2022/06/10.
//
#include "CylinderRegion.h"
#include "MC/Level.hpp"
#include "MC/Dimension.hpp"
#include "ParticleAPI.h"
namespace worldedit {
    CylinderRegion::CylinderRegion(const BoundingBox& region, const int& dim) : Region(region, dim) {
        this->selecting = false;
        hasY = false;
        this->regionType = CYLINDER;
    }

    void CylinderRegion::updateBoundingBox() {
        if (hasY) {
            auto range = Global<Level>->getDimension(dimensionID)->getHeightRange();
            rendertick = 0;
            auto newRadius = (int)(radius);
            boundingBox.min.x = center.x - newRadius;
            boundingBox.min.y = std::max(minY, static_cast<int>(range.min));
            boundingBox.min.z = center.z - newRadius;
            boundingBox.max.x = center.x + newRadius;
            boundingBox.max.y = std::min(maxY, static_cast<int>(range.max) - 1);
            boundingBox.max.z = center.z + newRadius;
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
            radius = static_cast<float>(dis + 0.5);
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

    int CylinderRegion::checkChanges(const std::vector<BlockPos>& changes) {
        int x = 0, z = 0;
        for (auto change : changes) {
            x += abs(change.x);
            z += abs(change.z);
        }
        if (x == z) {
            return x / 2;
        }
        return -1;
    }

    std::pair<std::string, bool> CylinderRegion::expand(const std::vector<BlockPos>& changes) {
        int check = checkChanges(changes);
        if (check == -1) {
            return {"This region can only be expanded isotropically", false};
        } else {
            radius += check;
        }
        for (BlockPos change : changes) {
            if (change.y > 0) {
                maxY += change.y;
            } else {
                minY += change.y;
            }
        }
        updateBoundingBox();
        return {"§aThis region has been expanded", true};
    }

    std::pair<std::string, bool> CylinderRegion::contract(const std::vector<BlockPos>& changes) {
        int check = checkChanges(changes);
        if (check == -1) {
            return {"This region can only be expanded isotropically", false};
        } else {
            radius -= check;
            radius = std::max(radius, 0.5f);
        }
        for (BlockPos change : changes) {
            int height = maxY - minY;
            if (change.y > 0) {
                minY += std::min(change.y, height);
            } else {
                maxY += std::max(change.y, -height);
            }
        }
        updateBoundingBox();
        return {"§aThis region has been contracted", true};
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
            centery.y = minY;
            globalPT().drawCuboid(AABB(centery.toVec3(), centery.toVec3() + Vec3(1, 1 + maxY - minY, 1)), dimensionID,
                                  mce::ColorPalette::GREEN);
            if (radius > 1.0f) {
                centery.x = center.x - (int)radius;
                globalPT().drawCuboid(AABB(centery.toVec3(), centery.toVec3() + Vec3(1, 1 + maxY - minY, 1)),
                                      dimensionID, mce::ColorPalette::YELLOW);
                centery.x = center.x + (int)radius;
                globalPT().drawCuboid(AABB(centery.toVec3(), centery.toVec3() + Vec3(1, 1 + maxY - minY, 1)),
                                      dimensionID, mce::ColorPalette::YELLOW);
                centery.x = center.x;
                centery.z = center.z - (int)radius;
                globalPT().drawCuboid(AABB(centery.toVec3(), centery.toVec3() + Vec3(1, 1 + maxY - minY, 1)),
                                      dimensionID, mce::ColorPalette::YELLOW);
                centery.z = center.z + (int)radius;
                globalPT().drawCuboid(AABB(centery.toVec3(), centery.toVec3() + Vec3(1, 1 + maxY - minY, 1)), dimensionID,
                                      mce::ColorPalette::YELLOW);
                globalPT().drawCircle(BlockPos(center.x, maxY, center.z), ParticleCUI::Direction::POS_Y, radius,
                                      dimensionID, ParticleCUI::PointSize::PX4, 1, 64, mce::ColorPalette::YELLOW);
                if (minY != maxY) {
                    globalPT().drawCircle(BlockPos(center.x, minY, center.z), ParticleCUI::Direction::POS_Y, radius,
                                          dimensionID, ParticleCUI::PointSize::PX4, 1, 64, mce::ColorPalette::YELLOW);
                }
            }
        }
        rendertick--;
    };

}  // namespace worldedit