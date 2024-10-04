#include "CommandMacro.h"
#include "Command.h"

namespace we {
auto& setupCommandFunctions() {
    static ll::DenseMap<std::string, std::function<void()>> ins;
    return ins;
}
void setupCommands() {
    for (auto& [name, fn] : setupCommandFunctions()) {
        WE_DEBUG("setup command {}", name);
        try {
            fn();
        } catch (...) {
            logger().error("threw from setup command {}", name);
            ll::error_utils::printCurrentException(logger());
        }
        WE_DEBUG("setup command {} done", name);
    }
}
bool SetupCommandFunctor::operator|(void (*fn)(ll::command::CommandHandle&)) noexcept {
    auto& map = setupCommandFunctions();
    WE_ASSERT(!map.contains(name), "setup same cmd twice");
    setupCommandFunctions().try_emplace(std::move(name), [handler = handler, fn] {
        handler().transform([&](auto& handle) {
            fn(handle);
            return true;
        });
    });
    return true;
}

optional_ref<Dimension> checkDimension(CommandContextRef const& ctx) {
    auto* dim = ctx.origin.getDimension();
    if (!dim) {
        ctx.error("origin doesn't have dimension");
    }
    return dim;
}
optional_ref<Player> checkPlayer(CommandContextRef const& ctx) {
    auto* e = ctx.origin.getEntity();
    if (e && e->isPlayer()) {
        return (Player*)e;
    } else {
        ctx.error("origin isn't player");
        return nullptr;
    }
}
std::shared_ptr<PlayerContext> checkPlayerContext(CommandContextRef const& ctx) {
    auto pctx = WorldEdit::getInstance().getPlayerContextManager().get(ctx.origin);
    if (!pctx) {
        ctx.error("origin didn't have pctx");
    }
    return pctx;
}
std::shared_ptr<PlayerContext> getPlayerContext(CommandContextRef const& ctx) {
    return WorldEdit::getInstance().getPlayerContextManager().getOrCreate(ctx.origin);
}
std::shared_ptr<Region> checkRegion(CommandContextRef const& ctx) {
    if (auto pctx = checkPlayerContext(ctx); !pctx->region) {
        ctx.error("origin didn't selected any region");
        return nullptr;
    } else {
        return pctx->region;
    }
}
} // namespace we
