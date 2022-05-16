//
// Created by OEOTYAN on 2021/2/8.
//

#include "ExpandRegion.h"
namespace worldedit {
    void ExpandRegion::updateBoundingBox() {
        rendertick = 0;
        boundingBox.bpos1.x = std::min(mainPos.x, vicePos.x);
        boundingBox.bpos1.y = std::min(mainPos.y, vicePos.y);
        boundingBox.bpos1.z = std::min(mainPos.z, vicePos.z);
        boundingBox.bpos2.x = std::max(mainPos.x, vicePos.x);
        boundingBox.bpos2.y = std::max(mainPos.y, vicePos.y);
        boundingBox.bpos2.z = std::max(mainPos.z, vicePos.z);
    }

    bool ExpandRegion::setMainPos(const BlockPos& pos, const int& dim) {
        selecting = true;
        dimensionID = dim;
        mainPos = pos;
        vicePos = pos;
        updateBoundingBox();
        return true;
    }

    bool ExpandRegion::setVicePos(const BlockPos& pos, const int& dim) {
        if (!selecting || dim != dimensionID) {
            return false;
        }
        if (!pos.containedWithin(boundingBox.bpos1, boundingBox.bpos2)) {
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

    bool ExpandRegion::expand(const BlockPos& change, Player* player) {
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

    bool ExpandRegion::contract(const BlockPos& change, Player* player) {
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

    bool ExpandRegion::expand(const std::vector<BlockPos>& changes,
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

    bool ExpandRegion::contract(const std::vector<BlockPos>& changes,
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

    bool ExpandRegion::shift(const BlockPos& change, Player* player) {
        mainPos = mainPos + change;
        vicePos = vicePos + change;
        updateBoundingBox();

        player->sendFormattedText("§aThis region has been shifted");
        return true;
    }

    ExpandRegion::ExpandRegion(const BoundingBox& region, const int& dim)
        : Region(region, dim), mainPos(region.bpos1), vicePos(region.bpos2) {
        this->regionType = EXPAND;
        this->selecting = true;
    }
}