//
// Created by OEOTYAN on 2022/05/17.
//
#pragma once
#ifndef WORLDEDIT_REGIONCOMMAND_H
#define WORLDEDIT_REGIONCOMMAND_H

//#include "pch.h"
// #include <MC/Level.hpp>
// #include <MC/BlockInstance.hpp>
// #include <MC/Block.hpp>
// #include <MC/BlockSource.hpp>
// #include <MC/Actor.hpp>
// #include <MC/Player.hpp>
// #include <MC/ServerPlayer.hpp>
#include <MC/Dimension.hpp>
// #include <MC/ItemStack.hpp>
// #include "Version.h"
// #include "eval/Eval.h"
// #include "string/StringTool.h"
// #include <LLAPI.h>
// #include <ServerAPI.h>
// #include <EventAPI.h>
// #include <ScheduleAPI.h>
#include <DynamicCommandAPI.h>
// #include "WorldEdit.h"

namespace worldedit {
    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;

    // sel pos1 pos2 hpos1 hpos2 chunk contract expand shift outset inset

    void regionCommandSetup() {
        DynamicCommand::setup(
            "sel",                  // command name
            "change/clear region",  // command description
            {
                // enums{enumName, {values...}}
                {"region",
                 {"cuboid", "extend", "poly", "sphere", "convex", "clear"}},
            },
            {// parameters(type, name, [optional], [enumOptions(also
             // enumName)], [identifier]) identifier: used to identify unique
             // parameter data, if idnetifier is not set,
             //   it is set to be the same as enumOptions or name (identifier
             //   = enumOptions.empty() ? name:enumOptions)
             ParamData("region", ParamType::Enum, "region")},
            {
                // overloads{ (type == Enum ? enumOptions : name) ...}
                {"region"},  // testenum <add|remove>
            },
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& action = results["region"].get<std::string&>();
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                if (action == "clear") {
                    if (mod.playerRegionMap.find(xuid) !=
                        mod.playerRegionMap.end()) {
                        mod.playerRegionMap[xuid]->selecting = false;
                    }
                    output.success("§aRegion has been cleared");
                } else {
                    BoundingBox tmpbox;
                    if (mod.playerRegionMap.find(xuid) !=
                        mod.playerRegionMap.end()) {
                        tmpbox = mod.playerRegionMap[xuid]->getBoundBox();
                        delete mod.playerRegionMap[xuid];
                    }
                    switch (do_hash(action.c_str())) {
                        case do_hash("cuboid"):
                            mod.playerRegionMap[xuid] =
                                worldedit::Region::createRegion(
                                    worldedit::CUBOID, tmpbox,
                                    origin.getPlayer()->getDimensionId());
                            output.success(
                                "§aRegion has been switched to cuboid");
                            break;
                        case do_hash("extend"):
                            mod.playerRegionMap[xuid] =
                                worldedit::Region::createRegion(
                                    worldedit::EXPAND, tmpbox,
                                    origin.getPlayer()->getDimensionId());
                            output.success(
                                "§aRegion has been switched to extend");
                            break;
                        case do_hash("poly"):
                            mod.playerRegionMap[xuid] =
                                worldedit::Region::createRegion(
                                    worldedit::POLY, tmpbox,
                                    origin.getPlayer()->getDimensionId());
                            output.success(
                                "§aRegion has been switched to poly");
                            break;
                        case do_hash("sphere"):
                            mod.playerRegionMap[xuid] =
                                worldedit::Region::createRegion(
                                    worldedit::SPHERE, tmpbox,
                                    origin.getPlayer()->getDimensionId());
                            output.success(
                                "§aRegion has been switched to sphere");
                            break;
                        case do_hash("convex"):
                            mod.playerRegionMap[xuid] =
                                worldedit::Region::createRegion(
                                    worldedit::CONVEX, tmpbox,
                                    origin.getPlayer()->getDimensionId());
                            output.success(
                                "§aRegion has been switched to convex");
                            break;
                        default:
                            break;
                    }
                }
                if (mod.playerMainPosMap.find(xuid) !=
                    mod.playerMainPosMap.end()) {
                    mod.playerMainPosMap.erase(xuid);
                }
                if (mod.playerVicePosMap.find(xuid) !=
                    mod.playerVicePosMap.end()) {
                    mod.playerVicePosMap.erase(xuid);
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "pos1",                // command name
            "set first position",  // command description
            {}, {ParamData("pos", ParamType::BlockPos, true, "pos")}, {{"pos"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                BlockInstance blockInstance;
                if (results["pos"].isSet) {
                    auto pos = results["pos"].get<BlockPos>();
                    blockInstance =
                        Level::getBlockInstance(pos, player->getDimensionId());
                } else {
                    blockInstance = Level::getBlockInstance(
                        player->getBlockPosCurrentlyStandingOn(player),
                        player->getDimensionId());
                }
                changeMainPos(player, blockInstance);
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "pos2",                 // command name
            "set second position",  // command description
            {}, {ParamData("pos", ParamType::BlockPos, true, "pos")}, {{"pos"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                BlockInstance blockInstance;
                if (results["pos"].isSet) {
                    auto pos = results["pos"].get<BlockPos>();
                    blockInstance =
                        Level::getBlockInstance(pos, player->getDimensionId());
                } else {
                    blockInstance = Level::getBlockInstance(
                        player->getBlockPosCurrentlyStandingOn(player),
                        player->getDimensionId());
                }
                changeVicePos(player, blockInstance);
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "hpos1",                         // command name
            "set first position from view",  // command description
            {}, {}, {{}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto player = origin.getPlayer();
                BlockInstance blockInstance;
                blockInstance = player->getBlockFromViewVector(
                    true, false, 2048.0f, true, false);
                changeMainPos(player, blockInstance);
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "hpos2",                          // command name
            "set second position from view",  // command description
            {}, {}, {{}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto player = origin.getPlayer();
                BlockInstance blockInstance;
                blockInstance = player->getBlockFromViewVector(
                    true, false, 2048.0f, true, false);
                changeVicePos(player, blockInstance);
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "chunk",              // command name
            "select your chunk",  // command description
            {}, {}, {{}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                auto heightRange = player->getDimensionConst().getHeightRange();
                if (mod.playerRegionMap.find(xuid) !=
                    mod.playerRegionMap.end()) {
                    delete mod.playerRegionMap[xuid];
                }
                mod.playerRegionMap.erase(xuid);
                BlockPos pos = player->getBlockPosCurrentlyStandingOn(player);
                pos.x = (pos.x >> 4) << 4;
                pos.z = (pos.z >> 4) << 4;
                pos.y = heightRange.min;

                BlockInstance blockInstance;
                blockInstance =
                    Level::getBlockInstance(pos, player->getDimensionId());
                changeMainPos(player, blockInstance, false);

                pos.x += 15;
                pos.z += 15;
                pos.y = heightRange.max - 1;
                blockInstance =
                    Level::getBlockInstance(pos, player->getDimensionId());
                changeVicePos(player, blockInstance, false);

                output.success("§aThis chunk has been selected");
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "contract",         // command name
            "contract region",  // command description
            {
                {"dir",
                 {"me", "back", "up", "down", "south", "north", "east",
                  "west"}},
            },
            {
                ParamData("dir", ParamType::Enum, true, "dir"),
                ParamData("num", ParamType::Int, false, "num"),
                ParamData("numback", ParamType::Int, true, "numback"),
            },
            {{"num", "dir", "numback"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerRegionMap.find(xuid) !=
                    mod.playerRegionMap.end()) {
                    int l = results["num"].get<int>();
                    worldedit::FACING facing;
                    if (results["dir"].isSet) {
                        auto str = results["dir"].getRaw<std::string>();
                        if (str == "me") {
                            facing = worldedit::dirToFacing(
                                player->getViewVector(1.0f));
                        } else if (str == "back") {
                            facing = worldedit::dirToFacing(
                                player->getViewVector(1.0f));
                            facing = worldedit::invFacing(facing);
                        } else {
                            facing = worldedit::dirStringToFacing(str);
                        }
                    } else {
                        facing =
                            worldedit::dirToFacing(player->getViewVector(1.0f));
                    }
                    std::vector<BlockPos> list;
                    list.resize(0);
                    list.push_back(worldedit::facingToPos(facing, l));
                    if (results["numback"].isSet) {
                        list.push_back(worldedit::facingToPos(
                            worldedit::invFacing(facing),
                            results["numback"].get<int>()));
                    }
                    auto res = mod.playerRegionMap[xuid]->contract(list);
                    if (res.second) {
                        output.success(res.first);
                    } else {
                        output.error(res.first);
                    }
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "expand",         // command name
            "expand region",  // command description
            {{"dir",
              {"me", "back", "up", "down", "south", "north", "east", "west"}},
             {"vert", {"vert"}}},
            {
                ParamData("dir", ParamType::Enum, true, "dir"),
                ParamData("num", ParamType::Int, false, "num"),
                ParamData("numback", ParamType::Int, true, "numback"),
                ParamData("vert", ParamType::Enum, "vert"),
            },
            {{"num", "dir", "numback"}, {"vert"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerRegionMap.find(xuid) !=
                    mod.playerRegionMap.end()) {
                    std::vector<BlockPos> list;
                    list.resize(0);
                    if (results["vert"].isSet) {
                        auto heightRange =
                            player->getDimensionConst().getHeightRange();
                        auto* region = mod.playerRegionMap[xuid];
                        auto boundingBox = region->getBoundBox();
                        list.push_back(BlockPos(
                            0,
                            std::min(boundingBox.bpos1.y - heightRange.min, 0),
                            0));
                        list.push_back(BlockPos(0, std::max(heightRange.max-boundingBox.bpos2.y-1,0), 0));
                    }else{
                        int l = results["num"].get<int>();
                        worldedit::FACING facing;
                        if (results["dir"].isSet) {
                            auto str = results["dir"].getRaw<std::string>();
                            if (str == "me") {
                                facing = worldedit::dirToFacing(
                                    player->getViewVector(1.0f));
                            } else if (str == "back") {
                                facing = worldedit::dirToFacing(
                                    player->getViewVector(1.0f));
                                facing = worldedit::invFacing(facing);
                            } else {
                                facing = worldedit::dirStringToFacing(str);
                            }
                        } else {
                            facing = worldedit::dirToFacing(
                                player->getViewVector(1.0f));
                        }
                        list.push_back(worldedit::facingToPos(facing, l));
                        if (results["numback"].isSet) {
                            list.push_back(worldedit::facingToPos(
                                worldedit::invFacing(facing),
                                results["numback"].get<int>()));
                        }
                    }
                        auto res = mod.playerRegionMap[xuid]->expand(list);
                        if (res.second) {
                            output.success(res.first);
                        } else {
                            output.error(res.first);
                        }
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "shift",         // command name
            "shift region",  // command description
            {
                {"dir",
                 {"me", "back", "up", "down", "south", "north", "east",
                  "west"}},
            },
            {ParamData("dir", ParamType::Enum, true, "dir"),
             ParamData("num", ParamType::Int, false, "num")},
            {{"num", "dir"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerRegionMap.find(xuid) !=
                    mod.playerRegionMap.end()) {
                    int l = results["num"].get<int>();
                    worldedit::FACING facing;
                    if (results["dir"].isSet) {
                        auto str = results["dir"].getRaw<std::string>();
                        if (str == "me") {
                            facing = worldedit::dirToFacing(
                                player->getViewVector(1.0f));
                        } else if (str == "back") {
                            facing = worldedit::dirToFacing(
                                player->getViewVector(1.0f));
                            facing = worldedit::invFacing(facing);
                        } else {
                            facing = worldedit::dirStringToFacing(str);
                        }
                    } else {
                        facing =
                            worldedit::dirToFacing(player->getViewVector(1.0f));
                    }
                    auto res = mod.playerRegionMap[xuid]->shift(
                        worldedit::facingToPos(facing, l));
                    if (res.second) {
                        output.success(res.first);
                    } else {
                        output.error(res.first);
                    }
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "outset",                   // command name
            "isotropic expand region",  // command description
            {},
            {ParamData("args", ParamType::String, true, "-hv"),
             ParamData("num", ParamType::Int, false, "num")},
            {{"num", "args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerRegionMap.find(xuid) !=
                    mod.playerRegionMap.end()) {
                    int l = results["num"].get<int>();
                    std::vector<BlockPos> list;
                    list.resize(0);
                    if (results["args"].isSet) {
                        auto str = results["args"].getRaw<std::string>();
                        if (str == "-h") {
                            list.push_back({0, 0, -l});
                            list.push_back({0, 0, l});
                            list.push_back({-l, 0, 0});
                            list.push_back({l, 0, 0});
                        } else if (str == "-v") {
                            list.push_back({0, -l, 0});
                            list.push_back({0, l, 0});
                        } else {
                            output.error("wrong args");
                            return;
                        }
                    } else {
                        list.push_back({-l, 0, 0});
                        list.push_back({l, 0, 0});
                        list.push_back({0, -l, 0});
                        list.push_back({0, l, 0});
                        list.push_back({0, 0, -l});
                        list.push_back({0, 0, l});
                    }
                    auto res = mod.playerRegionMap[xuid]->expand(list);
                    if (res.second) {
                        output.success(res.first);
                    } else {
                        output.error(res.first);
                    }
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "inset",                      // command name
            "isotropic contract region",  // command description
            {},
            {ParamData("args", ParamType::String, true, "-hv"),
             ParamData("num", ParamType::Int, false, "num")},
            {{"num", "args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerRegionMap.find(xuid) !=
                    mod.playerRegionMap.end()) {
                    int l = results["num"].get<int>();
                    std::vector<BlockPos> list;
                    list.resize(0);
                    if (results["args"].isSet) {
                        auto str = results["args"].getRaw<std::string>();
                        if (str == "-h") {
                            list.push_back({0, 0, -l});
                            list.push_back({0, 0, l});
                            list.push_back({-l, 0, 0});
                            list.push_back({l, 0, 0});
                        } else if (str == "-v") {
                            list.push_back({0, -l, 0});
                            list.push_back({0, l, 0});
                        } else {
                            output.error("wrong args");
                            return;
                        }
                    } else {
                        list.push_back({-l, 0, 0});
                        list.push_back({l, 0, 0});
                        list.push_back({0, -l, 0});
                        list.push_back({0, l, 0});
                        list.push_back({0, 0, -l});
                        list.push_back({0, 0, l});
                    }
                    auto res = mod.playerRegionMap[xuid]->contract(list);
                    if (res.second) {
                        output.success(res.first);
                    } else {
                        output.error(res.first);
                    }
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit

#endif  // WORLDEDIT_REGIONCOMMAND_H