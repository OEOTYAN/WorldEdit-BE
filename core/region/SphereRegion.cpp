//
// Created by OEOTYAN on 2021/2/8.
//

#include "SphereRegion.h"
#include "MC/Level.hpp"
#include "eval/Eval.h"
#include "MC/Dimension.hpp"
namespace worldedit {
    void SphereRegion::updateBoundingBox() {
        auto range = Global<Level>->getDimension(dimensionID)->getHeightRange();
        rendertick = 0;
        auto newRadius = (int)(ceil(radius));
        boundingBox.min.x = center.x - newRadius;
        boundingBox.min.y = std::max(center.y - newRadius, static_cast<int>(range.min));
        boundingBox.min.z = center.z - newRadius;
        boundingBox.max.x = center.x + newRadius;
        boundingBox.max.y = std::min(center.y + newRadius, static_cast<int>(range.max) - 1);
        boundingBox.max.z = center.z + newRadius;
    }

    void SphereRegion::forEachBlockUVInRegion(const std::function<void(const BlockPos&, double, double)>& todo) {
        this->forEachBlockInRegion([&](const BlockPos& pos) {
            int counts = 0;
            for (auto& calPos : pos.getNeighbors()) {
                counts += this->contains(calPos);
            }
            if (counts < 6) {
                double y = (pos.y - center.y) / radius;
                if (abs(y) > 0.8) {
                    todo(pos, (atan2(pos.z - center.z, pos.x - center.x) + M_PI) / (M_PI * 2),
                         acos(clamp(
                             -sign(y) *
                                 sqrt(std::max(0.0, pow2(radius) - pow2(pos.z - center.z) - pow2(pos.x - center.x))) /
                                 radius,
                             -1.0, 1.0)) *
                             M_1_PI);
                } else {
                    todo(pos, (atan2(pos.z - center.z, pos.x - center.x) + M_PI) / (M_PI * 2), acos(-y) * M_1_PI);
                }
            }
        });
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
        auto dis = pos.distanceTo(center);
        if (dis > radius) {
            radius = dis + 0.5f;
            updateBoundingBox();
            return true;
        }
        return false;
    }

    int SphereRegion::checkChanges(const std::vector<BlockPos>& changes) {
        int x = 0, y = 0, z = 0;
        for (auto change : changes) {
            x += abs(change.x);
            y += abs(change.y);
            z += abs(change.z);
        }
        if (x == y && y == z) {
            return x / 2;
        }
        return -1;
    }

    std::pair<std::string, bool> SphereRegion::expand(const std::vector<BlockPos>& changes) {
        int check = checkChanges(changes);
        if (check == -1) {
            return {"This region can only be expanded isotropically", false};
        } else {
            radius += check;
            updateBoundingBox();
        }

        return {"§aThis region has been expanded", true};
    }

    std::pair<std::string, bool> SphereRegion::contract(const std::vector<BlockPos>& changes) {
        int check = checkChanges(changes);
        if (check == -1) {
            return {"This region can only be contracted isotropically", false};
        } else {
            radius -= check;
            radius = std::max(radius, 0.5);
        }
        updateBoundingBox();
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

    SphereRegion::SphereRegion(const BoundingBox& region, const int& dim) : Region(region, dim) {
        this->selecting = false;
        this->regionType = SPHERE;
    }
    void SphereRegion::renderRegion() {
        if (selecting && dimensionID >= 0 && rendertick <= 0) {
            rendertick = 40;
            globalPT().drawCuboid(AABB(center.toVec3(), center.toVec3() + Vec3::ONE), dimensionID,
                                  mce::ColorPalette::GREEN);
            if (radius > 1.0f) {
                globalPT().drawCircle(center, ParticleCUI::Direction::POS_Y, radius, dimensionID,
                                      ParticleCUI::PointSize::PX4, 1, 64, mce::ColorPalette::YELLOW);
                globalPT().drawCircle(center, ParticleCUI::Direction::POS_Z, radius, dimensionID,
                                      ParticleCUI::PointSize::PX4, 1, 64, mce::ColorPalette::YELLOW);
                globalPT().drawCircle(center, ParticleCUI::Direction::POS_X, radius, dimensionID,
                                      ParticleCUI::PointSize::PX4, 1, 64, mce::ColorPalette::YELLOW);
            }
        }
        rendertick--;
    };
}  // namespace worldedit