//
// Created by OEOTYAN on 2021/2/8.
//

#include "SphereRegion.h"
#include "MC/Level.hpp"
#include "MC/Dimension.hpp"
namespace worldedit {
    void SphereRegion::updateBoundingBox() {
        auto range = Level::getDimension(dimensionID)->getHeightRange();
        rendertick = 0;
        auto newRadius = (int)(radius);
        boundingBox.bpos1.x = center.x - newRadius;
        boundingBox.bpos1.y =
            std::max(center.y - newRadius, static_cast<int>(range.min));
        boundingBox.bpos1.z = center.z - newRadius;
        boundingBox.bpos2.x = center.x + newRadius;
        boundingBox.bpos2.y =
            std::min(center.y + newRadius, static_cast<int>(range.max) - 1);
        boundingBox.bpos2.z = center.z + newRadius;
    }

    bool SphereRegion::setMainPos(const BlockPos& pos, const int& dim) {
        selecting = true;
        dimensionID = dim;
        center = pos;
        radius = 0.5;
        updateBoundingBox();
        return true;
    }

    bool SphereRegion::setVicePos(const BlockPos& pos, const int& dim) {
        if (!selecting || dim != dimensionID) {
            return false;
        }
        float dis = (float)pos.distanceTo(center);
        if (dis > radius) {
            radius = dis + 0.5f;
            updateBoundingBox();
            return true;
        }
        return false;
    }

    int SphereRegion::checkChanges(const std::vector<BlockPos>& changes) {
        BlockPos tmp;
        int x = 0, y = 0, z = 0;
        for (auto change : changes) {
            x += abs(change.x);
            y += abs(change.y);
            z += abs(change.z);
        }
        tmp = {x, y, z};
        if ((tmp.x) == (tmp.y) && (tmp.y) == (tmp.z)) {
            return (tmp.x) / 2;
        }
        return -1;
    }

    std::pair<std::string, bool> SphereRegion::expand(
        const std::vector<BlockPos>& changes) {
        int check = checkChanges(changes);
        if (check == -1) {
            return {"This region can only be expanded isotropically", false};
        } else {
            radius += check;
            updateBoundingBox();
        }

        return {"§aThis region has been expanded", true};
    }

    std::pair<std::string, bool> SphereRegion::contract(
        const std::vector<BlockPos>& changes) {
        int check = checkChanges(changes);
        if (check == -1) {
            return {"This region can only be contracted isotropically", false};
        } else {
            radius += check;
            updateBoundingBox();
        }
        return {"§aThis region has been contracted", true};
    }

    std::pair<std::string, bool> SphereRegion::shift(const BlockPos& change) {
        center = center + change;
        updateBoundingBox();
        return {"§aThis region has been shifted", true};
    }

    bool SphereRegion::contains(const BlockPos& pos) {
        return pos.distanceTo(center) <= radius;
    }

    SphereRegion::SphereRegion(const BoundingBox& region, const int& dim)
        : Region(region, dim) {
        this->selecting = false;
        this->regionType = SPHERE;
    }
    void SphereRegion::renderRegion() {
        if (selecting && dimensionID >= 0 && rendertick <= 0) {
            rendertick = 40;
            worldedit::spawnCuboidParticle(
                {center.toVec3(), center.toVec3() + Vec3::ONE},
                GRAPHIC_COLOR::GREEN, dimensionID);
        }
        rendertick--;
    };
}  // namespace worldedit