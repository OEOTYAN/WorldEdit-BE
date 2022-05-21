//
// Created by OEOTYAN on 2022/05/18.
//
#pragma once
#ifndef WORLDEDIT_REGIONINFOCOMMAND_H
#define WORLDEDIT_REGIONINFOCOMMAND_H
#include "pch.h"
#include <EventAPI.h>
#include <LoggerAPI.h>
#include <MC/Level.hpp>
#include <MC/BlockInstance.hpp>
#include <MC/BedrockBlocks.hpp>
#include <MC/Block.hpp>
#include <MC/BlockSource.hpp>
#include <MC/Actor.hpp>
#include <MC/Player.hpp>
#include <MC/ServerPlayer.hpp>
#include <MC/Dimension.hpp>
#include <MC/ItemStack.hpp>
#include "Version.h"
#include "eval/Eval.h"
#include "string/StringTool.h"
#include <LLAPI.h>
#include <ServerAPI.h>
#include <EventAPI.h>
#include <ScheduleAPI.h>
#include <DynamicCommandAPI.h>
#include "particle/Graphics.h"
#include "WorldEdit.h"
namespace worldedit {
    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;

    // size count distr

    void regionInfoCommandSetup() {
        DynamicCommand::setup(
            "size",                   // command name
            "calculate region size",  // command description
            {}, {ParamData("args", ParamType::String, true, "-ca")}, {{"args"}},
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
                    long long size = 0;
                    bool arg_a = false, arg_c = false;
                    if (results["args"].isSet) {
                        auto str = results["args"].getRaw<std::string>();
                        if (str.find("-") == std::string::npos) {
                            output.error("wrong args");
                            return;
                        }
                        if (str.find("a") != std::string::npos) {
                            arg_a = true;
                        }
                        if (str.find("c") != std::string::npos) {
                            arg_c = true;
                        }
                    }
                    if (!arg_c) {
                        if (arg_a) {
                            mod.playerRegionMap[xuid]->forEachBlockInRegion(
                                [&](const BlockPos& pos) { size++; });
                        } else {
                            auto dimID =
                                mod.playerRegionMap[xuid]->getDimensionID();
                            mod.playerRegionMap[xuid]->forEachBlockInRegion(
                                [&](const BlockPos& pos) {
                                    auto block = Level::getBlock(pos, dimID);
                                    if (!(block == BedrockBlocks::mAir))
                                        size++;
                                });
                        }
                    } else {
                        // clipboard
                    }
                    output.success(fmt::format("§6size: {}", size));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "count",        // command name
            "count block",  // command description
            {
                {"-c", {"-c"}},
            },
            {ParamData("args", ParamType::Enum, true, "-c"),
             ParamData("block", ParamType::Block, "block"),
             ParamData("data", ParamType::Int, true, "data")},
            {{"block", "data", "args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                int data = -1;
                auto blockname =
                    results["block"].get<Block const*>()->getTypeName();
                if (results["data"].isSet) {
                    data = results["data"].getRaw<int>();
                }
                if (mod.playerRegionMap.find(xuid) !=
                    mod.playerRegionMap.end()) {
                    long long count = 0;
                    if (results["args"].isSet) {
                        // clipboard
                    } else {
                        auto dimID =
                            mod.playerRegionMap[xuid]->getDimensionID();
                        mod.playerRegionMap[xuid]->forEachBlockInRegion(
                            [&](const BlockPos& pos) {
                                auto block = Level::getBlock(pos, dimID);
                                if (block->getTypeName() == blockname) {
                                    if (data == -1 ||
                                        data == (int)(block->getTileData()))
                                        count++;
                                }
                            });
                    }
                    output.success(fmt::format("§6count: {}", count));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "distr",                         // command name
            "block distribution in region",  // command description
            {}, {ParamData("args", ParamType::String, true, "-cd")}, {{"args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                bool arg_d = false, arg_c = false;
                if (results["args"].isSet) {
                    auto str = results["args"].getRaw<std::string>();
                    if (str.find("-") == std::string::npos) {
                        output.error("wrong args");
                        return;
                    }
                    if (str.find("d") != std::string::npos) {
                        arg_d = true;
                    }
                    if (str.find("c") != std::string::npos) {
                        arg_c = true;
                    }
                }

                if (mod.playerRegionMap.find(xuid) !=
                    mod.playerRegionMap.end()) {
                    std::map<std::string, long long> blocksMap;
                    std::vector<std::pair<std::string, long long>> blocksMap2;
                    blocksMap2.resize(0);
                    long long all = 0;
                    if (arg_c) {
                        // clipboard
                    } else {
                        auto dimID =
                            mod.playerRegionMap[xuid]->getDimensionID();
                        if (arg_d) {
                            mod.playerRegionMap[xuid]->forEachBlockInRegion(
                                [&](const BlockPos& pos) {
                                    all++;
                                    auto block = Level::getBlock(pos, dimID);
                                    std::string blockName =
                                        block->getTypeName();
                                    blockName +=
                                        " " + worldedit::fto_string(
                                                  block->getTileData(), 0);
                                    if (blocksMap.find(blockName) !=
                                        blocksMap.end()) {
                                        blocksMap[blockName] += 1;
                                    } else {
                                        blocksMap[blockName] = 1;
                                    }
                                });
                        } else {
                            mod.playerRegionMap[xuid]->forEachBlockInRegion(
                                [&](const BlockPos& pos) {
                                    all++;
                                    auto block = Level::getBlock(pos, dimID);
                                    std::string blockName =
                                        block->getTypeName();
                                    if (blocksMap.find(blockName) !=
                                        blocksMap.end()) {
                                        blocksMap[blockName] += 1;
                                    } else {
                                        blocksMap[blockName] = 1;
                                    }
                                });
                        }
                    }
                    for (auto& block : blocksMap) {
                        std::string name = block.first;
                        if (name.substr(0, 10) == "minecraft:") {
                            name = name.substr(10);
                        }
                        blocksMap2.emplace_back(
                            std::pair<std::string, long long>(name,
                                                              block.second));
                    }
                    std::sort(blocksMap2.begin(), blocksMap2.end(),
                              [](const std::pair<std::string, long long>& a,
                                 const std::pair<std::string, long long>& b) {
                                  return a.second > b.second;
                              });
                    std::string res(fmt::format("§6total: §b{}", all));
                    for (auto& block : blocksMap2) {
                        res += fmt::format(
                            "\n§b{}    §6({}%%) §f{}", block.second,
                            worldedit::fto_string(
                                static_cast<double>(block.second) /
                                    static_cast<double>(all) * 100.0,
                                3),
                            block.first);
                    }
                    output.success(res);
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit

#endif  // WORLDEDIT_REGIONINFOCOMMAND_H