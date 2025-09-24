#include "PolyRegion.h"
#include "utils/Math.h"
#include "utils/Serialize.h"
#include "worldedit/WorldEdit.h"

namespace we {
PolyRegion::PolyRegion(DimensionType d, BoundingBox const& b)
: Region(d, (b.max + b.min) / 2) {}

ll::Expected<> PolyRegion::serialize(CompoundTag& tag) const {
    return Region::serialize(tag).and_then([&, this]() {
        return ll::reflection::serialize_to(tag["points"], points);
    });
}
ll::Expected<> PolyRegion::deserialize(CompoundTag const& tag) {
    return Region::deserialize(tag).and_then([&, this]() {
        return ll::reflection::deserialize(points, tag.at("points"));
    });
}
void PolyRegion::updateBoundingBox() {
    if (points.empty()) {
        return;
    }
    boundingBox.min.y = minY;
    boundingBox.max.y = maxY;
    boundingBox.min.x = points.front().x;
    boundingBox.min.z = points.front().z;
    boundingBox.max.x = points.front().x;
    boundingBox.max.z = points.front().z;

    auto& we = WorldEdit::getInstance();

    auto& geo = we.getGeo();

    std::vector<bsci::GeometryGroup::GeoId> ids;
    for (auto& point : points) {
        boundingBox.min.x = std::min(boundingBox.min.x, point.x);
        boundingBox.min.z = std::min(boundingBox.min.z, point.z);
        boundingBox.max.x = std::max(boundingBox.max.x, point.x);
        boundingBox.max.z = std::max(boundingBox.max.z, point.z);

        ids.emplace_back(geo.line(
            getDim(),
            {point.x + 0.5, minY, point.z + 0.5},
            {point.x + 0.5, maxY + 1, point.z + 0.5},
            we.getConfig().colors.region_line_color
        ));
    }
    for (auto y : {minY, maxY + 1}) {
        for (auto [start, end] : points | std::views::transform([&](auto& p) {
                                     return Vec3{p.x + 0.5, y, p.z + 0.5};
                                 }) | std::views::pairwise) {
            ids.emplace_back(
                geo.line(getDim(), start, end, we.getConfig().colors.region_line_color)
            );
        }
        if (points.size() > 2) {
            ids.emplace_back(geo.line(
                getDim(),
                {points.front().x + 0.5, y, points.front().z + 0.5},
                {points.back().x + 0.5, y, points.back().z + 0.5},
                we.getConfig().colors.region_line_color
            ));
        }
    }
    outline = geo.merge(ids);
}

bool PolyRegion::removePoint(std::optional<BlockPos> const& pos) {
    if (points.empty()) {
        return false;
    }
    if (!pos) {
        points.pop_back();
    } else {
        double minDis = DBL_MAX;
        size_t pi     = 0;
        for (size_t j = 0; j < points.size(); ++j) {
            auto length = points[j].distanceTo(*pos);
            if (length <= minDis) {
                minDis = length;
                pi     = j;
            }
        }
        points.erase(points.begin() + pi);
    }
    updateBoundingBox();
    return true;
}
void PolyRegion::forEachBlockUVInRegion(
    std::function<void(BlockPos const&, double, double)>&& todo
) const {
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
Vec3 PolyRegion::getCenter() const {
    Pos2d tmp{};
    for (auto& point : points) {
        tmp = tmp + point;
    }
    auto size = points.size();
    return Vec3(
        static_cast<double>(tmp.x) / size + 0.5,
        static_cast<double>(maxY + minY) * 0.5 + 0.5,
        static_cast<double>(tmp.z) / size + 0.5
    );
}
bool PolyRegion::setMainPos(BlockPos const& pos) {
    minY = pos.y;
    maxY = pos.y;
    points.clear();
    points.emplace_back(pos.x, pos.z);
    updateBoundingBox();
    return true;
}
bool PolyRegion::setOffPos(BlockPos const& pos) {
    for (auto& p : points) {
        if (p.x == pos.x && p.z == pos.z) {
            minY = std::min(minY, pos.y);
            maxY = std::max(maxY, pos.y);
            updateBoundingBox();
            return true;
        }
    }
    minY = std::min(minY, pos.y);
    maxY = std::max(maxY, pos.y);
    points.emplace_back(pos.x, pos.z);
    updateBoundingBox();
    return true;
}
bool PolyRegion::expand(std::span<BlockPos> changes) {
    for (auto& change : changes) {
        if (change.x != 0 || change.z != 0) {
            return false;
        }
    }
    for (auto& change : changes) {
        int changeY = change.y;
        if (changeY > 0) {
            maxY += changeY;
        } else {
            minY += changeY;
        }
    }
    updateBoundingBox();
    return true;
}
bool PolyRegion::contract(std::span<BlockPos> changes) {
    for (auto& change : changes) {
        if (change.x != 0 || change.z != 0) {
            return false;
        }
    }
    for (auto& change : changes) {
        int changeY = change.y;
        if (changeY > 0) {
            minY += changeY;
        } else {
            maxY += changeY;
        }
    }
    updateBoundingBox();
    return true;
}
bool PolyRegion::shift(BlockPos const& change) {
    minY += change.y;
    maxY += change.y;
    for (auto& point : points) {
        point.x += change.x;
        point.z += change.z;
    }
    updateBoundingBox();
    return true;
}
size_t PolyRegion::size() const {
    if (points.empty()) {
        return 0;
    } else if (points.size() == 1) {
        return maxY - minY + 1;
    }
    int64 area      = 0;
    auto  lastPoint = points.front();
    for (auto& point : points) {
        area      += lastPoint.x * point.z - lastPoint.z * point.x;
        lastPoint  = point;
    }
    return (size_t)std::floor(std::abs((double)area * 0.5)) * (maxY - minY + 1);
};
bool PolyRegion::contains(BlockPos const& pos) const {
    if (points.size() < 3 || pos.y < minY || pos.y > maxY) {
        return false;
    }
    bool inside = false;

    auto lastPoint = points.front();
    for (auto& point : points) {
        if (lastPoint == point) {
            return true;
        }
        int x1;
        int z1;
        int x2;
        int z2;
        if (point.x > lastPoint.x) {
            x1 = lastPoint.x;
            x2 = point.x;
            z1 = lastPoint.z;
            z2 = point.z;
        } else {
            x1 = point.x;
            x2 = lastPoint.x;
            z1 = point.z;
            z2 = lastPoint.z;
        }
        if (x1 <= pos.x && pos.x <= x2) {
            int64 crossproduct = ((int64)pos.z - (int64)z1) * (int64)(x2 - x1)
                               - ((int64)z2 - (int64)z1) * (int64)(pos.x - x1);
            if (crossproduct == 0) {
                if ((z1 <= pos.z) == (pos.z <= z2)) {
                    return true;
                }
            } else if (crossproduct < 0 && (x1 != pos.x)) {
                inside = !inside;
            }
        }
        lastPoint = point;
    }
    return inside;
}
} // namespace we
