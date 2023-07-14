
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
#include "mc/BlockTypeRegistry.hpp"
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

                addFileWatch(WE_DIR + "image",
                             [&](const std::filesystem::path& path, const filewatch::Event change_type) {
                                 if (change_type == filewatch::Event::modified) {
                                     return;
                                 }
                                 auto fileName = UTF82String(path.u8string());

                                 if (fileName.find(".") == std::string::npos) {
                                     return;
                                 }

                                 ReplaceStr(fileName, "\\", "/");
                                 std::vector<std::string> fileEnum{fileName};
                                 switch (change_type) {
                                     case filewatch::Event::renamed_new:
                                     case filewatch::Event::added:
                                         Global<CommandRegistry>->addSoftEnumValues("imagefilename", fileEnum);
                                         break;
                                     case filewatch::Event::renamed_old:
                                     case filewatch::Event::removed:
                                         Global<CommandRegistry>->removeSoftEnumValues("imagefilename", fileEnum);
                                         break;
                                 }
                             });

                addFileWatch(
                    WE_DIR + "models", [&](const std::filesystem::path& path, const filewatch::Event change_type) {
                        if (change_type == filewatch::Event::modified) {
                            return;
                        }
                        auto fileName = UTF82String(path.u8string());

                        if (fileName.find(".") == std::string::npos || fileName.substr(fileName.size() - 4) != ".obj") {
                            return;
                        }

                        ReplaceStr(fileName, "\\", "/");
                        std::vector<std::string> fileEnum{fileName};
                        switch (change_type) {
                            case filewatch::Event::renamed_new:
                            case filewatch::Event::added:
                                Global<CommandRegistry>->addSoftEnumValues("modelfilename", fileEnum);
                                break;
                            case filewatch::Event::renamed_old:
                            case filewatch::Event::removed:
                                Global<CommandRegistry>->removeSoftEnumValues("modelfilename", fileEnum);
                                break;
                        }
                    });
            },
            20);

        // DynamicCommand::setup(
        //     "updatefile",                                    // command name
        //     tr("worldedit.command.description.updatefile"),  // command description
        //     {}, {}, {{}},
        //     // dynamic command callback
        //     [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
        //        std::unordered_map<std::string, DynamicCommand::Result>& results) {
        //         updateFile();
        //         output.trSuccess("worldedit.updatefile.success");
        //     },
        //     CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "gmask",                                    // command name
            tr("worldedit.command.description.gmask"),  // command description
            {}, {ParamData("mask", ParamType::String, true, "mask")}, {{"mask"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto player = origin.getPlayer();
                if (player == nullptr) {
                    output.trError("worldedit.error.noplayer");
                    return;
                }
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
                if (player == nullptr) {
                    output.trError("worldedit.error.noplayer");
                    return;
                }
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
                if (player == nullptr) {
                    output.trError("worldedit.error.noplayer");
                    return;
                }
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
                if (player == nullptr) {
                    output.trError("worldedit.error.noplayer");
                    return;
                }
                Level::runcmdAs(player, "give @s wooden_axe");
                output.trSuccess("");
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "clearplayerdatas",                                    // command name
            tr("worldedit.command.description.clearplayerdatas"),  // command description
            {}, {}, {{}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto player = origin.getPlayer();
                if (player == nullptr) {
                    output.trError("worldedit.error.noplayer");
                    return;
                }
                getPlayersDataMap().erase(player->getXuid());
                output.trSuccess("worldedit.clearplayerdatas.success");
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "wetest",  // command name
            "wetest",  // command description
            {},
            {
                ParamData("block", ParamType::Block, "block"),
                ParamData("blockstates", ParamType::BlockState, "blockstates"),
            },
            {{"block", "blockstates"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto player = origin.getPlayer();
                if (player == nullptr) {
                    output.trError("worldedit.error.noplayer");
                    return;
                }
                auto dimID = player->getDimensionId();
                auto blockSource = &player->getDimensionBlockSource();

                auto states = results["blockstates"].get<std::vector<BlockStateCommandParam>>();

                output.success(BlockTypeRegistry::getBlockNameFromNameHash(results["block"].get<CommandBlockName>()));
                for (auto& state : states) {
                    output.success(state.mBlockState + " : " + state.mValue);
                }
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit