#include "CommandMacro.h"
#include "Command.h"
#include "utils/FacingUtils.h"

namespace we {
auto& setupCommandFunctions() {
    static ll::OrderedMap<std::string, std::function<void()>> ins;
    return ins;
}
void setupCommands() {
    for (auto& [name, fn] : setupCommandFunctions()) {
        WE_DEBUG("setup command {0}", name);
        try {
            fn();
        } catch (...) {
            logger().error("threw from setup command {0}", name);
            ll::error_utils::printCurrentException(logger());
        }
        WE_DEBUG("setup command {0} done", name);
    }
}
bool SetupCommandFunctor::operator|(void (*fn)(ll::command::CommandHandle&)) noexcept {
    auto& map = setupCommandFunctions();
    WE_ASSERT(!map.contains(name), "setup same cmd twice");
    map.try_emplace(std::move(name), [handler = handler, fn] {
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
std::shared_ptr<LocalContext> checkLocalContext(CommandContextRef const& ctx) {
    auto lctx = WorldEdit::getInstance().getLocalContextManager().get(ctx.origin);
    if (!lctx) {
        ctx.error("origin didn't have LocalContext");
    }
    return lctx;
}
std::shared_ptr<LocalContext> getLocalContext(CommandContextRef const& ctx) {
    return WorldEdit::getInstance().getLocalContextManager().getOrCreate(ctx.origin);
}
std::shared_ptr<Region> checkRegion(CommandContextRef const& ctx) {
    auto lctx = checkLocalContext(ctx);
    if (!lctx) return nullptr;
    if (!lctx->region) {
        ctx.error("origin didn't selected any region");
        return nullptr;
    } else {
        return lctx->region;
    }
}
std::optional<FacingID> checkFacing(CommandFacing facing, CommandContextRef const& ctx) {
    if (facing == CommandFacing::Me) {
        auto player = checkPlayer(ctx);
        if (!player) return std::nullopt;
        return dirToFacing(player->getViewVector());
    } else if (facing == CommandFacing::Back) {
        auto player = checkPlayer(ctx);
        if (!player) return std::nullopt;
        return opposite(dirToFacing(player->getViewVector()));
    }
    return (FacingID)facing;
}
} // namespace we
