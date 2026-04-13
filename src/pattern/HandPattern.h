#pragma once

#include "pattern/Pattern.h"

namespace we {
class HandPattern : public Pattern {
    optional_ref<Block const> handBlock = nullptr;

public:
    static ll::Expected<std::shared_ptr<HandPattern>> create();

    ll::Expected<> prepare(CommandContextRef const& ctx) override;

    BlockOperation pickBlock(BlockPos const& pos) const override;
};
} // namespace we