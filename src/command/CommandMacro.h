#pragma once

#include "data/LocalContextManager.h"
#include "worldedit/WorldEdit.h"

#include <mc/common/FacingID.h>

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
            "command [{0}] [success]: {1}",
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
            "command [{0}] [error]: {1}",
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

enum class CommandFacing : schar {
    Down  = 0,
    Up    = 1,
    North = 2,
    South = 3,
    West  = 4,
    East  = 5,
    NegY  = Down,
    PosY  = Up,
    NegZ  = North,
    PosZ  = South,
    NegX  = West,
    PosX  = East,
    Me    = 0x10,
    Back  = 0x11,
};

template <class T>
concept IsVaArg = ll::reflection::type_unprefix_name_v<T>.starts_with("we::")
               && ll::reflection::type_unprefix_name_v<T>.contains("VaArg");

optional_ref<Dimension>       checkDimension(CommandContextRef const& ctx);
optional_ref<Player>          checkPlayer(CommandContextRef const& ctx);
std::shared_ptr<Region>       checkRegion(CommandContextRef const& ctx);
std::shared_ptr<LocalContext> checkLocalContext(CommandContextRef const& ctx);
std::shared_ptr<LocalContext> getLocalContext(CommandContextRef const& ctx);
std::optional<FacingID>       checkFacing(CommandFacing, CommandContextRef const& ctx);

} // namespace we

template <we::IsVaArg T>
struct ll::command::ParamTraits<T> : ParamTraitsBase<T> {
    using Base = ParamTraitsBase<T>;
    struct Parser {
        bool
        operator()(CommandRegistry const&, void* storage, CommandRegistry::ParseToken const& token, CommandOrigin const&, int, std::string&, std::vector<std::string>&)
            const {
            std::string str = token.toString();
            ll::reflection::forEachMember(
                *(T*)storage,
                [&](std::string_view name, bool& value) {
                    value = str.contains(name.front());
                }
            );
            return true;
        }
    };
    static inline auto enumName = [] {
        std::string str{"we::va_arg_"};
        for (size_t i = 0; i < ll::reflection::member_count_v<T>; ++i) {
            str += ll::reflection::member_name_array_v<T>[i].front();
        }
        return str;
    }();
    static inline auto enumValues = [] {
        std::vector<std::pair<std::string, uint64>> values;
        for (uint64 i = 1; i < (uint64)std::exp2(ll::reflection::member_count_v<T>);
             ++i) {
            std::string arg{"-"};
            for (uint64 j = 0; j < ll::reflection::member_count_v<T>; ++j) {
                if ((i >> j) & 1)
                    arg += ll::reflection::member_name_array_v<T>[j].front();
            }
            values.emplace_back(std::move(arg), i);
        }
        return values;
    }();
    static constexpr CommandParameterDataType dataType() {
        return CommandParameterDataType::Enum;
    }
    static constexpr CommandParameterOption options() {
        return CommandParameterOption::EnumAutocompleteExpansion;
    }
    static inline std::string_view enumNameOrPostfix() {
        WE_DEBUG("{}", enumName);
        return enumName;
    }
    static void transformData(CommandParameterData&) {
        WE_DEBUG("{}", enumValues);
        CommandRegistrar::getInstance()
            .tryRegisterEnum(enumName, enumValues, Base::typeId(), Base::parseFn());
    }
};

#ifndef REG_CMD

#define REG_CMD(type, name, description)                                                 \
    [[maybe_unused]] static bool _##name =                                               \
        SetupCommandFunctor{                                                             \
            #type "." #name,                                                             \
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
