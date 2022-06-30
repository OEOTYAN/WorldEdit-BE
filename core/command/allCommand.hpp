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
    void commandsSetup() {
        Schedule::delay(
            [&]() {
                std::vector<std::string> imagesName;
                getImageFiles(WE_DIR + "image", imagesName);
                Global<CommandRegistry>->setSoftEnumValues("filename", imagesName);

                std::vector<std::string> arg_hcr;
                std::vector<std::string> arg_hv;
                std::vector<std::string> arg_h;
                std::vector<std::string> arg_aho;
                std::vector<std::string> arg_sa;
                std::vector<std::string> arg_sal;
                std::vector<std::string> arg_anos;
                std::vector<std::string> arg_ca;
                std::vector<std::string> arg_c;
                std::vector<std::string> arg_cd;
                arg_aho.push_back("-a");
                arg_sa.push_back("-a");
                arg_sal.push_back("-a");
                arg_anos.push_back("-a");
                arg_ca.push_back("-a");
                arg_hcr.push_back("-h");
                arg_hv.push_back("-h");
                arg_h.push_back("-h");
                arg_aho.push_back("-h");
                arg_anos.push_back("-n");
                arg_anos.push_back("-o");
                arg_aho.push_back("-o");
                arg_hv.push_back("-v");
                arg_ca.push_back("-c");
                arg_c.push_back("-c");
                arg_cd.push_back("-c");
                arg_hcr.push_back("-c");
                arg_cd.push_back("-d");
                arg_hcr.push_back("-r");
                arg_sal.push_back("-l");
                arg_sa.push_back("-s");
                arg_sal.push_back("-s");
                arg_anos.push_back("-s");
                Global<CommandRegistry>->setSoftEnumValues("-aho", arg_aho);
                Global<CommandRegistry>->setSoftEnumValues("-anos", arg_anos);
                Global<CommandRegistry>->setSoftEnumValues("-h", arg_h);
                Global<CommandRegistry>->setSoftEnumValues("-c", arg_c);
                Global<CommandRegistry>->setSoftEnumValues("-ca", arg_ca);
                Global<CommandRegistry>->setSoftEnumValues("-cd", arg_cd);
                Global<CommandRegistry>->setSoftEnumValues("-hv", arg_hv);
                Global<CommandRegistry>->setSoftEnumValues("-hcr", arg_hcr);
                Global<CommandRegistry>->setSoftEnumValues("-sa", arg_sa);
                Global<CommandRegistry>->setSoftEnumValues("-sal", arg_sal);
            },
            20);

        brushCommandSetup();
        regionCommandSetup();
        historyCommandSetup();
        handToolCommandSetup();
        clipboardCommandSetup();
        regionInfoCommandSetup();
        generationCommandSetup();
        regionOperationCommandSetup();

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
    }
}  // namespace worldedit

#endif  // WORLDEDIT_ALLCOMMAND_H