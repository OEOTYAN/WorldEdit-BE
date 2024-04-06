
#include "PolyRegion.h"
#include "mc/Dimension.hpp"
#include "mc/Level.hpp"
namespace we {
void PolyRegion::updateBoundingBox() {
    auto range = reinterpret_cast<Dimension*>(
                     Global<Level>->getDimension(dimensionID).mHandle.lock().get()
    )
                     ->getHeightRange();
    rendertick        = 0;
    minY              = std::max(minY, static_cast<int>(range.min));
    maxY              = std::min(maxY, static_cast<int>(range.max) - 1);
    boundingBox.min.y = minY;
    boundingBox.max.y = maxY;
    boundingBox.min.x = points[0].x;
    boundingBox.min.z = points[0].z;
    boundingBox.max.x = points[0].x;
    boundingBox.max.z = points[0].z;
    for (auto& point : points) {
        boundingBox.min.x = std::min(boundingBox.min.x, point.x);
        boundingBox.min.z = std::min(boundingBox.min.z, point.z);
        boundingBox.max.x = std::max(boundingBox.max.x, point.x);
        boundingBox.max.z = std::max(boundingBox.max.z, point.z);
    }
}

void PolyRegion::forEachBlockUVInRegion(
    const std::function<void(BlockPos const&, double, double)>& todo
) {
    auto faceNum = points.size() - 1;
    for (int i = 0; i < faceNum; ++i) {
        auto pos0 = points[i];
        auto pos1 = points[i + 1];
        int  x0   = pos0.x;
        int  z0   = pos0.z;
        int  x1   = pos1.x;
        int  z1   = pos1.z;
        int  dx   = abs(x1 - x0);
        int  dz   = abs(z1 - z0);
        int  sx   = x0 < x1 ? 1 : -1;
        int  sz   = z0 < z1 ? 1 : -1;

        int dMax = std::max(dx, dz);
        if (dMax == dx) {
            for (int domstep = 0; domstep <= dx; domstep++) {
                int tipx = x0 + domstep * sx;
                int tipz = (int)round(z0 + domstep / ((double)dx) * ((double)dz) * sz);
                for (int y = minY; y <= maxY; ++y) {
                    todo(
                        BlockPos(tipx, y, tipz),
                        (i + (static_cast<double>(domstep) / dx)) / faceNum,
                        static_cast<double>(y - minY) / (maxY - minY)
                    );
                }
            }
        } else {
            for (int domstep = 0; domstep <= dz; domstep++) {
                int tipx = (int)round(x0 + domstep / ((double)dz) * ((double)dx) * sx);
                int tipz = z0 + domstep * sz;
                for (int y = minY; y <= maxY; ++y) {
                    todo(
                        BlockPos(tipx, y, tipz),
                        (i + (static_cast<double>(domstep) / dz)) / faceNum,
                        static_cast<double>(y - minY) / (maxY - minY)
                    );
                }
            }
        }
    }
}

PolyRegion::PolyRegion(const BoundingBox& region, int dim) : Region(region, dim) {
    points.clear();
    regionType = POLY;
}
bool PolyRegion::setMainPos(BlockPos const& pos, int dim) {
    mainPos     = pos;
    dimensionID = dim;
    selecting   = 1;
    minY        = pos.y;
    maxY        = pos.y;
    points.clear();
    points.push_back({pos.x, 0, pos.z});
    updateBoundingBox();
    return true;
}
bool PolyRegion::setVicePos(BlockPos const& pos, int dim) {
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
bool PolyRegion::expand(const std::vector<BlockPos>& changes) {
    for (BlockPos change : changes) {
        if (change.x != 0 || change.z != 0) {
            return {"worldedit.selection.expand.poly.error", false};
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

    return {"worldedit.expand.expanded", true};
}

bool PolyRegion::contract(const std::vector<BlockPos>& changes) {
    for (BlockPos change : changes) {
        if (change.x != 0 || change.z != 0) {
            return {"worldedit.selection.contract.poly.error", false};
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
    return {"worldedit.contract.contracted", true};
}

bool PolyRegion::shift(BlockPos const& change) {
    minY    += change.y;
    maxY    += change.y;
    mainPos  = mainPos + change;
    for (auto& point : points) {
        point.x += change.x;
        point.z += change.z;
    }
    updateBoundingBox();
    return {"worldedit.shift.shifted", true};
}

uint64_t PolyRegion::size() const {
    uint64_t area = 0;
    auto     j    = points.size() - 1;
    for (int i = 0; i <= j; ++i) {
        int x  = points[j].x + points[i].x;
        int z  = points[j].z - points[i].z;
        area  += x * z;
        j      = i;
    }

    return (uint64_t)std::floor(std::abs((double)area * 0.5)) * (maxY - minY + 1);
};

void PolyRegion::renderRegion() {
    if (selecting && dimensionID >= 0 && rendertick <= 0) {
        rendertick = 40;
        auto size  = points.size();
        for (int i = 0; i < size; ++i) {
            globalPT().drawCuboid(
                AABB(
                    Vec3(points[i].x, minY, points[i].z),
                    Vec3(points[i].x, maxY, points[i].z) + Vec3::ONE
                ),
                dimensionID,
                mce::ColorPalette::YELLOW
            );
        }
        for (int y : {minY, maxY}) {
            for (int i = 0; i < size - 1; ++i) {
                globalPT().drawOrientedLine(
                    BlockPos(points[i].x, y, points[i].z),
                    BlockPos(points[i + 1].x, y, points[i + 1].z),
                    dimensionID,
                    ParticleCUI::PointSize::PX4,
                    1,
                    64,
                    mce::ColorPalette::YELLOW
                );
            }
            globalPT().drawOrientedLine(
                BlockPos(points[0].x, y, points[0].z),
                BlockPos(points[size - 1].x, y, points[size - 1].z),
                dimensionID,
                ParticleCUI::PointSize::PX4,
                1,
                64,
                mce::ColorPalette::YELLOW
            );
        }
    }
    rendertick--;
};

bool PolyRegion::contains(BlockPos const& pos) {
    if (points.size() < 3 || pos.y < minY || pos.y > maxY) {
        return false;
    }
    bool      inside = false;
    int       x1;
    int       z1;
    int       x2;
    int       z2;
    long long crossproduct;
    BlockPos  point = points.back();

    for (auto iter = points.cbegin(); iter != points.cend(); iter++) {
        auto value = *iter;
        if (value == BlockPos(pos.x, 0, pos.z)) return true;
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
            crossproduct = ((long long)pos.z - (long long)z1) * (long long)(x2 - x1)
                         - ((long long)z2 - (long long)z1) * (long long)(pos.x - x1);
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
} // namespace we