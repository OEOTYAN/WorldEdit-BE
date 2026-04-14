#pragma once

#include <mc/world/level/BlockPos.h>
#include <mc/world/level/BlockSource.h>

namespace we {
using namespace ll::math;

class ExprCtx {
    BlockSource* mBlockSource = nullptr;
    BoundingBox  mBox;

public:
    mutable BlockPos pos;
    mutable double3  realPos;
    mutable double3  unitPos;

    void setBlockSource(BlockSource* blockSource) { mBlockSource = blockSource; }

    [[nodiscard]] BlockSource* blockSource() const { return mBlockSource; }

    void setBoundingBox(BoundingBox const& box) { mBox = box; }

    void update(BlockPos const& p) {
        pos     = p;
        realPos = double3{pos};
        if (mBox.getSideLength() != 0) {
            auto center = double3{mBox.max + mBox.min} * 0.5;
            unitPos     = (realPos - center) / double3{mBox.getSideLength()} * 2.0;
        } else {
            unitPos = 0;
        }
    }
};
} // namespace we
