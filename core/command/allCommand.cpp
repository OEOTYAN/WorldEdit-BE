
//
// Created by OEOTYAN on 2022/09/04.
//
#include "allCommand.hpp"
#include <ScheduleAPI.h>
#include "filesys/file.h"
#include "eval/Eval.h"
#include "WorldEdit.h"
#include "utils/StringTool.h"
#include <mc/CommandBlockNameResult.hpp>

#include "mc/Level.hpp"
#include "mc/Block.hpp"
#include "mc/MobSpawnRules.hpp"
#include "mc/SpawnGroupData.hpp"
#include "mc/SpawnGroupRegistry.hpp"

namespace worldedit {
    void updateFile() {
        Global<CommandRegistry>->setSoftEnumValues("imagefilename", getWEFiles("image"));
        Global<CommandRegistry>->setSoftEnumValues(
            "modelfilename",
            getWEFiles("models", [](std::string_view s) -> bool { return s.substr(s.size() - 4) == ".obj"; }));
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
                updateFile();
                setArg("-ahor");
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
                std::vector<std::string> blocksName;
                blocksName.clear();
                blocksName.push_back("#clipboard");
                blocksName.push_back("#lighten");
                blocksName.push_back("#darken");
                blocksName.push_back("#hand");

                for (auto& b : getJavaBlockMap()) {
                    if (!isBEBlock(b.first)) {
                        blocksName.push_back(b.first);
                    }
                }

                Global<CommandRegistry>->setSoftEnumValues("blockPattern", blocksName);
            },
            20);

        DynamicCommand::setup(
            "updatefile",                                    // command name
            tr("worldedit.command.description.updatefile"),  // command description
            {}, {}, {{}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                updateFile();
                output.trSuccess("worldedit.updatefile.success");
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "gmask",                                    // command name
            tr("worldedit.command.description.gmask"),  // command description
            {}, {ParamData("mask", ParamType::String, true, "mask")}, {{"mask"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                auto& playerData = getPlayersData(xuid);
                if (results["mask"].isSet) {
                    auto str = results["mask"].getRaw<std::string>();
                    playerData.gMask = str;
                    output.trSuccess("worldedit.gmask.success", str);
                } else {
                    playerData.gMask = "";
                    output.trSuccess("worldedit.gmask.clear");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "neighberupdate",                                    // command name
            tr("worldedit.command.description.neighberupdate"),  // command description
            {}, {ParamData("bool", ParamType::Bool, "bool")}, {{"bool"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                auto& playerData = getPlayersData(xuid);
                if (results["bool"].get<bool>()) {
                    playerData.updateArg = 1;
                    output.trSuccess("worldedit.neighberupdate.on");
                } else {
                    playerData.updateArg = 0;
                    output.trSuccess("worldedit.neighberupdate.off");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "blockupdate",                                    // command name
            tr("worldedit.command.description.blockupdate"),  // command description
            {}, {ParamData("bool", ParamType::Bool, "bool")}, {{"bool"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                auto& playerData = getPlayersData(xuid);
                if (results["bool"].get<bool>()) {
                    playerData.updateExArg = 1;
                    output.trSuccess("worldedit.blockupdate.on");
                } else {
                    playerData.updateExArg = 0;
                    output.trSuccess("worldedit.blockupdate.off");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "wand",                                    // command name
            tr("worldedit.command.description.wand"),  // command description
            {}, {}, {{}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto player = origin.getPlayer();
                Level::runcmdAs(player, "give @s wooden_axe");
                output.trSuccess("");
            },
            CommandPermissionLevel::GameMasters);

        // DynamicCommand::setup(
        //     "wetest",  // command name
        //     "wetest",  // command description
        //     {}, {
        //         ParamData("block", ParamType::Block, "block"),}, {{"block"}},
        //     // dynamic command callback
        //     [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
        //        std::unordered_map<std::string, DynamicCommand::Result>& results) {

        //         auto player = origin.getPlayer();
        //         auto dimID = player->getDimensionId();
        //         auto blockSource = &player->getDimensionBlockSource();

        //         output.trSuccess(results["block"].get<CommandBlockName>().resolveBlock(0).getBlock()->getTypeName());
        //     },
        //     CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit