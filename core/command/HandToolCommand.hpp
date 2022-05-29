//
// Created by OEOTYAN on 2022/05/20.
//
#pragma once
#ifndef WORLDEDIT_HANDTOOLCOMMAND_H
#define WORLDEDIT_HANDTOOLCOMMAND_H

#include "pch.h"
// #include <MC/Level.hpp>
// #include <MC/BlockInstance.hpp>
// #include <MC/Block.hpp>
// #include <MC/BlockActor.hpp>
// #include <MC/BlockSource.hpp>
// #include <MC/CompoundTag.hpp>
// #include <MC/Actor.hpp>
// #include <MC/Player.hpp>
// #include <MC/ServerPlayer.hpp>
// #include <MC/Dimension.hpp>
// #include <MC/ItemStack.hpp>
// #include "Version.h"
#include "string/StringTool.h"
// #include <LLAPI.h>
// #include <ServerAPI.h>
// #include <EventAPI.h>
// #include <ScheduleAPI.h>
// #include <DynamicCommandAPI.h>
// #include "WorldEdit.h"

namespace worldedit {
    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;

    // tool

    void handToolCommandSetup() {
        DynamicCommand::setup(
            "tool",                // command name
            "set your hand tool",  // command description
            {
                {"tree", {"tree"}},
                {"deltree", {"deltree"}},
                {"farwand", {"farwand"}},
                {"cycler", {"cycler"}},
                {"info", {"info"}},
                {"flood", {"flood"}},
                {"none", {"none"}},
            },
            {
                ParamData("tree", ParamType::Enum, "tree"),
                ParamData("deltree", ParamType::Enum, "deltree"),
                ParamData("farwand", ParamType::Enum, "farwand"),
                ParamData("cycler", ParamType::Enum, "cycler"),
                ParamData("info", ParamType::Enum, "info"),
                ParamData("flood", ParamType::Enum, "flood"),
                ParamData("none", ParamType::Enum, "none"),
                ParamData("blockPattern", ParamType::String, "blockPattern"),
                ParamData("distance", ParamType::Float, "distance"),
            },
            {
                {"tree"},
                {"deltree"},
                {"farwand"},
                {"cycler"},
                {"info"},
                {"flood", "blockPattern", "distance"},
                {"none"},
            },
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                std::string toolName =
                    origin.getPlayer()->getHandSlot()->getTypeName();
                if (results["tree"].isSet) {
                    mod.playerHandToolMap[xuid][toolName] = "tree";
                    stringReplace(toolName, "minecraft:", "");
                    output.success(
                        fmt::format("§aTree tool bound to {}", toolName));
                } else if (results["deltree"].isSet) {
                    mod.playerHandToolMap[xuid][toolName] = "deltree";
                    stringReplace(toolName, "minecraft:", "");
                    output.success(
                        fmt::format("§aDeltree tool bound to {}", toolName));
                } else if (results["farwand"].isSet) {
                    mod.playerHandToolMap[xuid][toolName] = "farwand";
                    stringReplace(toolName, "minecraft:", "");
                    output.success(
                        fmt::format("§aFar wand tool bound to {}", toolName));
                } else if (results["cycler"].isSet) {
                    mod.playerHandToolMap[xuid][toolName] = "cycler";
                    stringReplace(toolName, "minecraft:", "");
                    output.success(
                        fmt::format("§aCycler tool bound to {}", toolName));
                } else if (results["info"].isSet) {
                    mod.playerHandToolMap[xuid][toolName] = "info";
                    stringReplace(toolName, "minecraft:", "");
                    output.success(
                        fmt::format("§aBlock info tool bound to {}", toolName));
                } else if (results["flood"].isSet) {
                } else if (results["none"].isSet) {
                    mod.playerHandToolMap[xuid].erase(toolName);
                    output.success(fmt::format("§aHand tool cleared"));
                }
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit

#endif  // WORLDEDIT_HANDTOOLCOMMAND_H