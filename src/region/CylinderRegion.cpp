#include "CylinderRegion.h"
#include "utils/Math.h"
#include "utils/Serialize.h"
#include "worldedit/WorldEdit.h"

namespace we {
CylinderRegion::CylinderRegion(DimensionType d, BoundingBox const& b)
: Region(d, b),
  center((b.min + b.max) / 2),
  radius(b.getSideLength().dot({1, 0, 1}) / 4.0),
  minY(b.min.y),
  maxY(b.max.y) {}

ll::Expected<> CylinderRegion::serialize(CompoundTag& tag) const {
    return Region::serialize(tag)
        .and_then([&, this]() {
            return ll::reflection::serialize_to(tag["center"], center);
        })
        .and_then([&, this]() {
            return ll::reflection::serialize_to(tag["radius"], radius);
        })
        .and_then([&, this]() { return ll::reflection::serialize_to(tag["minY"], minY); })
        .and_then([&, this]() { return ll::reflection::serialize_to(tag["maxY"], maxY); }
        );
}
ll::Expected<> CylinderRegion::deserialize(CompoundTag const& tag) {
    return Region::deserialize(tag)
        .and_then([&, this]() {
            return ll::reflection::deserialize(center, tag.at("center"));
        })
        .and_then([&, this]() {
            return ll::reflection::deserialize(radius, tag.at("radius"));
        })
        .and_then([&, this]() {
            return ll::reflection::deserialize(minY, tag.at("minY"));
        })
        .and_then([&, this]() {
            return ll::reflection::deserialize(maxY, tag.at("maxY"));
        });
}

void CylinderRegion::forEachBlockUVInRegion(
    std::function<void(BlockPos const&, double, double)>&& todo
) const {
    forEachBlockInRegion([&](BlockPos const& pos) {
        int counts = 0;
        for (auto& calPos : pos.getNeighbors()) {
            counts += contains(calPos);
        }
        if (counts < 6) {
            todo(
                pos,
                (atan2(pos.z - center.z, pos.x - center.x) + std::numbers::pi)
                    / (std::numbers::pi * 2),
                (pos.y - minY) / static_cast<double>(maxY - minY)
            );
        }
    });
}

void CylinderRegion::updateBoundingBox() {
    auto newRadius    = static_cast<int>(radius);
    boundingBox.min.x = center.x - newRadius;
    boundingBox.min.y = minY;
    boundingBox.min.z = center.z - newRadius;
    boundingBox.max.x = center.x + newRadius;
    boundingBox.max.y = maxY;
    boundingBox.max.z = center.z + newRadius;

    auto& we = WorldEdit::getInstance();

    auto& geo = we.getGeo();

    std::vector<bsci::GeometryGroup::GeoId> ids;
    for (uint i = 0, e = std::min(8, std::max(3, maxY - minY + 1) / 3); i <= e; i++) {
        ids.emplace_back(geo.circle(
            getDim(),
            {center.x + 0.5,
             (i / static_cast<double>(e)) * (maxY - minY + 1) + minY,
             center.z + 0.5},
            {0, 1, 0},
            (float)radius,
            we.getConfig().colors.region_line_color
        ));
    }
    circles  = geo.merge(ids);
    startbox = geo.box(
        getDim(),
        BlockPos{center.x, minY, center.z},
        we.getConfig().colors.region_point_color
    );
    if (maxY != minY) {
        endbox = geo.box(
            getDim(),
            BlockPos{center.x, maxY, center.z},
            we.getConfig().colors.region_point_color
        );
        centerline = geo.line(
            getDim(),
            BlockPos{center.x, minY, center.z}.center(),
            BlockPos{center.x, maxY, center.z}.center(),
            we.getConfig().colors.region_point_color
        );
    } else {
        endbox     = {};
        centerline = {};
    }
}

bool CylinderRegion::setY(int y) {
    if (y < minY) {
        minY = y;
        return true;
    } else if (y > maxY) {
        maxY = y;
        return true;
    }
    return false;
}

bool CylinderRegion::setMainPos(BlockPos const& pos) {
    center = pos;
    radius = 0.5;
    minY   = pos.y;
    maxY   = pos.y;
    updateBoundingBox();
    return true;
}

bool CylinderRegion::setVicePos(BlockPos const& pos) {
    auto   disx = pos.x - center.x;
    auto   disz = pos.z - center.z;
    double dis  = sqrt(disx * disx + disz * disz);
    radius      = std::max(dis, 0.0) + 0.5;
    setY(pos.y);
    updateBoundingBox();
    return true;
}

bool CylinderRegion::contains(BlockPos const& pos) const {
    if (pos.y < minY || pos.y > maxY) {
        return false;
    }
    auto disx = pos.x - center.x;
    auto disz = pos.z - center.z;
    return disx * disx + disz * disz <= radius * radius;
}

std::optional<int> CylinderRegion::checkChanges(std::span<BlockPos> changes) const {
    int x = 0, z = 0;
    for (auto& change : changes) {
        x += abs(change.x);
        z += abs(change.z);
    }
    if (x == z) {
        return x / 2;
    }
    return -1;
}

bool CylinderRegion::expand(std::span<BlockPos> changes) {
    auto check = checkChanges(changes);
    if (!check) {
        return false;
    } else {
        radius += *check;
    }
    for (auto& change : changes) {
        if (change.y > 0) {
            maxY += change.y;
        } else {
            minY += change.y;
        }
    }
    updateBoundingBox();
    return true;
}

bool CylinderRegion::contract(std::span<BlockPos> changes) {
    auto check = checkChanges(changes);
    if (!check) {
        return false;
    } else {
        radius -= *check;
        radius  = std::max(radius, 0.5);
    }
    for (auto& change : changes) {
        int height = maxY - minY;
        if (change.y > 0) {
            minY += std::min(change.y, height);
        } else {
            maxY += std::max(change.y, -height);
        }
    }
    updateBoundingBox();
    return false;
}

bool CylinderRegion::shift(BlockPos const& change) {
    center  = center + change;
    maxY   += change.y;
    minY   += change.y;
    updateBoundingBox();
    return true;
}
} // namespace we