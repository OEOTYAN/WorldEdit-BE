#include "CuboidRegion.h"
#include "utils/Serialize.h"
#include "worldedit/WorldEdit.h"

namespace we {
CuboidRegion::CuboidRegion(DimensionType d, BoundingBox const& b)
: Region(d, b),
  mainPos(b.min),
  offPos(b.max) {}

ll::Expected<> CuboidRegion::serialize(CompoundTag& tag) const {
    return Region::serialize(tag)
        .and_then([&, this]() {
            return ll::reflection::serialize_to(tag["mainPos"], mainPos);
        })
        .and_then([&, this]() {
            return ll::reflection::serialize_to(tag["offPos"], offPos);
        });
}
ll::Expected<> CuboidRegion::deserialize(CompoundTag const& tag) {
    return Region::deserialize(tag)
        .and_then([&, this]() {
            return ll::reflection::deserialize(mainPos, tag.at("mainPos"));
        })
        .and_then([&, this]() {
            return ll::reflection::deserialize(offPos, tag.at("offPos"));
        });
}

void CuboidRegion::updateBoundingBox() {
    boundingBox.min.x = std::min(mainPos.x, offPos.x);
    boundingBox.min.y = std::min(mainPos.y, offPos.y);
    boundingBox.min.z = std::min(mainPos.z, offPos.z);
    boundingBox.max.x = std::max(mainPos.x, offPos.x);
    boundingBox.max.y = std::max(mainPos.y, offPos.y);
    boundingBox.max.z = std::max(mainPos.z, offPos.z);
    boundingBox.max.x = std::max(mainPos.x, offPos.x);
    box               = WorldEdit::getInstance().getGeo().box(
        getDim(),
        boundingBox,
        WorldEdit::getInstance().getConfig().colors.region_line_color
    );
}

bool CuboidRegion::setMainPos(BlockPos const& pos) {
    mainPos = pos;
    updateBoundingBox();
    return true;
}

bool CuboidRegion::setOffPos(BlockPos const& pos) {
    offPos = pos;
    updateBoundingBox();
    return true;
}

bool CuboidRegion::expand(std::span<BlockPos> changes) {
    for (auto& change : changes) {
        if (change.x > 0) {
            if (std::max(mainPos.x, offPos.x) == mainPos.x) {
                mainPos += {change.x, 0, 0};
            } else {
                offPos += {change.x, 0, 0};
            }
        } else {
            if (std::min(mainPos.x, offPos.x) == mainPos.x) {
                mainPos += {change.x, 0, 0};
            } else {
                offPos += {change.x, 0, 0};
            }
        }

        if (change.y > 0) {
            if (std::max(mainPos.y, offPos.y) == mainPos.y) {
                mainPos += {0, change.y, 0};
            } else {
                offPos += {0, change.y, 0};
            }
        } else {
            if (std::min(mainPos.y, offPos.y) == mainPos.y) {
                mainPos += {0, change.y, 0};
            } else {
                offPos += {0, change.y, 0};
            }
        }

        if (change.z > 0) {
            if (std::max(mainPos.z, offPos.z) == mainPos.z) {
                mainPos += {0, 0, change.z};
            } else {
                offPos += {0, 0, change.z};
            }
        } else {
            if (std::min(mainPos.z, offPos.z) == mainPos.z) {
                mainPos += {0, 0, change.z};
            } else {
                offPos += {0, 0, change.z};
            }
        }
    }
    updateBoundingBox();
    return true;
}

bool CuboidRegion::contract(std::span<BlockPos> changes) {
    for (auto& change : changes) {
        if (change.x < 0) {
            if (std::max(mainPos.x, offPos.x) == mainPos.x) {
                mainPos += {change.x, 0, 0};
            } else {
                offPos += {change.x, 0, 0};
            }
        } else {
            if (std::min(mainPos.x, offPos.x) == mainPos.x) {
                mainPos += {change.x, 0, 0};
            } else {
                offPos += {change.x, 0, 0};
            }
        }

        if (change.y < 0) {
            if (std::max(mainPos.y, offPos.y) == mainPos.y) {
                mainPos += {0, change.y, 0};
            } else {
                offPos += {0, change.y, 0};
            }
        } else {
            if (std::min(mainPos.y, offPos.y) == mainPos.y) {
                mainPos += {0, change.y, 0};
            } else {
                offPos += {0, change.y, 0};
            }
        }

        if (change.z < 0) {
            if (std::max(mainPos.z, offPos.z) == mainPos.z) {
                mainPos += {0, 0, change.z};
            } else {
                offPos += {0, 0, change.z};
            }
        } else {
            if (std::min(mainPos.z, offPos.z) == mainPos.z) {
                mainPos += {0, 0, change.z};
            } else {
                offPos += {0, 0, change.z};
            }
        }
    }
    updateBoundingBox();
    return true;
}

bool CuboidRegion::shift(BlockPos const& change) {
    mainPos += change;
    offPos  += change;
    updateBoundingBox();
    return true;
}

void CuboidRegion::forEachLine(
    std::function<void(BlockPos const&, BlockPos const&)>&& todo
) const {
    todo(mainPos, offPos);
}
} // namespace we
