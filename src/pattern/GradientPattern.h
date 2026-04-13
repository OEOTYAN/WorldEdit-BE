#pragma once

#include "data/GradientManager.h"
#include "pattern/Pattern.h"

namespace we {
class GradientPattern : public Pattern {
    bool                             lighten = false;
    std::vector<std::string>         selectors;
    std::vector<GradientData const*> gradients;
    BlockSource*                     blockSource = nullptr;

    GradientPattern(bool lighten, std::vector<std::string> selectors);

public:
    static ll::Expected<std::shared_ptr<GradientPattern>>
    create(GradientPatternAst const& ast);

    ll::Expected<> prepare(CommandContextRef const& ctx) override;

    BlockOperation pickBlock(BlockPos const& pos) const override;
};
} // namespace we