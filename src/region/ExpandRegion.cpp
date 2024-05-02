#include "ExpandRegion.h"

namespace we {
ExpandRegion::ExpandRegion(DimensionType d, BoundingBox const& b) : CuboidRegion(d, b) {}

bool ExpandRegion::setMainPos(BlockPos const& pos) {
    mainPos = pos;
    vicePos = pos;
    updateBoundingBox();
    return true;
}

bool ExpandRegion::setVicePos(BlockPos const& pos) {
    if (contains(pos)) {
        std::cout << "Enter"<< std::endl;
        return false;
    }
    mainPos.x = std::min(mainPos.x, pos.x);
    mainPos.y = std::min(mainPos.y, pos.y);
    mainPos.z = std::min(mainPos.z, pos.z);
    vicePos.x = std::max(pos.x, vicePos.x);
    vicePos.y = std::max(pos.y, vicePos.y);
    vicePos.z = std::max(pos.z, vicePos.z);
    updateBoundingBox();
    return true;
}
} // namespace we
