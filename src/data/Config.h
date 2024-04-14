#pragma once

#include "region/RegionType.h"
#include "worldedit/Global.h"

#include <ll/api/reflection/Dispatcher.h>
#include <mc/server/commands/CommandPermissionLevel.h>

namespace we {
struct Config {
    int version = 0;
    struct CmdSetting {
        bool                   enabled    = true;
        CommandPermissionLevel permission = CommandPermissionLevel::GameDirectors;
    };
    struct {
        struct {
            CmdSetting sel{};
        } region;
    } commands{};
    struct {
        mce::Color region_line_color{"#FFEC27"};
        mce::Color region_point_color{"#10E436"};
        mce::Color main_hand_color{"#FF3040"};
        mce::Color off_hand_color{"#29ADFF"};
    } colors{};
    struct {
        double maximum_brush_length  = 2048;
        int    minimum_response_tick = 3;
    } player_state;
    struct {
        int player_log_level = 3;
    } log;

    struct PlayerConfig {
        RegionType default_region_type{RegionType::Cuboid};
    } player_default_config;
};
} // namespace we

// "#000000"
// "#144A74"
// "#8E65F3"
// "#07946E"
// "#AB5236"
// "#56575F"
// "#A2A3A7"
// "#FFFFFF"
// "#FF3040"
// "#FF7300"
// "#FFEC27"
// "#10E436"
// "#29ADFF"
// "#83769C"
// "#FF77A8"
// "#FFCCAA"
