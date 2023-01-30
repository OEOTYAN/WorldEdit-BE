
//
// Created by OEOTYAN on 2022/09/04.
//
#include "allCommand.hpp"
#include <ScheduleAPI.h>
#include "filesys/file.h"
#include "eval/Eval.h"
#include "WorldEdit.h"
#include "string/StringTool.h"

#include "MC/Level.hpp"
#include "MC/Block.hpp"
#include "MC/MobSpawnRules.hpp"
// #include "MC/SpawnGroupData.hpp"
// #include "MC/SpawnGroupRegistry.hpp"

#include "Nlohmann/json.hpp"

namespace worldedit {
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
                std::vector<std::string> blocksName;
                blocksName.clear();
                blocksName.push_back("#clipboard");
                blocksName.push_back("#hand");

                nlohmann::json blockList;
                std::ifstream i(WE_DIR + "mappings/blocks.json");
                i >> blockList;

                for (auto& b : blockList.items()) {
                    std::string key = b.key();
                    if (!isBEBlock(key)) {
                        Block* block = nullptr;
                        if (b.value().contains("bedrock_states")) {
                            std::string snbt = "{\"name\":\"";
                            snbt += b.value()["bedrock_identifier"];
                            snbt += "\",\"states\":";
                            std::string states = b.value()["bedrock_states"].dump();
                            stringReplace(states, ":false", ":0b");
                            stringReplace(states, ":true", ":1b");
                            snbt += states;
                            snbt += "}";
                            block = Block::create(CompoundTag::fromSNBT(snbt).get());
                            // if (block == nullptr)
                            //     std::cout << snbt << std::endl;
                        } else {
                            block = Block::create(b.value()["bedrock_identifier"], 0);
                        }
                        if (block != nullptr) {
                            getJavaBlockMap()[key] = block;
                            blocksName.push_back(key);
                        }
                    }
                }

                Global<CommandRegistry>->setSoftEnumValues("blockPattern", blocksName);
            },
            20);

        DynamicCommand::setup(
            "updateimage",                                    // command name
            tr("worldedit.command.description.updateimage"),  // command description
            {}, {}, {{}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                std::vector<std::string> test;
                getImageFiles(WE_DIR + "image", test);
                Global<CommandRegistry>->setSoftEnumValues("filename", test);
                output.trSuccess("worldedit.updateimage.success");
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
            "neighberupdate",                                // command name
            "worldedit.command.description.neighberupdate",  // command description
            {}, {ParamData("bool", ParamType::Bool, "bool")}, {{"bool"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                auto& playerData = getPlayersData(xuid);
                if (results["bool"].get<bool>()) {
                    playerData.updateArg = 3;
                    output.trSuccess("worldedit.neighberupdate.on");
                } else {
                    playerData.updateArg = 2;
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
        //     "wetest",                    // command name
        //     "calculate expression",  // command description
        //     {}, {ParamData("exp", ParamType::Message, "exp")}, {{"exp"}},
        //     // dynamic command callback
        //     [](DynamicCommand const& command, CommandOrigin const& origin,
        //     CommandOutput& output,
        //        std::unordered_map<std::string, DynamicCommand::Result>&
        //        results) {
        //
        //         auto player = origin.getPlayer();
        //         auto dimID = player->getDimensionId();
        //         auto blockSource = &player->getRegion();

        //         auto* spawnGroupRegistry =
        //         blockSource->getLevel().getSpawnGroupRegistry();

        //         for (auto& m : spawnGroupRegistry->mSpawnGroupLookupMap) {
        //             std::cout << m.first << std::endl;
        //         }
        //         output.trSuccess("");
        //     },
        //     CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit