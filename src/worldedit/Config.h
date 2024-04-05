#pragma once

#include <ll/api/reflection/Dispatcher.h>
#include <mc/server/commands/CommandPermissionLevel.h>

namespace we {
struct Config {
    int version = 1;
    struct CmdSetting {
        bool                   enabled    = true;
        CommandPermissionLevel permission = CommandPermissionLevel::GameDirectors;
    };
    struct {
        CmdSetting weconfig{};
    } commands{};
};
} // namespace we
