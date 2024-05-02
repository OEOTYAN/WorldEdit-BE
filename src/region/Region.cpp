#include "Region.h"
#include "ConvexRegion.h"
#include "CuboidRegion.h"
#include "CylinderRegion.h"
#include "ExpandRegion.h"
#include "PolyRegion.h"
#include "SphereRegion.h"
#include "utils/Serialize.h"

namespace we {

ll::Expected<> Region::serialize(CompoundTag& tag) const {
    return ll::reflection::serialize_to(tag["type"], getType())
        .and_then([&, this]() {
            return ll::reflection::serialize_to(tag["boundingBox"], boundingBox);
        })
        .and_then([&, this]() { return ll::reflection::serialize_to(tag["dim"], dim.id); }
        );
}
ll::Expected<> Region::deserialize(CompoundTag const&) { return {}; }

void Region::forEachBlockInRegion(std::function<void(BlockPos const&)>&& todo) const {
    boundingBox.forEachPos([todo = std::move(todo), this](BlockPos const& pos) {
        if (contains(pos)) {
            todo(pos);
        }
        return true;
    });
}
void Region::forEachBlockUVInRegion(
    std::function<void(BlockPos const&, double, double)>&& todo
) const {
    auto size = boundingBox.getSideLength();
    if (size.x == 1) {
        Region::forEachBlockInRegion(
            [todo = std::move(todo), this, &size](BlockPos const& pos) {
                auto localPos = pos - boundingBox.min;
                todo(pos, (localPos.z + 0.5) / size.z, (localPos.y + 0.5) / size.y);
            }
        );
    } else if (size.z == 1) {
        Region::forEachBlockInRegion(
            [todo = std::move(todo), this, &size](BlockPos const& pos) {
                auto localPos = pos - boundingBox.min;
                todo(pos, (localPos.x + 0.5) / size.x, (localPos.y + 0.5) / size.y);
            }
        );
    } else {
        Region::forEachBlockInRegion(
            [todo = std::move(todo), this, &size](BlockPos const& pos) {
                auto localPos = pos - boundingBox.min;
                todo(pos, (localPos.x + 0.5) / size.x, (localPos.z + 0.5) / size.z);
            }
        );
    }
}

int Region::getHeighest(Pos2d xz) const {
    for (int y = boundingBox.max.y; y >= boundingBox.min.y; y--) {
        if (contains({xz.x, y, xz.z})) {
            return y;
        }
    }
    return boundingBox.min.y - 1;
}

Region::Region(DimensionType d, BoundingBox const& b) : boundingBox(b), dim(d) {}

std::shared_ptr<Region>
Region::create(RegionType type, DimensionType dim, BoundingBox const& box, bool update) {
    std::shared_ptr<Region> res;
    switch (type) {
    case RegionType::Cuboid:
        res = std::make_shared<CuboidRegion>(dim, box);
        break;
    case RegionType::Expand:
        res = std::make_shared<ExpandRegion>(dim, box);
        break;
    case RegionType::Sphere:
        res = std::make_shared<SphereRegion>(dim, box);
        break;
    case RegionType::Poly:
        res = std::make_shared<PolyRegion>(dim, box);
        break;
    case RegionType::Convex:
        res = std::make_shared<ConvexRegion>(dim, box);
        break;
    case RegionType::Cylinder:
        res = std::make_shared<CylinderRegion>(dim, box);
        break;
    // case RegionType::Loft:
    //     res = std::make_shared<LoftRegion>(dim, box); break;
    default:
        std::unreachable();
    }
    if (update) {
        res->updateBoundingBox();
    }
    return res;
}
ll::Expected<std::shared_ptr<Region>> Region::create(CompoundTag const& tag) {
    RegionType type;
    int        dim;
    return ll::reflection::deserialize_to<RegionType>(tag["type"])
        .and_then([&](auto&& r) {
            type = r;
            return ll::reflection::deserialize_to<int>(tag["dim"]);
        })
        .and_then([&](auto&& r) {
            dim = r;
            return ll::reflection::deserialize_to<BoundingBox>(tag["boundingBox"]);
        })
        .transform([&](auto&& r) { return create(type, dim, r, false); })
        .and_then([&](auto&& r) -> ll::Expected<std::shared_ptr<Region>> {
            if (auto res = r->deserialize(tag); res) {
                return r;
            } else {
                return ll::forwardError(res.error());
            }
        })
        .transform([&](auto&& r) {
            r->updateBoundingBox();
            return r;
        });
}
} // namespace we
