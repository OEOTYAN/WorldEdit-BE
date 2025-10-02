#include "ExpandRegion.h"

namespace we {
ExpandRegion::ExpandRegion(DimensionType d, BoundingBox const& b) : CuboidRegion(d, b) {}

bool ExpandRegion::setMainPos(BlockPos const& pos) {
    mainPos = pos;
    offPos  = pos;
    updateBoundingBox();
    return true;
}

bool ExpandRegion::setOffPos(BlockPos const& pos) {
    if (contains(pos)) {
        return false;
    }
    mainPos.x = std::min(mainPos.x, pos.x);
    mainPos.y = std::min(mainPos.y, pos.y);
    mainPos.z = std::min(mainPos.z, pos.z);
    offPos.x  = std::max(pos.x, offPos.x);
    offPos.y  = std::max(pos.y, offPos.y);
    offPos.z  = std::max(pos.z, offPos.z);
    updateBoundingBox();
    return true;
}
} // namespace we
