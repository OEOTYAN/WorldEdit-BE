//
// Created by OEOTYAN on 2021/2/8.
//

#include "CuboidRegion.h"
namespace worldedit {
    void CuboidRegion::updateBoundingBox() {
        rendertick = 0;
        boundingBox.bpos1.x = std::min(mainPos.x, vicePos.x);
        boundingBox.bpos1.y = std::min(mainPos.y, vicePos.y);
        boundingBox.bpos1.z = std::min(mainPos.z, vicePos.z);
        boundingBox.bpos2.x = std::max(mainPos.x, vicePos.x);
        boundingBox.bpos2.y = std::max(mainPos.y, vicePos.y);
        boundingBox.bpos2.z = std::max(mainPos.z, vicePos.z);
    }

    bool CuboidRegion::setMainPos(const BlockPos& pos, const int& dim) {
        if (mainPos != pos) {
            if (!selecting) {
                dimensionID = dim;
                vicePos = pos;
                selecting = true;
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
                mainPos = pos;
                selecting = true;
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

    bool CuboidRegion::expand(const BlockPos& change, Player* player) {
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

        updateBoundingBox();

        player->sendFormattedText("§aThis region has been expanded");
        return true;
    }

    bool CuboidRegion::contract(const BlockPos& change, Player* player) {
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

        updateBoundingBox();
        player->sendFormattedText("§aThis region has been contracted");
        return true;
    }

    bool CuboidRegion::expand(const std::vector<BlockPos>& changes,
                              Player* player) {
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

        player->sendFormattedText("§aThis region has been expanded");
        return true;
    }

    bool CuboidRegion::contract(const std::vector<BlockPos>& changes,
                                Player* player) {
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
        player->sendFormattedText("§aThis region has been contracted");
        return true;
    }

    bool CuboidRegion::shift(const BlockPos& change, Player* player) {
        mainPos = mainPos + change;
        vicePos = vicePos + change;
        updateBoundingBox();

        player->sendFormattedText("§aThis region has been shifted");
        return true;
    }

    CuboidRegion::CuboidRegion() : Region{BoundingBox(), -1} {
        this->regionType = CUBOID;
    }
    CuboidRegion::CuboidRegion(const BoundingBox& region, const int& dim)
        : Region(region, dim) {
        this->regionType = CUBOID;
        this->mainPos = region.bpos1;
        this->vicePos = region.bpos2;
        this->selecting = true;
    }
}  // namespace worldedit