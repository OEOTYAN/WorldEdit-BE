//
// Created by OEOTYAN on 2022/05/27.
//
#pragma once
#ifndef WORLDEDIT_REGIONOPERATIONCOMMAND_H
#define WORLDEDIT_REGIONOPERATIONCOMMAND_H

#include "store/BlockPattern.hpp"
#include "eval/blur.hpp"
#include "image/Image.h"

namespace worldedit {

    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;

    // set rep center stack move gen walls faces overlay naturalize hollow
    // smooth | deform

    void regionOperationCommandSetup() {
        DynamicCommand::setup(
            "set",        // command name
            "set block",  // command description
            {},
            {ParamData("block", ParamType::Block, "block"),
             ParamData("blockPattern", ParamType::String, "blockPattern")},
            {{"block"}, {"blockPattern"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod  = worldedit::getMod();
                auto  xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region      = mod.playerRegionMap[xuid];
                    auto  center      = region->getCenter();
                    auto  dimID       = region->getDimensionID();
                    auto  boundingBox = region->getBoundBox();
                    auto  blockSource = Level::getBlockSource(dimID);

                    auto history            = mod.getPlayerNextHistory(xuid);
                    *history                = Clipboard(boundingBox.max - boundingBox.min);
                    history->playerRelPos.x = dimID;
                    history->playerPos      = boundingBox.min;
                    // history->used = true;
                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        auto localPos      = pos - boundingBox.min;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history->storeBlock(blockInstance, localPos);
                    });

                    long long i = 0;

                    std::string gMask = "";
                    if (mod.playerGMaskMap.find(xuid) != mod.playerGMaskMap.end()) {
                        gMask = mod.playerGMaskMap[xuid];
                    }

                    auto          playerPos = origin.getPlayer()->getPosition();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;

                    std::string bps = "minecraft:air";
                    if (results["blockPattern"].isSet) {
                        bps = results["blockPattern"].get<std::string>();
                    } else if (results["block"].isSet) {
                        bps = results["block"].get<Block const*>()->getTypeName();
                    }
                    BlockPattern blockPattern(bps, xuid, region);

                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        setFunction(variables, f, boundingBox, playerPos, pos, center);
                        if (gMask == "" || cpp_eval::eval<double>(gMask.c_str(), variables, f) > 0.5) {
                            blockPattern.setBlock(variables, f, blockSource, pos);
                            i++;
                        }
                    });

                    output.success(fmt::format("§a{} block(s) placed", i));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "rep",            // command name
            "replace block",  // command description
            {},
            {ParamData("blockAfter", ParamType::Block, "blockAfter"),
             ParamData("blockBefore", ParamType::Block, true, "blockBefore"),
             ParamData("blockAfterS", ParamType::String, "blockAfterS"),
             ParamData("blockBeforeS", ParamType::String, true, "blockBeforeS")},
            {{"blockBefore", "blockAfter"},
             {"blockBeforeS", "blockAfterS"},
             {"blockBeforeS", "blockAfter"},
             {"blockBefore", "blockAfterS"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod  = worldedit::getMod();
                auto  xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region      = mod.playerRegionMap[xuid];
                    auto  center      = region->getCenter();
                    auto  dimID       = region->getDimensionID();
                    auto  boundingBox = region->getBoundBox();
                    auto  blockSource = Level::getBlockSource(dimID);

                    auto history            = mod.getPlayerNextHistory(xuid);
                    *history                = Clipboard(boundingBox.max - boundingBox.min);
                    history->playerRelPos.x = dimID;
                    history->playerPos      = boundingBox.min;
                    // history->used = true;
                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        auto localPos      = pos - boundingBox.min;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history->storeBlock(blockInstance, localPos);
                    });

                    long long i = 0;

                    std::string gMask = "";
                    if (mod.playerGMaskMap.find(xuid) != mod.playerGMaskMap.end()) {
                        gMask = mod.playerGMaskMap[xuid];
                    }

                    auto          playerPos = origin.getPlayer()->getPosition();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;

                    std::string bps = "minecraft:air";
                    if (results["blockAfterS"].isSet) {
                        bps = results["blockAfterS"].get<std::string>();
                    } else if (results["blockAfter"].isSet) {
                        bps = results["blockAfter"].get<Block const*>()->getTypeName();
                    }
                    std::string bps2 = "minecraft:air";
                    if (results["blockBeforeS"].isSet) {
                        bps2 = results["blockBeforeS"].get<std::string>();
                    } else if (results["blockBefore"].isSet) {
                        bps2 = results["blockBefore"].get<Block const*>()->getTypeName();
                    }
                    BlockPattern blockPattern(bps, xuid, region);
                    BlockPattern blockFilter(bps2);
                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        setFunction(variables, f, boundingBox, playerPos, pos, center);
                        if ((gMask == "" || cpp_eval::eval<double>(gMask.c_str(), variables, f) > 0.5) &&
                            blockFilter.hasBlock(const_cast<Block*>(&blockSource->getBlock(pos)))) {
                            blockPattern.setBlock(variables, f, blockSource, pos);
                            i++;
                        }
                    });

                    output.success(fmt::format("§a{} block(s) replaced", i));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "center",                         // command name
            "set center block(s) of region",  // command description
            {},
            {ParamData("block", ParamType::Block, "block"),
             ParamData("blockPattern", ParamType::String, "blockPattern")},
            {{"block"}, {"blockPattern"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod  = worldedit::getMod();
                auto  xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto*       region = mod.playerRegionMap[xuid];
                    auto        dimID  = region->getDimensionID();
                    auto        center = region->getCenter();
                    BoundingBox boundingBox;
                    boundingBox.min.x = static_cast<int>(floor(center.x - 0.49));
                    boundingBox.min.y = static_cast<int>(floor(center.y - 0.49));
                    boundingBox.min.z = static_cast<int>(floor(center.z - 0.49));
                    boundingBox.max.x = static_cast<int>(floor(center.x + 0.49));
                    boundingBox.max.y = static_cast<int>(floor(center.y + 0.49));
                    boundingBox.max.z = static_cast<int>(floor(center.z + 0.49));
                    auto blockSource  = Level::getBlockSource(dimID);

                    auto history            = mod.getPlayerNextHistory(xuid);
                    *history                = Clipboard(boundingBox.max - boundingBox.min);
                    history->playerRelPos.x = dimID;
                    history->playerPos      = boundingBox.min;
                    // history->used = true;
                    boundingBox.forEachBlockInBox([&](const BlockPos& pos) {
                        auto localPos      = pos - boundingBox.min;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history->storeBlock(blockInstance, localPos);
                    });

                    auto          playerPos = origin.getPlayer()->getPosition();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;

                    std::string bps = "minecraft:air";
                    if (results["blockPattern"].isSet) {
                        bps = results["blockPattern"].get<std::string>();
                    } else if (results["block"].isSet) {
                        bps = results["block"].get<Block const*>()->getTypeName();
                    }
                    BlockPattern blockPattern(bps, xuid, region);
                    boundingBox.forEachBlockInBox([&](const BlockPos& pos) {
                        setFunction(variables, f, boundingBox, playerPos, pos, center);
                        blockPattern.setBlock(variables, f, blockSource, pos);
                    });

                    output.success(fmt::format("§acenter placed"));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "stack",         // command name
            "stack region",  // command description
            {
                {"dir", {"me", "back", "up", "down", "south", "north", "east", "west"}},
            },
            {ParamData("times", ParamType::Int, "times"), ParamData("dir", ParamType::Enum, true, "dir"),
             ParamData("args", ParamType::String, true, "-sa")},
            {{"times", "dir", "args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod    = worldedit::getMod();
                auto  player = origin.getPlayer();
                auto  xuid   = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto times = results["times"].get<int>();
                    bool arg_a = false, arg_s = false;
                    if (results["args"].isSet) {
                        auto str = results["args"].getRaw<std::string>();
                        if (str.find("-") == std::string::npos) {
                            output.error("wrong args");
                            return;
                        }
                        if (str.find("a") != std::string::npos) {
                            arg_a = true;
                        }
                        if (str.find("s") != std::string::npos) {
                            arg_s = true;
                        }
                    }
                    worldedit::FACING facing;
                    if (results["dir"].isSet) {
                        auto str = results["dir"].getRaw<std::string>();
                        if (str == "me") {
                            facing = worldedit::dirToFacing(player->getViewVector(1.0f));
                        } else if (str == "back") {
                            facing = worldedit::dirToFacing(player->getViewVector(1.0f));
                            facing = worldedit::invFacing(facing);
                        } else {
                            facing = worldedit::dirStringToFacing(str);
                        }
                    } else {
                        facing = worldedit::dirToFacing(player->getViewVector(1.0f));
                    }
                    BlockPos faceVec = worldedit::facingToPos(facing, 1);

                    auto* region          = mod.playerRegionMap[xuid];
                    auto  center          = region->getCenter();
                    auto  dimID           = region->getDimensionID();
                    auto  boundingBox     = region->getBoundBox();
                    auto  boundingBoxLast = boundingBox;
                    auto  blockSource     = Level::getBlockSource(dimID);

                    auto movingVec = faceVec * (boundingBox.max - boundingBox.min + 1);

                    boundingBoxLast.min     = boundingBoxLast.min + movingVec * times;
                    boundingBoxLast.max     = boundingBoxLast.max + movingVec * times;
                    auto boundingBoxHistory = BoundingBox::mergeBoundingBox(boundingBoxLast, boundingBox);

                    auto history            = mod.getPlayerNextHistory(xuid);
                    *history                = Clipboard(boundingBoxHistory.max - boundingBoxHistory.min);
                    history->playerRelPos.x = dimID;
                    history->playerPos      = boundingBoxHistory.min;
                    // history->used = true;
                    boundingBoxHistory.forEachBlockInBox([&](const BlockPos& pos) {
                        auto localPos      = pos - boundingBoxHistory.min;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history->storeBlock(blockInstance, localPos);
                    });

                    long long i = 0;

                    std::string gMask = "";
                    if (mod.playerGMaskMap.find(xuid) != mod.playerGMaskMap.end()) {
                        gMask = mod.playerGMaskMap[xuid];
                    }

                    auto          playerPos = origin.getPlayer()->getPosition();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;
                    region->forEachBlockInRegion([&](const BlockPos& posk) {
                        for (int j = 1; j <= times; j++) {
                            auto pos = posk + movingVec * j;
                            setFunction(variables, f, boundingBox, playerPos, pos, center);
                            if ((arg_a && (&blockSource->getBlock(posk) != BedrockBlocks::mAir)) || !arg_a)
                                if (gMask == "" || cpp_eval::eval<double>(gMask.c_str(), variables, f) > 0.5) {
                                    setBlockSimple(blockSource, pos, const_cast<Block*>(&blockSource->getBlock(posk)));
                                    i++;
                                }
                        }
                    });
                    if (arg_s) {
                        mod.playerRegionMap[xuid]->shift(movingVec * times);
                    }
                    output.success(fmt::format("§a{} block(s) stacked", i));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "move",         // command name
            "move region",  // command description
            {
                {"dir", {"me", "back", "up", "down", "south", "north", "east", "west"}},
            },
            {ParamData("dis", ParamType::Int, "dis"), ParamData("dir", ParamType::Enum, true, "dir"),
             ParamData("args", ParamType::String, true, "-sa"), ParamData("block", ParamType::Block, true, "block"),
             ParamData("blockPattern", ParamType::String, true, "blockPattern")},
            {{"dis", "dir", "block", "args"}, {"dis", "dir", "blockPattern", "args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod    = worldedit::getMod();
                auto  player = origin.getPlayer();
                auto  xuid   = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto dis   = results["dis"].get<int>();
                    bool arg_a = false, arg_s = false;
                    if (results["args"].isSet) {
                        auto str = results["args"].getRaw<std::string>();
                        if (str.find("-") == std::string::npos) {
                            output.error("wrong args");
                            return;
                        }
                        if (str.find("a") != std::string::npos) {
                            arg_a = true;
                        }
                        if (str.find("s") != std::string::npos) {
                            arg_s = true;
                        }
                    }
                    worldedit::FACING facing;
                    if (results["dir"].isSet) {
                        auto str = results["dir"].getRaw<std::string>();
                        if (str == "me") {
                            facing = worldedit::dirToFacing(player->getViewVector(1.0f));
                        } else if (str == "back") {
                            facing = worldedit::dirToFacing(player->getViewVector(1.0f));
                            facing = worldedit::invFacing(facing);
                        } else {
                            facing = worldedit::dirStringToFacing(str);
                        }
                    } else {
                        facing = worldedit::dirToFacing(player->getViewVector(1.0f));
                    }
                    BlockPos faceVec = worldedit::facingToPos(facing, dis);

                    auto* region          = mod.playerRegionMap[xuid];
                    auto  center          = region->getCenter();
                    auto  dimID           = region->getDimensionID();
                    auto  boundingBox     = region->getBoundBox();
                    auto  boundingBoxLast = boundingBox;
                    auto  blockSource     = Level::getBlockSource(dimID);

                    boundingBoxLast.min     = boundingBoxLast.min + faceVec;
                    boundingBoxLast.max     = boundingBoxLast.max + faceVec;
                    auto boundingBoxHistory = BoundingBox::mergeBoundingBox(boundingBoxLast, boundingBox);

                    auto history            = mod.getPlayerNextHistory(xuid);
                    *history                = Clipboard(boundingBoxHistory.max - boundingBoxHistory.min);
                    history->playerRelPos.x = dimID;
                    history->playerPos      = boundingBoxHistory.min;
                    // history->used = true;
                    boundingBox.forEachBlockInBox([&](const BlockPos& pos) {
                        auto localPos      = pos - boundingBoxHistory.min;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history->storeBlock(blockInstance, localPos);
                    });
                    boundingBoxLast.forEachBlockInBox([&](const BlockPos& pos) {
                        auto localPos = pos - boundingBoxHistory.min;
                        if (!history->getSet(localPos).hasBlock) {
                            auto blockInstance = blockSource->getBlockInstance(pos);
                            history->storeBlock(blockInstance, localPos);
                        }
                    });

                    long long i = 0;

                    std::string gMask = "";
                    if (mod.playerGMaskMap.find(xuid) != mod.playerGMaskMap.end()) {
                        gMask = mod.playerGMaskMap[xuid];
                    }

                    auto          playerPos = origin.getPlayer()->getPosition();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;

                    std::string bps = "minecraft:air";
                    if (results["blockPattern"].isSet) {
                        bps = results["blockPattern"].get<std::string>();
                    } else if (results["block"].isSet) {
                        bps = results["block"].get<Block const*>()->getTypeName();
                    }
                    BlockPattern blockPattern(bps, xuid, region);
                    region->forEachBlockInRegion([&](const BlockPos& posk) {
                        auto pos = posk + faceVec;
                        setFunction(variables, f, boundingBox, playerPos, pos, center);
                        if (gMask == "" || cpp_eval::eval<double>(gMask.c_str(), variables, f) > 0.5) {
                            blockPattern.setBlock(variables, f, blockSource, posk);
                            i++;
                        }
                    });

                    region->forEachBlockInRegion([&](const BlockPos& posk) {
                        auto pos      = posk + faceVec;
                        auto localPos = posk - boundingBoxHistory.min;
                        auto block    = history->getSet(localPos).getBlock();
                        setFunction(variables, f, boundingBox, playerPos, pos, center);
                        if ((arg_a && (block != BedrockBlocks::mAir)) || !arg_a)
                            if (gMask == "" || cpp_eval::eval<double>(gMask.c_str(), variables, f) > 0.5) {
                                setBlockSimple(blockSource, pos, block);
                            }
                    });
                    if (arg_s) {
                        mod.playerRegionMap[xuid]->shift(faceVec);
                    }
                    output.success(fmt::format("§a{} block(s) moved", i));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "gen",                     // command name
            "generate with function",  // command description
            {},
            {ParamData("block", ParamType::Block, "block"),
             ParamData("blockPattern", ParamType::String, "blockPattern"),
             ParamData("function", ParamType::String, "function"), ParamData("args", ParamType::String, true, "-h")},
            {{"block", "function", "args"}, {"blockPattern", "function", "args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod  = worldedit::getMod();
                auto  xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    bool arg_h = false;
                    if (results["args"].isSet) {
                        auto str = results["args"].getRaw<std::string>();
                        if (str.find("-") == std::string::npos) {
                            output.error("wrong args");
                            return;
                        }
                        if (str.find("h") != std::string::npos) {
                            arg_h = true;
                        }
                    }

                    auto* region      = mod.playerRegionMap[xuid];
                    auto  center      = region->getCenter();
                    auto  dimID       = region->getDimensionID();
                    auto  boundingBox = region->getBoundBox();
                    auto  blockSource = Level::getBlockSource(dimID);

                    auto history            = mod.getPlayerNextHistory(xuid);
                    *history                = Clipboard(boundingBox.max - boundingBox.min);
                    history->playerRelPos.x = dimID;
                    history->playerPos      = boundingBox.min;
                    // history->used = true;
                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        auto localPos      = pos - boundingBox.min;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history->storeBlock(blockInstance, localPos);
                    });

                    long long i = 0;

                    std::string gMask = "";
                    if (mod.playerGMaskMap.find(xuid) != mod.playerGMaskMap.end()) {
                        gMask = mod.playerGMaskMap[xuid];
                    }

                    auto genfunc = results["function"].get<std::string>();

                    auto          playerPos = origin.getPlayer()->getPosition();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;

                    auto sizeDim = boundingBox.max - boundingBox.min + 3;

                    long long size = (sizeDim.x) * (sizeDim.y) * (sizeDim.z);

                    std::vector<bool> tmp(size, false);
                    std::vector<bool> caled(size, false);

                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        setFunction(variables, f, boundingBox, playerPos, pos, center);
                        if (gMask == "" || cpp_eval::eval<double>(gMask.c_str(), variables, f) > 0.5) {
                            auto localPos = pos - boundingBox.min + 1;
                            if (cpp_eval::eval<double>(genfunc.c_str(), variables, f) > 0.5) {
                                tmp[(localPos.y + sizeDim.y * localPos.z) * sizeDim.x + localPos.x] = true;
                            }
                            caled[(localPos.y + sizeDim.y * localPos.z) * sizeDim.x + localPos.x] = true;
                        }
                    });

                    if (arg_h) {
                        std::vector<bool> tmp2(size, false);
                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            auto localPos = pos - boundingBox.min + 1;
                            if (tmp[(localPos.y + sizeDim.y * localPos.z) * sizeDim.x + localPos.x]) {
                                int counts = 0;

                                for (auto& calPos : localPos.getNeighbors()) {
                                    if (!caled[(calPos.y + sizeDim.y * calPos.z) * sizeDim.x + calPos.x]) {
                                        setFunction(variables, f, boundingBox, playerPos, calPos + boundingBox.min - 1,
                                                    center);
                                        tmp[(calPos.y + sizeDim.y * calPos.z) * sizeDim.x + calPos.x] =
                                            cpp_eval::eval<double>(genfunc.c_str(), variables, f) > 0.5;

                                        caled[(calPos.y + sizeDim.y * calPos.z) * sizeDim.x + calPos.x] = true;
                                    }
                                    if (tmp[(calPos.y + sizeDim.y * calPos.z) * sizeDim.x + calPos.x]) {
                                        counts++;
                                    }
                                }
                                if (counts < 6)
                                    tmp2[(localPos.y + sizeDim.y * localPos.z) * sizeDim.x + localPos.x] = true;
                            }
                        });
                        tmp.assign(tmp2.begin(), tmp2.end());
                    }

                    std::string bps = "minecraft:air";
                    if (results["blockPattern"].isSet) {
                        bps = results["blockPattern"].get<std::string>();
                    } else if (results["block"].isSet) {
                        bps = results["block"].get<Block const*>()->getTypeName();
                    }
                    BlockPattern blockPattern(bps, xuid, region);
                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        auto localPos = pos - boundingBox.min + 1;
                        if (tmp[(localPos.y + sizeDim.y * localPos.z) * sizeDim.x + localPos.x]) {
                            setFunction(variables, f, boundingBox, playerPos, pos, center);
                            blockPattern.setBlock(variables, f, blockSource, pos);
                            i++;
                        }
                    });

                    output.success(fmt::format("§gfunction: {}\n§a{} block(s) placed", genfunc, i));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "walls",                // command name
            "set walls of region",  // command description
            {},
            {ParamData("block", ParamType::Block, "block"),
             ParamData("blockPattern", ParamType::String, "blockPattern")},
            {{"block"}, {"blockPattern"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod  = worldedit::getMod();
                auto  xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region      = mod.playerRegionMap[xuid];
                    auto  center      = region->getCenter();
                    auto  dimID       = region->getDimensionID();
                    auto  boundingBox = region->getBoundBox();
                    auto  blockSource = Level::getBlockSource(dimID);

                    auto history            = mod.getPlayerNextHistory(xuid);
                    *history                = Clipboard(boundingBox.max - boundingBox.min);
                    history->playerRelPos.x = dimID;
                    history->playerPos      = boundingBox.min;
                    // history->used = true;
                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        auto localPos      = pos - boundingBox.min;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history->storeBlock(blockInstance, localPos);
                    });

                    long long i = 0;

                    std::string gMask = "";
                    if (mod.playerGMaskMap.find(xuid) != mod.playerGMaskMap.end()) {
                        gMask = mod.playerGMaskMap[xuid];
                    }

                    auto          playerPos = origin.getPlayer()->getPosition();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;

                    std::string bps = "minecraft:air";
                    if (results["blockPattern"].isSet) {
                        bps = results["blockPattern"].get<std::string>();
                    } else if (results["block"].isSet) {
                        bps = results["block"].get<Block const*>()->getTypeName();
                    }
                    BlockPattern blockPattern(bps, xuid, region);
                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        setFunction(variables, f, boundingBox, playerPos, pos, center);
                        if (gMask == "" || cpp_eval::eval<double>(gMask.c_str(), variables, f) > 0.5) {
                            int counts = 0;
                            for (auto& calPos : pos.getNeighbors()) {
                                if (calPos.y <= boundingBox.max.y && calPos.y >= boundingBox.min.y) {
                                    counts += region->contains(calPos);
                                } else {
                                    counts += 1;
                                }
                            }
                            if (counts < 6) {
                                blockPattern.setBlock(variables, f, blockSource, pos);
                                i++;
                            }
                        }
                    });

                    output.success(fmt::format("§a{} block(s) placed", i));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "faces",                // command name
            "set faces of region",  // command description
            {},
            {ParamData("block", ParamType::Block, "block"),
             ParamData("blockPattern", ParamType::String, "blockPattern")},
            {{"block"}, {"blockPattern"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod  = worldedit::getMod();
                auto  xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region      = mod.playerRegionMap[xuid];
                    auto  center      = region->getCenter();
                    auto  dimID       = region->getDimensionID();
                    auto  boundingBox = region->getBoundBox();
                    auto  blockSource = Level::getBlockSource(dimID);

                    auto history            = mod.getPlayerNextHistory(xuid);
                    *history                = Clipboard(boundingBox.max - boundingBox.min);
                    history->playerRelPos.x = dimID;
                    history->playerPos      = boundingBox.min;
                    // history->used = true;
                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        auto localPos      = pos - boundingBox.min;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history->storeBlock(blockInstance, localPos);
                    });

                    long long i = 0;

                    std::string gMask = "";
                    if (mod.playerGMaskMap.find(xuid) != mod.playerGMaskMap.end()) {
                        gMask = mod.playerGMaskMap[xuid];
                    }

                    auto          playerPos = origin.getPlayer()->getPosition();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;
                    std::string                             bps = "minecraft:air";
                    if (results["blockPattern"].isSet) {
                        bps = results["blockPattern"].get<std::string>();
                    } else if (results["block"].isSet) {
                        bps = results["block"].get<Block const*>()->getTypeName();
                    }
                    BlockPattern blockPattern(bps, xuid, region);
                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        setFunction(variables, f, boundingBox, playerPos, pos, center);
                        if (gMask == "" || cpp_eval::eval<double>(gMask.c_str(), variables, f) > 0.5) {
                            int counts = 0;
                            for (auto& calPos : pos.getNeighbors()) {
                                counts += region->contains(calPos);
                            }
                            if (counts < 6) {
                                blockPattern.setBlock(variables, f, blockSource, pos);
                                i++;
                            }
                        }
                    });

                    output.success(fmt::format("§a{} block(s) placed", i));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "overlay",        // command name
            "overlay block",  // command description
            {},
            {ParamData("block", ParamType::Block, "block"),
             ParamData("blockPattern", ParamType::String, "blockPattern")},
            {{"block"}, {"blockPattern"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod  = worldedit::getMod();
                auto  xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region      = mod.playerRegionMap[xuid];
                    auto  center      = region->getCenter();
                    auto  dimID       = region->getDimensionID();
                    auto  boundingBox = region->getBoundBox();
                    auto  blockSource = Level::getBlockSource(dimID);

                    auto history            = mod.getPlayerNextHistory(xuid);
                    *history                = Clipboard(boundingBox.max - boundingBox.min);
                    history->playerRelPos.x = dimID;
                    history->playerPos      = boundingBox.min;
                    // history->used = true;
                    region->forTopBlockInRegion([&](const BlockPos& pos) {
                        auto localPos      = pos - boundingBox.min;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history->storeBlock(blockInstance, localPos);
                    });

                    long long i = 0;

                    std::string gMask = "";
                    if (mod.playerGMaskMap.find(xuid) != mod.playerGMaskMap.end()) {
                        gMask = mod.playerGMaskMap[xuid];
                    }

                    auto          playerPos = origin.getPlayer()->getPosition();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;
                    std::string                             bps = "minecraft:air";
                    if (results["blockPattern"].isSet) {
                        bps = results["blockPattern"].get<std::string>();
                    } else if (results["block"].isSet) {
                        bps = results["block"].get<Block const*>()->getTypeName();
                    }
                    BlockPattern blockPattern(bps, xuid, region);
                    region->forTopBlockInRegion([&](const BlockPos& pos) {
                        setFunction(variables, f, boundingBox, playerPos, pos, center);
                        if (gMask == "" || cpp_eval::eval<double>(gMask.c_str(), variables, f) > 0.5) {
                            blockPattern.setBlock(variables, f, blockSource, pos);
                            i++;
                        }
                    });
                    output.success(fmt::format("§a{} block(s) placed", i));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "naturalize",         // command name
            "naturalize region",  // command description
            {}, {}, {{}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod  = worldedit::getMod();
                auto  xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region      = mod.playerRegionMap[xuid];
                    auto  center      = region->getCenter();
                    auto  dimID       = region->getDimensionID();
                    auto  boundingBox = region->getBoundBox();
                    auto  blockSource = Level::getBlockSource(dimID);

                    auto history            = mod.getPlayerNextHistory(xuid);
                    *history                = Clipboard(boundingBox.max - boundingBox.min);
                    history->playerRelPos.x = dimID;
                    history->playerPos      = boundingBox.min;
                    // history->used = true;
                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        auto localPos      = pos - boundingBox.min;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history->storeBlock(blockInstance, localPos);
                    });

                    std::string gMask = "";
                    if (mod.playerGMaskMap.find(xuid) != mod.playerGMaskMap.end()) {
                        gMask = mod.playerGMaskMap[xuid];
                    }

                    auto          playerPos = origin.getPlayer()->getPosition();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;

                    region->forTopBlockInRegion([&](const BlockPos& posk) {
                        BlockPos pos(posk.x, posk.y - 1, posk.z);
                        if (&blockSource->getBlock(pos) == VanillaBlocks::mStone && region->contains(pos)) {
                            setFunction(variables, f, boundingBox, playerPos, pos, center);
                            if (gMask == "" || cpp_eval::eval<double>(gMask.c_str(), variables, f) > 0.5) {
                                setBlockSimple(blockSource, pos, VanillaBlocks::mGrass);
                            }
                        }
                        for (int mY = -2; mY >= -4; mY--) {
                            BlockPos pos(posk.x, posk.y + mY, posk.z);
                            if (&blockSource->getBlock(pos) == VanillaBlocks::mStone && region->contains(pos)) {
                                setFunction(variables, f, boundingBox, playerPos, pos, center);
                                if (gMask == "" || cpp_eval::eval<double>(gMask.c_str(), variables, f) > 0.5) {
                                    setBlockSimple(blockSource, pos, const_cast<Block*>(VanillaBlocks::mDirt));
                                }
                            }
                        }
                    });
                    output.success(fmt::format("§aRegion naturalized"));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "hollow",              // command name
            "hollows out region",  // command description
            {},
            {ParamData("num", ParamType::Int, true, "num"), ParamData("block", ParamType::Block, true, "block"),
             ParamData("blockPattern", ParamType::String, true, "blockPattern")},
            {{"num", "block"}, {"num", "blockPattern"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod  = worldedit::getMod();
                auto  xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    int layer = 0;
                    if (results["num"].isSet) {
                        layer = results["num"].get<int>();
                    }

                    auto* region      = mod.playerRegionMap[xuid];
                    auto  center      = region->getCenter();
                    auto  dimID       = region->getDimensionID();
                    auto  boundingBox = region->getBoundBox();
                    auto  blockSource = Level::getBlockSource(dimID);

                    auto history            = mod.getPlayerNextHistory(xuid);
                    *history                = Clipboard(boundingBox.max - boundingBox.min);
                    history->playerRelPos.x = dimID;
                    history->playerPos      = boundingBox.min;
                    // history->used = true;
                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        auto localPos      = pos - boundingBox.min;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history->storeBlock(blockInstance, localPos);
                    });

                    std::string gMask = "";
                    if (mod.playerGMaskMap.find(xuid) != mod.playerGMaskMap.end()) {
                        gMask = mod.playerGMaskMap[xuid];
                    }

                    auto sizeDim = boundingBox.max - boundingBox.min + 3;

                    long long size = (sizeDim.x) * (sizeDim.y) * (sizeDim.z);

                    std::vector<bool> tmp(size, false);

                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        int counts = 0;

                        for (auto& calPos : pos.getNeighbors()) {
                            counts += &blockSource->getBlock(calPos) != BedrockBlocks::mAir;
                        }
                        if (counts == 6) {
                            auto localPos = pos - boundingBox.min + 1;
                            tmp[(localPos.y + sizeDim.y * localPos.z) * sizeDim.x + localPos.x] = true;
                        }
                    });

                    for (int manhattan = 0; manhattan < layer; manhattan++) {
                        std::vector<bool> tmp2(tmp);
                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            int  counts   = 0;
                            auto localPos = pos - boundingBox.min + 1;
                            if (tmp[(localPos.y + sizeDim.y * localPos.z) * sizeDim.x + localPos.x]) {
                                for (auto& calPos : localPos.getNeighbors()) {
                                    counts += tmp[(calPos.y + sizeDim.y * calPos.z) * sizeDim.x + calPos.x];
                                }
                                if (counts < 6) {
                                    tmp2[(localPos.y + sizeDim.y * localPos.z) * sizeDim.x + localPos.x] = false;
                                }
                            }
                        });
                        tmp.assign(tmp2.begin(), tmp2.end());
                    }

                    auto          playerPos = origin.getPlayer()->getPosition();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;

                    long long i = 0;

                    std::string bps = "minecraft:air";
                    if (results["blockPattern"].isSet) {
                        bps = results["blockPattern"].get<std::string>();
                    } else if (results["block"].isSet) {
                        bps = results["block"].get<Block const*>()->getTypeName();
                    }
                    BlockPattern blockPattern(bps, xuid, region);
                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        auto localPos = pos - boundingBox.min + 1;
                        setFunction(variables, f, boundingBox, playerPos, pos, center);
                        if (gMask == "" || cpp_eval::eval<double>(gMask.c_str(), variables, f) > 0.5) {
                            auto localPos = pos - boundingBox.min + 1;
                            if (tmp[(localPos.y + sizeDim.y * localPos.z) * sizeDim.x + localPos.x]) {
                                blockPattern.setBlock(variables, f, blockSource, pos);
                                i++;
                            }
                        }
                    });
                    output.success(fmt::format("§a{} block(s) hollowed", i));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "smooth",            // command name
            "smooth selection",  // command description
            {}, {ParamData("num", ParamType::Int, true, "num")}, {{"num"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod  = worldedit::getMod();
                auto  xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    int ksize = 5;
                    if (results["num"].isSet) {
                        ksize = results["num"].get<int>();
                    }

                    auto* region      = mod.playerRegionMap[xuid];
                    auto  center      = region->getCenter();
                    auto  dimID       = region->getDimensionID();
                    auto  boundingBox = region->getBoundBox();
                    auto  blockSource = Level::getBlockSource(dimID);

                    auto history            = mod.getPlayerNextHistory(xuid);
                    *history                = Clipboard(boundingBox.max - boundingBox.min);
                    history->playerRelPos.x = dimID;
                    history->playerPos      = boundingBox.min;
                    // history->used = true;

                    boundingBox.forEachBlockInBox([&](const BlockPos& pos) {
                        auto localPos      = pos - boundingBox.min;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history->storeBlock(blockInstance, localPos);
                    });

                    auto heightMap = region->getHeightMap();
                    int  sizex     = boundingBox.max.x - boundingBox.min.x + 1;
                    int  sizez     = boundingBox.max.z - boundingBox.min.z + 1;

                    auto smoothedHeightMap = blur2D(heightMap, ksize, sizex, sizez);
                    region->applyHeightMap(smoothedHeightMap);

                    output.success(fmt::format("§aRegion smoothed"));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "image",      // command name
            "set image",  // command description
            {}, {ParamData("file", ParamType::String, "file")}, {{"file"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod  = worldedit::getMod();
                auto  xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region      = mod.playerRegionMap[xuid];
                    auto  center      = region->getCenter();
                    auto  dimID       = region->getDimensionID();
                    auto  boundingBox = region->getBoundBox();
                    auto  size        = boundingBox.max - boundingBox.min;
                    auto  blockSource = Level::getBlockSource(dimID);

                    if (size.x != 0 && size.y != 0 && size.z != 0) {
                        output.error("You need to select a slice");
                    }
                    auto history            = mod.getPlayerNextHistory(xuid);
                    *history                = Clipboard(boundingBox.max - boundingBox.min);
                    history->playerRelPos.x = dimID;
                    history->playerPos      = boundingBox.min;

                    boundingBox.forEachBlockInBox([&](const BlockPos& pos) {
                        auto localPos      = pos - boundingBox.min;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history->storeBlock(blockInstance, localPos);
                    });

                    std::string gMask = "";
                    if (mod.playerGMaskMap.find(xuid) != mod.playerGMaskMap.end()) {
                        gMask = mod.playerGMaskMap[xuid];
                    }

                    std::unordered_map<mce::Color, int> blockColorMap;
                    blockColorMap[mce::Color(208, 214, 215)] = 0;
                    blockColorMap[mce::Color(224, 96, 0)]    = 1;
                    blockColorMap[mce::Color(170, 45, 160)]  = 2;
                    blockColorMap[mce::Color(30, 138, 200)]  = 3;
                    blockColorMap[mce::Color(240, 175, 13)]  = 4;
                    blockColorMap[mce::Color(93, 168, 16)]   = 5;
                    blockColorMap[mce::Color(215, 102, 145)] = 6;
                    blockColorMap[mce::Color(52, 56, 60)]    = 7;
                    blockColorMap[mce::Color(125, 125, 125)] = 8;
                    blockColorMap[mce::Color(13, 119, 136)]  = 9;
                    blockColorMap[mce::Color(100, 25, 157)]  = 10;
                    blockColorMap[mce::Color(40, 42, 144)]   = 11;
                    blockColorMap[mce::Color(96, 57, 25)]    = 12;
                    blockColorMap[mce::Color(72, 91, 31)]    = 13;
                    blockColorMap[mce::Color(144, 30, 30)]   = 14;
                    blockColorMap[mce::Color(2, 3, 7)]       = 15;

                    auto          playerPos = origin.getPlayer()->getPosition();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;

                    long long i = 0;

                    auto filename = WE_PNG_DIR + results["file"].get<std::string>();

                    auto texture2D = loadpng(filename);

                    Sampler sampler(SamplerType::Bilinear, EdgeType::FLIP);

                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        setFunction(variables, f, boundingBox, playerPos, pos, center);
                        if (gMask == "" || cpp_eval::eval<double>(gMask.c_str(), variables, f) > 0.5) {
                            auto   localPos = pos - boundingBox.min;
                            double u, v;
                            if (size.x == 0) {
                                u = localPos.z;
                                u /= size.z;
                                v = localPos.y;
                                v /= size.y;
                            } else if (size.z == 0) {
                                u = localPos.x;
                                u /= size.x;
                                v = localPos.y;
                                v /= size.y;
                            } else {
                                u = localPos.x;
                                u /= size.x;
                                v = localPos.z;
                                v /= size.z;
                            }
                            u = 1 - u;
                            v = 1 - v;

                            auto color = texture2D.sample(sampler, u, v);

                            double minDist = DBL_MAX;
                            int    minTile = 15;
                            for (auto& i : blockColorMap) {
                                auto dst = i.first.distanceTo(color);
                                if (dst < minDist) {
                                    minDist = dst;
                                    minTile = i.second;
                                }
                            }
                            if (uniformRandDouble() <= color.a){
                                setBlockSimple(blockSource, pos, Block::create("minecraft:concrete", minTile));
                            i++;
                            }
                        }
                    });

                    output.success(fmt::format("§apng setted"));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit

#endif  // WORLDEDIT_REGIONOPERATIONCOMMAND_H