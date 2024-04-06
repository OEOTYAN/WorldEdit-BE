#pragma once

#include "CuboidRegion.h"

namespace we {
class ExpandRegion : public CuboidRegion {

public:
    ExpandRegion(DimensionType, BoundingBox const&);

    Type getType() const override { return Expand; }

    bool needResetVice() const override { return true; }

    bool setMainPos(BlockPos const&) override;

    bool setVicePos(BlockPos const&) override;
};
} // namespace we
