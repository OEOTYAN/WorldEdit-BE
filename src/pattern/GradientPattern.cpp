#include "pattern/GradientPattern.h"

#include "command/CommandMacro.h"

namespace we {
static std::pair<std::string, std::string> splitSelector(std::string_view selector) {
    auto pos = selector.find(':');
    if (pos == std::string_view::npos) {
        return {"default", std::string(selector)};
    }
    return {std::string(selector.substr(0, pos)), std::string(selector.substr(pos + 1))};
}

static int findGradientIndex(GradientData const& gradient, Block const& block) {
    auto const& name = block.getTypeName();
    for (size_t i = 0; i < gradient.blocks.size(); ++i) {
        if (gradient.blocks[i] == name) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

GradientPattern::GradientPattern(bool lighten, std::vector<std::string> selectors)
: lighten(lighten),
  selectors(std::move(selectors)) {}

ll::Expected<std::shared_ptr<GradientPattern>>
GradientPattern::create(GradientPatternAst const& ast) {
    return std::shared_ptr<GradientPattern>(
        new GradientPattern(ast.lighten, ast.selectors)
    );
}

ll::Expected<> GradientPattern::prepare(CommandContextRef const& ctx) {
    auto dim = checkDimension(ctx);
    if (!dim) {
        return ll::makeStringError("origin doesn't have dimension");
    }

    blockSource = &dim->getBlockSourceFromMainChunkSource();
    gradients.clear();

    static GradientManager manager;
    auto                   loadResult = manager.loadAll();
    if (!loadResult) {
        return ll::forwardError(loadResult.error());
    }

    for (auto const& selector : selectors) {
        auto [namespaceName, gradientName] = splitSelector(selector);
        auto gradient                      = manager.tryGet(namespaceName, gradientName);
        if (!gradient) {
            return ll::makeStringError(
                "unknown gradient: " + namespaceName + ":" + gradientName
            );
        }
        gradients.push_back(&gradient.value());
    }

    return {};
}

BlockOperation GradientPattern::pickBlock(BlockPos const& pos) const {
    BlockOperation operation;
    if (!blockSource || gradients.empty()) {
        return operation;
    }

    auto const& current = blockSource->getBlock(pos);
    for (auto const* gradient : gradients) {
        auto index = findGradientIndex(*gradient, current);
        if (index < 0) {
            continue;
        }

        auto nextIndex = lighten ? index + 1 : index - 1;
        if (nextIndex < 0 || nextIndex >= static_cast<int>(gradient->blocks.size())) {
            return operation;
        }

        operation.block = Block::tryGetFromRegistry(gradient->blocks[(size_t)nextIndex]);
        return operation;
    }
    return operation;
}
} // namespace we
