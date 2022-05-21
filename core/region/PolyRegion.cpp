//
// Created by OEOTYAN on 2021/2/8.
//

#include "PolyRegion.h"
namespace worldedit {
    void PolyRegion::updateBoundingBox() {
        rendertick = 0;
        boundingBox.bpos1.y = minY;
        boundingBox.bpos2.y = maxY;
        boundingBox.bpos1.x = points[0].x;
        boundingBox.bpos1.z = points[0].z;
        boundingBox.bpos2.x = points[0].x;
        boundingBox.bpos2.z = points[0].z;
        for_each(points.begin(), points.end(), [&](BlockPos value) {
            boundingBox.bpos1.x = std::min(boundingBox.bpos1.x, value.x);
            boundingBox.bpos1.z = std::min(boundingBox.bpos1.z, value.z);
            boundingBox.bpos2.x = std::max(boundingBox.bpos2.x, value.x);
            boundingBox.bpos2.z = std::max(boundingBox.bpos2.z, value.z);
        });
    }
    PolyRegion::PolyRegion(const BoundingBox& region, const int& dim)
        : Region(region, dim) {
        points.clear();
        this->regionType = POLY;
    }
    bool PolyRegion::setMainPos(const BlockPos& pos, const int& dim) {
        mainPos = pos;
        dimensionID = dim;
        selecting = 1;
        minY = pos.y;
        maxY = pos.y;
        points.clear();
        points.push_back({pos.x, 0, pos.z});
        updateBoundingBox();
        return true;
    }
    bool PolyRegion::setVicePos(const BlockPos& pos, const int& dim) {
        if (!selecting || dim != dimensionID) {
            return false;
        }
        for (auto iter = points.cbegin(); iter != points.cend(); iter++) {
            if (*iter == BlockPos(pos.x, 0, pos.z)) {
                minY = std::min(minY, pos.y);
                maxY = std::max(maxY, pos.y);
                updateBoundingBox();
                return true;
            }
        }
        minY = std::min(minY, pos.y);
        maxY = std::max(maxY, pos.y);
        points.push_back({pos.x, 0, pos.z});
        updateBoundingBox();
        return true;
    }

    bool PolyRegion::expand(const BlockPos& change, Player* player) {
        if (change.x != 0 || change.z != 0) {
            player->sendFormattedText(
                "§cThis region can only expand vertically");
            return false;
        }
        int changeY = change.y;
        if (changeY > 0) {
            maxY += changeY;
        } else {
            minY += changeY;
        }

        updateBoundingBox();

        player->sendFormattedText("§aThis region has been expanded");
        return true;
    }

    bool PolyRegion::contract(const BlockPos& change, Player* player) {
        if (change.x != 0 || change.z != 0) {
            player->sendFormattedText(
                "§cThis region can only contract vertically");
            return false;
        }
        int changeY = change.y;
        if (changeY > 0) {
            minY += changeY;
        } else {
            maxY += changeY;
        }

        updateBoundingBox();
        player->sendFormattedText("§aThis region has been contracted");
        return true;
    }
    bool PolyRegion::expand(const std::vector<BlockPos>& changes,
                            Player* player) {
        for (BlockPos change : changes) {
            if (change.x != 0 || change.z != 0) {
                player->sendFormattedText(
                    "§cThis region can only expand vertically");
                return false;
            }
        }
        for (BlockPos change : changes) {
            int changeY = change.y;
            if (changeY > 0) {
                maxY += changeY;
            } else {
                minY += changeY;
            }
        }
        updateBoundingBox();

        player->sendFormattedText("§aThis region has been expanded");
        return true;
    }

    bool PolyRegion::contract(const std::vector<BlockPos>& changes,
                              Player* player) {
        for (BlockPos change : changes) {
            if (change.x != 0 || change.z != 0) {
                player->sendFormattedText(
                    "§cThis region can only contract vertically");
                return false;
            }
        }
        for (BlockPos change : changes) {
            int changeY = change.y;
            if (changeY > 0) {
                minY += changeY;
            } else {
                maxY += changeY;
            }
        }
        updateBoundingBox();
        player->sendFormattedText("§aThis region has been contracted");
        return true;
    }

    bool PolyRegion::shift(const BlockPos& change, Player* player) {
        minY += change.y;
        maxY += change.y;
        mainPos = mainPos + change;
        for (auto& point : points) {
            point.x += change.x;
            point.z += change.z;
        }
        updateBoundingBox();
        player->sendFormattedText("§aThis region has been shifted");
        return true;
    }

    int PolyRegion::size() const {
        int area = 0;
        int j = (int)points.size() - 1;
        for (int i = 0; i < points.size(); ++i) {
            int x = points[j].x + points[i].x;
            int z = points[j].z - points[i].z;
            area += x * z;
            j = i;
        }

        return (int)std::floor(std::abs((double)area * 0.5)) *
               (maxY - minY + 1);
    };

    void PolyRegion::renderRegion() {
        if (selecting && dimensionID >= 0 && rendertick <= 0) {
            rendertick = 40;
            auto size = points.size();
            for (int i = 0; i < size; i++) {
                worldedit::spawnCuboidParticle(
                    {Vec3(points[i].x, minY, points[i].z),
                     Vec3(points[i].x, maxY, points[i].z) + Vec3::ONE},
                    GRAPHIC_COLOR::GREEN, dimensionID);
            }
            for (int y : {minY, maxY}) {
                for (int i = 0; i < size - 1; i++) {
                    drawOrientedLine(Vec3(points[i].x, y, points[i].z),
                                     Vec3(points[i + 1].x, y, points[i + 1].z),
                                     dimensionID);
                }
                drawOrientedLine(
                    Vec3(points[0].x, y, points[0].z),
                    Vec3(points[size - 1].x, y, points[size - 1].z),
                    dimensionID);
            }
        }
        rendertick--;
    };

    bool PolyRegion::contains(const BlockPos& pos) {
        if (points.size() < 3 || pos.y < minY || pos.y > maxY) {
            return false;
        }
        bool inside = false;
        int x1;
        int z1;
        int x2;
        int z2;
        long long crossproduct;
        BlockPos point = points.back();

        for (auto iter = points.cbegin(); iter != points.cend(); iter++) {
            auto value = *iter;
            if (value == BlockPos(pos.x, 0, pos.z))
                return true;
            if (value.x > point.x) {
                x1 = point.x;
                x2 = value.x;
                z1 = point.z;
                z2 = value.z;
            } else {
                x1 = value.x;
                x2 = point.x;
                z1 = value.z;
                z2 = point.z;
            }
            if (x1 <= pos.x && pos.x <= x2) {
                crossproduct =
                    ((long long)pos.z - (long long)z1) * (long long)(x2 - x1) -
                    ((long long)z2 - (long long)z1) * (long long)(pos.x - x1);
                if (crossproduct == 0) {
                    if ((z1 <= pos.z) == (pos.z <= z2)) {
                        return true;
                    }
                } else if (crossproduct < 0 && (x1 != pos.x)) {
                    inside = !inside;
                }
            }
            point.x = value.x;
            point.z = value.z;
        }
        return inside;
    }
}  // namespace worldedit