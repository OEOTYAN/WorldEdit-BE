#pragma once

#include "data/PlayerContextManager.h"
#include "worldedit/WorldEdit.h"

namespace we {

class SetupCommandFunctor {
    using Handler = optional_ref<ll::command::CommandHandle>();

    std::string name;
    Handler*    handler;

public:
    SetupCommandFunctor(std::string name, Handler* handler)
    : name(std::move(name)),
      handler(std::move(handler)) {}

    bool operator|(void (*)(ll::command::CommandHandle&)) noexcept;
};

class CommandContextRef {
public:
    CommandOrigin const& origin;
    CommandOutput&       output;
    ::Command const&     cmd;

    template <class... Args>
    void success(fmt::format_string<Args...> fmt, Args&&... args) const {
        auto fsv = fmt.get();
        WE_DEBUG(
            "command [{0}] [success]: {0}",
            cmd.getCommandName(),
            fmt::vformat(
                ll::i18n::getInstance().get({fsv.data(), fsv.size()}, {}),
                fmt::make_format_args(args...)
            )
        );
        output.success(fmt::vformat(
            ll::i18n::getInstance().get({fsv.data(), fsv.size()}, {}),
            fmt::make_format_args(args...)
        ));
    }

    template <class... Args>
    void error(fmt::format_string<Args...> fmt, Args&&... args) const {
        auto fsv = fmt.get();
        WE_DEBUG(
            "command [{0}] [error]: {0}",
            cmd.getCommandName(),
            fmt::vformat(
                ll::i18n::getInstance().get({fsv.data(), fsv.size()}, {}),
                fmt::make_format_args(args...)
            )
        );
        output.error(fmt::vformat(
            ll::i18n::getInstance().get({fsv.data(), fsv.size()}, {}),
            fmt::make_format_args(args...)
        ));
    }

    Vec3 pos() const { return origin.getWorldPosition(); }
    Vec3 blockPos() const { return origin.getBlockPosition(); }

    Vec3 transformPos(CommandPositionFloat const& pos) const {
        return origin.getExecutePosition(cmd.mVersion, pos);
    }
    auto transformPos() const {
        return
            [this](CommandPositionFloat const& cmdPos) { return transformPos(cmdPos); };
    }
};

class CmdCtxBuilder {
public:
    template <class Fn>
        requires(
            requires(Fn fn, CommandContextRef ref) { fn(ref, {}); }
            || requires(Fn fn, CommandContextRef ref) { fn(ref); }
            || requires(Fn fn) { fn(); }
        )
    constexpr auto operator|(Fn&& fn) const noexcept {
        return [fn = std::forward<Fn>(fn
                )](CommandOrigin const& origin,
                   CommandOutput&       output,
                   auto&                param,
                   ::Command const&     cmd) {
            CommandContextRef ctxref{origin, output, cmd};
            if constexpr (std::is_invocable_v<
                              Fn,
                              CommandContextRef const&,
                              decltype((param))>) {
                fn(ctxref, param);
            } else if constexpr (std::is_invocable_v<Fn, CommandContextRef const&>) {
                fn(ctxref);
            } else if constexpr (std::is_invocable_v<Fn>) {
                fn();
            } else {
                static_assert(false);
            }
        };
    }
};

optional_ref<Dimension>        checkDimension(CommandContextRef const& ctx);
optional_ref<Player>           checkPlayer(CommandContextRef const& ctx);
std::shared_ptr<Region>        checkRegion(CommandContextRef const& ctx);
std::shared_ptr<PlayerContext> checkPlayerContext(CommandContextRef const& ctx);
std::shared_ptr<PlayerContext> getPlayerContext(CommandContextRef const& ctx);
} // namespace we

#ifndef REG_CMD

#define REG_CMD(type, name, description)                                                 \
    [[maybe_unused]] static bool _##name =                                               \
        SetupCommandFunctor{                                                             \
            #name,                                                                       \
            []() -> optional_ref<ll::command::CommandHandle> {                           \
                auto& config = WorldEdit::getInstance().getConfig().commands.type.name;  \
                if (!config.enabled) {                                                   \
                    return std::nullopt;                                                 \
                }                                                                        \
                return ll::command::CommandRegistrar::getInstance()                      \
                    .getOrCreateCommand(#name, description##_tr(), config.permission);   \
            }                                                                            \
        }                                                                                \
        | [](ll::command::CommandHandle & command)
#endif
