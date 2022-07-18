//
// Created by OEOTYAN on 2022/05/16.
//
#pragma once
#ifndef WORLDEDIT_ALLCOMMAND_H
#define WORLDEDIT_ALLCOMMAND_H
#include "filesys/file.h"
#include "command/BrushCommand.hpp"
#include "command/RegionCommand.hpp"
#include "command/HistoryCommand.hpp"
#include "command/HandToolCommand.hpp"
#include "command/ClipboardCommand.hpp"
#include "command/RegionInfoCommand.hpp"
#include "command/GenerationCommand.hpp"
#include "command/RegionOperationCommand.hpp"

namespace worldedit {
    // Direct setup of dynamic command with necessary information
    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;

    void setArg(std::string const& args) {
        std::vector<std::string> arg;
        arg.clear();
        for (auto& c : args.substr(1)) {
            arg.push_back(std::string("-") + c);
        }
        Global<CommandRegistry>->setSoftEnumValues(args, arg);
    }

    void commandsSetup() {
        brushCommandSetup();
        regionCommandSetup();
        historyCommandSetup();
        handToolCommandSetup();
        clipboardCommandSetup();
        regionInfoCommandSetup();
        generationCommandSetup();
        regionOperationCommandSetup();

        Schedule::delay(
            [&]() {
                std::vector<std::string> imagesName;
                getImageFiles(WE_DIR + "image", imagesName);
                Global<CommandRegistry>->setSoftEnumValues("filename", imagesName);
                setArg("-aho");
                setArg("-anose");
                setArg("-h");
                setArg("-l");
                setArg("-c");
                setArg("-ca");
                setArg("-cd");
                setArg("-hv");
                setArg("-hcr");
                setArg("-sa");
                setArg("-sale");
            },
            20);

        DynamicCommand::setup(
            "updateimage",             // command name
            "update image file enum",  // command description
            {}, {}, {{}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                std::vector<std::string> test;
                getImageFiles(WE_DIR + "image", test);
                Global<CommandRegistry>->setSoftEnumValues("filename", test);
                output.success("§aUpdate image file enum success");
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "gmask",            // command name
            "set global mask",  // command description
            {}, {ParamData("mask", ParamType::String, true, "mask")}, {{"mask"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (results["mask"].isSet) {
                    auto str = results["mask"].getRaw<std::string>();
                    mod.playerGMaskMap[xuid] = str;
                    output.success(fmt::format("§aglobal mask set to: §g{}", str));
                } else {
                    mod.playerGMaskMap.erase(xuid);
                    output.success(fmt::format("§aYour global mask is cleared"));
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "needneighberupdate",                              // command name
            "whether the neighber block needs to be updated",  // command description
            {}, {ParamData("bool", ParamType::Bool, "bool")}, {{"bool"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (results["bool"].get<bool>()) {
                    mod.updateArg = 3;
                    output.success(fmt::format("§aNeighber block update is now enabled"));
                } else {
                    mod.updateArg = 2;
                    output.success(fmt::format("§aNeighber block update is now disabled"));
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "needblockupdate",                        // command name
            "whether the block needs to be updated",  // command description
            {}, {ParamData("bool", ParamType::Bool, "bool")}, {{"bool"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (results["bool"].get<bool>()) {
                    mod.updateExArg = 1;
                    output.success(fmt::format("§aBlock update is now enabled"));
                } else {
                    mod.updateExArg = 0;
                    output.success(fmt::format("§aBlock update is now disabled"));
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "wand",             // command name
            "give you a wand",  // command description
            {}, {}, {{}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                Level::runcmdAs(player, "give @s wooden_axe");
                output.success("");
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit

#endif  // WORLDEDIT_ALLCOMMAND_H