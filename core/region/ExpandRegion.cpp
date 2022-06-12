//
// Created by OEOTYAN on 2021/2/8.
//

#include "ExpandRegion.h"
#include "MC/Level.hpp"
#include "MC/Dimension.hpp"
namespace worldedit {
    void ExpandRegion::updateBoundingBox() {
        auto range        = Level::getDimension(dimensionID)->getHeightRange();
        rendertick        = 0;
        boundingBox.min.x = std::min(mainPos.x, vicePos.x);
        boundingBox.min.y = std::max(std::min(mainPos.y, vicePos.y), static_cast<int>(range.min));
        boundingBox.min.z = std::min(mainPos.z, vicePos.z);
        boundingBox.max.x = std::max(mainPos.x, vicePos.x);
        boundingBox.max.y = std::min(std::max(mainPos.y, vicePos.y), static_cast<int>(range.max) - 1);
        boundingBox.max.z = std::max(mainPos.z, vicePos.z);
    }

    bool ExpandRegion::setMainPos(const BlockPos& pos, const int& dim) {
        selecting   = true;
        dimensionID = dim;
        mainPos     = pos;
        vicePos     = pos;
        updateBoundingBox();
        return true;
    }

    bool ExpandRegion::setVicePos(const BlockPos& pos, const int& dim) {
        if (!selecting || dim != dimensionID) {
            return false;
        }
        if (!pos.containedWithin(boundingBox.min, boundingBox.max)) {
            mainPos.x = std::min(mainPos.x, pos.x);
            mainPos.y = std::min(mainPos.y, pos.y);
            mainPos.z = std::min(mainPos.z, pos.z);
            vicePos.x = std::max(pos.x, vicePos.x);
            vicePos.y = std::max(pos.y, vicePos.y);
            vicePos.z = std::max(pos.z, vicePos.z);
            updateBoundingBox();
            return true;
        }
        return false;
    }

    std::pair<std::string, bool> ExpandRegion::expand(const std::vector<BlockPos>& changes) {
        for (auto change : changes) {
            if (change.x > 0) {
                if (std::max(mainPos.x, vicePos.x) == mainPos.x) {
                    mainPos = mainPos + BlockPos(change.x, 0, 0);
                } else {
                    vicePos = vicePos + BlockPos(change.x, 0, 0);
                }
            } else {
                if (std::min(mainPos.x, vicePos.x) == mainPos.x) {
                    mainPos = mainPos + BlockPos(change.x, 0, 0);
                } else {
                    vicePos = vicePos + BlockPos(change.x, 0, 0);
                }
            }

            if (change.y > 0) {
                if (std::max(mainPos.y, vicePos.y) == mainPos.y) {
                    mainPos = mainPos + BlockPos(0, change.y, 0);
                } else {
                    vicePos = vicePos + BlockPos(0, change.y, 0);
                }
            } else {
                if (std::min(mainPos.y, vicePos.y) == mainPos.y) {
                    mainPos = mainPos + BlockPos(0, change.y, 0);
                } else {
                    vicePos = vicePos + BlockPos(0, change.y, 0);
                }
            }

            if (change.z > 0) {
                if (std::max(mainPos.z, vicePos.z) == mainPos.z) {
                    mainPos = mainPos + BlockPos(0, 0, change.z);
                } else {
                    vicePos = vicePos + BlockPos(0, 0, change.z);
                }
            } else {
                if (std::min(mainPos.z, vicePos.z) == mainPos.z) {
                    mainPos = mainPos + BlockPos(0, 0, change.z);
                } else {
                    vicePos = vicePos + BlockPos(0, 0, change.z);
                }
            }
        }

        updateBoundingBox();

        return {"§aThis region has been expanded", true};
    }

    std::pair<std::string, bool> ExpandRegion::contract(const std::vector<BlockPos>& changes) {
        for (auto change : changes) {
            if (change.x < 0) {
                if (std::max(mainPos.x, vicePos.x) == mainPos.x) {
                    mainPos = mainPos + BlockPos(change.x, 0, 0);
                } else {
                    vicePos = vicePos + BlockPos(change.x, 0, 0);
                }
            } else {
                if (std::min(mainPos.x, vicePos.x) == mainPos.x) {
                    mainPos = mainPos + BlockPos(change.x, 0, 0);
                } else {
                    vicePos = vicePos + BlockPos(change.x, 0, 0);
                }
            }

            if (change.y < 0) {
                if (std::max(mainPos.y, vicePos.y) == mainPos.y) {
                    mainPos = mainPos + BlockPos(0, change.y, 0);
                } else {
                    vicePos = vicePos + BlockPos(0, change.y, 0);
                }
            } else {
                if (std::min(mainPos.y, vicePos.y) == mainPos.y) {
                    mainPos = mainPos + BlockPos(0, change.y, 0);
                } else {
                    vicePos = vicePos + BlockPos(0, change.y, 0);
                }
            }

            if (change.z < 0) {
                if (std::max(mainPos.z, vicePos.z) == mainPos.z) {
                    mainPos = mainPos + BlockPos(0, 0, change.z);
                } else {
                    vicePos = vicePos + BlockPos(0, 0, change.z);
                }
            } else {
                if (std::min(mainPos.z, vicePos.z) == mainPos.z) {
                    mainPos = mainPos + BlockPos(0, 0, change.z);
                } else {
                    vicePos = vicePos + BlockPos(0, 0, change.z);
                }
            }
        }

        updateBoundingBox();
        return {"§aThis region has been contracted", true};
    }

    std::pair<std::string, bool> ExpandRegion::shift(const BlockPos& change) {
        mainPos = mainPos + change;
        vicePos = vicePos + change;
        updateBoundingBox();

        return {"§aThis region has been shifted", true};
    }

    ExpandRegion::ExpandRegion(const BoundingBox& region, const int& dim)
        : Region(region, dim), mainPos(region.min), vicePos(region.max) {
        this->regionType = EXPAND;
        this->selecting  = true;
    }
}  // namespace worldedit