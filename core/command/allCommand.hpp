//
// Created by OEOTYAN on 2022/05/16.
//
#pragma once
#ifndef WORLDEDIT_ALLCOMMAND_H
#define WORLDEDIT_ALLCOMMAND_H
#include "command/RegionCommand.hpp"
#include "command/HistoryCommand.hpp"
#include "command/HandToolCommand.hpp"
#include "command/ClipboardCommand.hpp"
#include "command/RegionInfoCommand.hpp"
#include "command/RegionOperationCommand.hpp"

namespace worldedit {
    // Direct setup of dynamic command with necessary information
    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;
    void commandsSetup() {
        regionCommandSetup();
        historyCommandSetup();
        handToolCommandSetup();
        clipboardCommandSetup();
        regionInfoCommandSetup();
        regionOperationCommandSetup();

        DynamicCommand::setup(
            "gmask",            // command name
            "set global mask",  // command description
            {}, {ParamData("mask", ParamType::String, true, "mask")},
            {{"mask"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (results["mask"].isSet) {
                    auto str = results["mask"].getRaw<std::string>();
                    mod.playerGMaskMap[xuid] = str;
                    output.success(
                        fmt::format("§aglobal mask set to: §g{}", str));
                } else {
                    mod.playerGMaskMap.erase(xuid);
                    output.success(
                        fmt::format("§aYour global mask is cleared"));
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "needblockupdate",                        // command name
            "whether the block needs to be updated",  // command description
            {}, {ParamData("bool", ParamType::Bool, "bool")}, {{"bool"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (results["bool"].get<bool>()) {
                    mod.updateArg = 3;
                    output.success(
                        fmt::format("§aBlock update is now enabled"));
                } else {
                    mod.updateArg = 2;
                    output.success(
                        fmt::format("§aBlock update is now disabled"));
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "debugblockupdate",           // command name
            "§cDo not use this command",  // command description
            {}, {ParamData("int", ParamType::Int, "int")}, {{"int"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                mod.updateArg = results["int"].get<int>();
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit

#endif  // WORLDEDIT_ALLCOMMAND_H