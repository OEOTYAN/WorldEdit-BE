//
// Created by OEOTYAN on 2021/2/8.
//

#include "CuboidRegion.h"
#include "MC/Level.hpp"
#include "MC/Dimension.hpp"
namespace worldedit {
    void CuboidRegion::updateBoundingBox() {
        auto range        = Level::getDimension(dimensionID)->getHeightRange();
        rendertick        = 0;
        boundingBox.min.x = std::min(mainPos.x, vicePos.x);
        boundingBox.min.y = std::max(std::min(mainPos.y, vicePos.y), static_cast<int>(range.min));
        boundingBox.min.z = std::min(mainPos.z, vicePos.z);
        boundingBox.max.x = std::max(mainPos.x, vicePos.x);
        boundingBox.max.y = std::min(std::max(mainPos.y, vicePos.y), static_cast<int>(range.max) - 1);
        boundingBox.max.z = std::max(mainPos.z, vicePos.z);
        boundingBox.max.x = std::max(mainPos.x, vicePos.x);
    }

    bool CuboidRegion::setMainPos(const BlockPos& pos, const int& dim) {
        if (mainPos != pos) {
            if (!selecting) {
                dimensionID = dim;
                vicePos     = pos;
                selecting   = true;
            } else {
                if (dim != dimensionID)
                    return false;
            }
            mainPos = pos;
            updateBoundingBox();
            return true;
        }
        return false;
    }

    bool CuboidRegion::setVicePos(const BlockPos& pos, const int& dim) {
        if (vicePos != pos) {
            if (!selecting) {
                dimensionID = dim;
                mainPos     = pos;
                selecting   = true;
            } else {
                if (dim != dimensionID)
                    return false;
            }
            vicePos = pos;
            updateBoundingBox();
            return true;
        }
        return false;
    }

    std::pair<std::string, bool> CuboidRegion::expand(const std::vector<BlockPos>& changes) {
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

    std::pair<std::string, bool> CuboidRegion::contract(const std::vector<BlockPos>& changes) {
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

    std::pair<std::string, bool> CuboidRegion::shift(const BlockPos& change) {
        mainPos = mainPos + change;
        vicePos = vicePos + change;
        updateBoundingBox();

        return {"§aThis region has been shifted", true};
    }

    void CuboidRegion::forEachLine(const std::function<void(const BlockPos&, const BlockPos&)>& todo) {
        todo(mainPos, vicePos);
    }

    CuboidRegion::CuboidRegion() : Region{BoundingBox(), -1} {
        this->regionType    = CUBOID;
        this->needResetVice = false;
    }
    CuboidRegion::CuboidRegion(const BoundingBox& region, const int& dim) : Region(region, dim) {
        this->regionType    = CUBOID;
        this->mainPos       = region.min;
        this->vicePos       = region.max;
        this->selecting     = true;
        this->needResetVice = false;
    }
}  // namespace worldedit