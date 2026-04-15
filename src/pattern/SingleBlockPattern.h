#pragma once

#include "pattern/Pattern.h"

namespace we {
class SingleBlockPattern : public Pattern {
public:
    optional_ref<Block const> block = nullptr;

    static ll::Expected<std::shared_ptr<SingleBlockPattern>> create();

    BlockOperation pickBlock(BlockPos const& pos) const override;
};
} // namespace we
