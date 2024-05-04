#pragma once

#include "CuboidRegion.h"

namespace we {
class ExpandRegion : public CuboidRegion {

public:
    ExpandRegion(DimensionType, BoundingBox const&);

    RegionType getType() const override { return RegionType::Expand; }

    bool needResetVice() const override { return true; }

    bool setMainPos(BlockPos const&) override;

    bool setOffPos(BlockPos const&) override;
};
} // namespace we
