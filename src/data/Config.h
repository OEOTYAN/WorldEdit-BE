#pragma once

#include "region/RegionType.h"
#include "worldedit/Global.h"

#include <ll/api/reflection/Dispatcher.h>

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
            CmdSetting loftcycle{};
            CmdSetting pos1{};
            CmdSetting pos2{};
            CmdSetting hpos1{};
            CmdSetting hpos2{};
            CmdSetting chunk{};
            CmdSetting contract{};
            CmdSetting expand{};
            CmdSetting shift{};
            CmdSetting outset{};
            CmdSetting inset{};
        } region;
        struct {
            CmdSetting undo{};
            CmdSetting redo{};
            CmdSetting clearhistory{};
            CmdSetting maxhistorylength{};

        } history;
        struct {
            CmdSetting set{};
        } edit;
    } commands{};
    struct {
        mce::Color region_line_color{"#FFEC27"};
        mce::Color region_point_color{"#10E436"};
        mce::Color region_point_color2{"#94FFD8"};
        mce::Color main_hand_color{"#FF3040"};
        mce::Color off_hand_color{"#29ADFF"};
    } colors{};
    struct {
        float maximum_trace_length  = 2048;
        int   minimum_response_tick = 3;
        bool  allow_bot_builder     = false;
        bool  allow_inplace_builder = true;
    } player_state;
    struct {
        ll::io::LogLevel player_log_level{ll::io::LogLevel::Warn};
    } log;

    struct PlayerConfig {
        RegionType   default_region_type{RegionType::Expand};
        HashedString wand = VanillaItemNames::WoodenAxe();
        std::string  global_mask;
        size_t       history_length = 128;
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
