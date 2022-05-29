//
// Created by OEOTYAN on 2022/05/27.
//
#pragma once
#ifndef WORLDEDIT_REGIONOPERATIONCOMMAND_H
#define WORLDEDIT_REGIONOPERATIONCOMMAND_H

#include "store/BlockPattern.hpp"
#include "eval/blur.hpp"

namespace worldedit {
    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;

    // set rep center stack move gen walls faces overlay naturalize hollow
    // | deform smooth

    void regionOperationCommandSetup() {
        DynamicCommand::setup(
            "set",        // command name
            "set block",  // command description
            {},
            {ParamData("block", ParamType::Block, "block"),
             ParamData("blockPattern", ParamType::String, "blockPattern")},
            {{"block"}, {"blockPattern"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) !=
                        mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto blockSource = Level::getBlockSource(dimID);

                    auto history = mod.getPlayerNextHistory(xuid);
                    *history = Clipboard(boundingBox.bpos2 - boundingBox.bpos1);
                    history->playerRelPos.x = dimID;
                    history->playerPos = boundingBox.bpos1;
                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        auto localPos = pos - boundingBox.bpos1;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history->storeBlock(blockInstance, localPos);
                    });

                    long long i = 0;

                    std::string gMask = "";
                    if (mod.playerGMaskMap.find(xuid) !=
                        mod.playerGMaskMap.end()) {
                        gMask = mod.playerGMaskMap[xuid];
                    }

                    auto playerPos = origin.getPlayer()->getPosition();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    std::unordered_map<std::string, double> variables;

                    if (results["block"].isSet) {
                        auto* block = const_cast<Block*>(
                            results["block"].get<Block const*>());
                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            setFunction(variables, f, boundingBox, playerPos,
                                        pos);
                            if (gMask == "" ||
                                cpp_eval::eval<double>(gMask.c_str(), variables,
                                                       f) > 0.5) {
                                setBlockSimple(blockSource, pos, block);
                                i++;
                            }
                        });
                    } else {
                        auto bps = results["blockPattern"].get<std::string>();
                        BlockPattern blockPattern(bps);
                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            setFunction(variables, f, boundingBox, playerPos,
                                        pos);
                            if (gMask == "" ||
                                cpp_eval::eval<double>(gMask.c_str(), variables,
                                                       f) > 0.5) {
                                blockPattern.setBlock(variables, f, blockSource,
                                                      pos);
                                i++;
                            }
                        });
                    }
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
             ParamData("blockBeforeS", ParamType::String, true,
                       "blockBeforeS")},
            {{"blockBefore", "blockAfter"}, {"blockBeforeS", "blockAfterS"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) !=
                        mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto blockSource = Level::getBlockSource(dimID);

                    auto history = mod.getPlayerNextHistory(xuid);
                    *history = Clipboard(boundingBox.bpos2 - boundingBox.bpos1);
                    history->playerRelPos.x = dimID;
                    history->playerPos = boundingBox.bpos1;
                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        auto localPos = pos - boundingBox.bpos1;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history->storeBlock(blockInstance, localPos);
                    });

                    long long i = 0;

                    std::string gMask = "";
                    if (mod.playerGMaskMap.find(xuid) !=
                        mod.playerGMaskMap.end()) {
                        gMask = mod.playerGMaskMap[xuid];
                    }

                    auto playerPos = origin.getPlayer()->getPosition();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    std::unordered_map<std::string, double> variables;

                    if (results["blockAfter"].isSet) {
                        auto* block = const_cast<Block*>(
                            results["blockAfter"].get<Block const*>());
                        if (results["blockBefore"].isSet) {
                            auto* blockFilter = const_cast<Block*>(
                                results["blockBefore"].get<Block const*>());
                            region->forEachBlockInRegion([&](const BlockPos&
                                                                 pos) {
                                setFunction(variables, f, boundingBox,
                                            playerPos, pos);
                                if ((gMask == "" ||
                                     cpp_eval::eval<double>(
                                         gMask.c_str(), variables, f) > 0.5) &&
                                    blockSource->getBlock(pos).getTypeName() ==
                                        blockFilter->getTypeName()) {
                                    setBlockSimple(blockSource, pos, block);
                                    i++;
                                }
                            });
                        } else {
                            region->forEachBlockInRegion(
                                [&](const BlockPos& pos) {
                                    setFunction(variables, f, boundingBox,
                                                playerPos, pos);
                                    if ((gMask == "" ||
                                         cpp_eval::eval<double>(gMask.c_str(),
                                                                variables,
                                                                f) > 0.5) &&
                                        &blockSource->getBlock(pos) !=
                                            BedrockBlocks::mAir) {
                                        setBlockSimple(blockSource, pos, block);
                                        i++;
                                    }
                                });
                        }
                    } else {
                        auto bps = results["blockAfterS"].get<std::string>();
                        BlockPattern blockPattern(bps);
                        if (results["blockBeforeS"].isSet) {
                            auto bps2 =
                                results["blockBeforeS"].get<std::string>();
                            BlockPattern blockFilter(bps2);
                            region->forEachBlockInRegion(
                                [&](const BlockPos& pos) {
                                    setFunction(variables, f, boundingBox,
                                                playerPos, pos);
                                    if ((gMask == "" ||
                                         cpp_eval::eval<double>(gMask.c_str(),
                                                                variables,
                                                                f) > 0.5) &&
                                        blockFilter.hasBlock(const_cast<Block*>(
                                            &blockSource->getBlock(pos)))) {
                                        blockPattern.setBlock(variables, f,
                                                              blockSource, pos);
                                        i++;
                                    }
                                });
                        } else {
                            region->forEachBlockInRegion(
                                [&](const BlockPos& pos) {
                                    setFunction(variables, f, boundingBox,
                                                playerPos, pos);
                                    if ((gMask == "" ||
                                         cpp_eval::eval<double>(gMask.c_str(),
                                                                variables,
                                                                f) > 0.5) &&
                                        &blockSource->getBlock(pos) !=
                                            BedrockBlocks::mAir) {
                                        blockPattern.setBlock(variables, f,
                                                              blockSource, pos);
                                        i++;
                                    }
                                });
                        }
                    }
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
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) !=
                        mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];
                    auto dimID = region->getDimensionID();
                    auto center = region->getCenter();
                    BoundingBox boundingBox;
                    boundingBox.bpos1.x =
                        static_cast<int>(floor(center.x - 0.49));
                    boundingBox.bpos1.y =
                        static_cast<int>(floor(center.y - 0.49));
                    boundingBox.bpos1.z =
                        static_cast<int>(floor(center.z - 0.49));
                    boundingBox.bpos2.x =
                        static_cast<int>(floor(center.x + 0.49));
                    boundingBox.bpos2.y =
                        static_cast<int>(floor(center.y + 0.49));
                    boundingBox.bpos2.z =
                        static_cast<int>(floor(center.z + 0.49));
                    auto blockSource = Level::getBlockSource(dimID);

                    auto history = mod.getPlayerNextHistory(xuid);
                    *history = Clipboard(boundingBox.bpos2 - boundingBox.bpos1);
                    history->playerRelPos.x = dimID;
                    history->playerPos = boundingBox.bpos1;
                    for (int y = boundingBox.bpos1.y; y <= boundingBox.bpos2.y;
                         y++)
                        for (int x = boundingBox.bpos1.x;
                             x <= boundingBox.bpos2.x; x++)
                            for (int z = boundingBox.bpos1.z;
                                 z <= boundingBox.bpos2.z; z++) {
                                BlockPos pos(x, y, z);
                                auto localPos = pos - boundingBox.bpos1;
                                auto blockInstance =
                                    blockSource->getBlockInstance(pos);
                                history->storeBlock(blockInstance, localPos);
                            }

                    auto playerPos = origin.getPlayer()->getPosition();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    std::unordered_map<std::string, double> variables;

                    if (results["block"].isSet) {
                        auto* block = const_cast<Block*>(
                            results["block"].get<Block const*>());
                        for (int y = boundingBox.bpos1.y;
                             y <= boundingBox.bpos2.y; y++)
                            for (int x = boundingBox.bpos1.x;
                                 x <= boundingBox.bpos2.x; x++)
                                for (int z = boundingBox.bpos1.z;
                                     z <= boundingBox.bpos2.z; z++) {
                                    BlockPos pos(x, y, z);
                                    setFunction(variables, f, boundingBox,
                                                playerPos, pos);
                                    setBlockSimple(blockSource, pos, block);
                                }
                    } else {
                        auto bps = results["blockPattern"].get<std::string>();
                        BlockPattern blockPattern(bps);
                        for (int y = boundingBox.bpos1.y;
                             y <= boundingBox.bpos2.y; y++)
                            for (int x = boundingBox.bpos1.x;
                                 x <= boundingBox.bpos2.x; x++)
                                for (int z = boundingBox.bpos1.z;
                                     z <= boundingBox.bpos2.z; z++) {
                                    BlockPos pos(x, y, z);
                                    setFunction(variables, f, boundingBox,
                                                playerPos, pos);
                                    blockPattern.setBlock(variables, f,
                                                          blockSource, pos);
                                }
                    }
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
                {"dir",
                 {"me", "back", "up", "down", "south", "north", "east",
                  "west"}},
            },
            {ParamData("times", ParamType::Int, "times"),
             ParamData("dir", ParamType::Enum, true, "dir"),
             ParamData("args", ParamType::String, true, "-sa")},
            {{"times", "dir", "args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) !=
                        mod.playerRegionMap.end() &&
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
                    BlockPos faceVec = worldedit::facingToPos(facing, 1);

                    auto* region = mod.playerRegionMap[xuid];
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto boundingBoxLast = boundingBox;
                    auto blockSource = Level::getBlockSource(dimID);

                    auto movingVec =
                        faceVec * (boundingBox.bpos2 - boundingBox.bpos1 + 1);

                    boundingBoxLast.bpos1 =
                        boundingBoxLast.bpos1 + movingVec * times;
                    boundingBoxLast.bpos2 =
                        boundingBoxLast.bpos2 + movingVec * times;
                    auto boundingBoxHistory = boundingBoxLast;

                    boundingBoxHistory.bpos1.x = std::min(
                        boundingBoxHistory.bpos1.x, boundingBox.bpos1.x);
                    boundingBoxHistory.bpos1.y = std::min(
                        boundingBoxHistory.bpos1.y, boundingBox.bpos1.y);
                    boundingBoxHistory.bpos1.z = std::min(
                        boundingBoxHistory.bpos1.z, boundingBox.bpos1.z);
                    boundingBoxHistory.bpos2.x = std::max(
                        boundingBoxHistory.bpos2.x, boundingBox.bpos2.x);
                    boundingBoxHistory.bpos2.y = std::max(
                        boundingBoxHistory.bpos2.y, boundingBox.bpos2.y);
                    boundingBoxHistory.bpos2.z = std::max(
                        boundingBoxHistory.bpos2.z, boundingBox.bpos2.z);

                    auto history = mod.getPlayerNextHistory(xuid);
                    *history = Clipboard(boundingBoxHistory.bpos2 -
                                         boundingBoxHistory.bpos1);
                    history->playerRelPos.x = dimID;
                    history->playerPos = boundingBoxHistory.bpos1;
                    for (int y = boundingBoxHistory.bpos1.y;
                         y <= boundingBoxHistory.bpos2.y; y++)
                        for (int x = boundingBoxHistory.bpos1.x;
                             x <= boundingBoxHistory.bpos2.x; x++)
                            for (int z = boundingBoxHistory.bpos1.z;
                                 z <= boundingBoxHistory.bpos2.z; z++) {
                                BlockPos pos(x, y, z);
                                auto localPos = pos - boundingBoxHistory.bpos1;
                                auto blockInstance =
                                    blockSource->getBlockInstance(pos);
                                history->storeBlock(blockInstance, localPos);
                            }

                    long long i = 0;

                    std::string gMask = "";
                    if (mod.playerGMaskMap.find(xuid) !=
                        mod.playerGMaskMap.end()) {
                        gMask = mod.playerGMaskMap[xuid];
                    }

                    auto playerPos = origin.getPlayer()->getPosition();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    std::unordered_map<std::string, double> variables;
                    region->forEachBlockInRegion([&](const BlockPos& posk) {
                        for (int j = 1; j <= times; j++) {
                            auto pos = posk + movingVec * j;
                            setFunction(variables, f, boundingBox, playerPos,
                                        pos);
                            if ((arg_a && (&blockSource->getBlock(posk) !=
                                           BedrockBlocks::mAir)) ||
                                !arg_a)
                                if (gMask == "" ||
                                    cpp_eval::eval<double>(
                                        gMask.c_str(), variables, f) > 0.5) {
                                    setBlockSimple(
                                        blockSource, pos,
                                        const_cast<Block*>(
                                            &blockSource->getBlock(posk)));
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
                {"dir",
                 {"me", "back", "up", "down", "south", "north", "east",
                  "west"}},
            },
            {ParamData("dis", ParamType::Int, "dis"),
             ParamData("dir", ParamType::Enum, true, "dir"),
             ParamData("args", ParamType::String, true, "-sa")},
            {{"dis", "dir", "args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) !=
                        mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto dis = results["dis"].get<int>();
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
                    BlockPos faceVec = worldedit::facingToPos(facing, dis);

                    auto* region = mod.playerRegionMap[xuid];
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto boundingBoxLast = boundingBox;
                    auto blockSource = Level::getBlockSource(dimID);

                    boundingBoxLast.bpos1 = boundingBoxLast.bpos1 + faceVec;
                    boundingBoxLast.bpos2 = boundingBoxLast.bpos2 + faceVec;
                    auto boundingBoxHistory = boundingBoxLast;

                    boundingBoxHistory.bpos1.x = std::min(
                        boundingBoxHistory.bpos1.x, boundingBox.bpos1.x);
                    boundingBoxHistory.bpos1.y = std::min(
                        boundingBoxHistory.bpos1.y, boundingBox.bpos1.y);
                    boundingBoxHistory.bpos1.z = std::min(
                        boundingBoxHistory.bpos1.z, boundingBox.bpos1.z);
                    boundingBoxHistory.bpos2.x = std::max(
                        boundingBoxHistory.bpos2.x, boundingBox.bpos2.x);
                    boundingBoxHistory.bpos2.y = std::max(
                        boundingBoxHistory.bpos2.y, boundingBox.bpos2.y);
                    boundingBoxHistory.bpos2.z = std::max(
                        boundingBoxHistory.bpos2.z, boundingBox.bpos2.z);

                    auto history = mod.getPlayerNextHistory(xuid);
                    *history = Clipboard(boundingBoxHistory.bpos2 -
                                         boundingBoxHistory.bpos1);
                    history->playerRelPos.x = dimID;
                    history->playerPos = boundingBoxHistory.bpos1;
                    for (int y = boundingBox.bpos1.y; y <= boundingBox.bpos2.y;
                         y++)
                        for (int x = boundingBox.bpos1.x;
                             x <= boundingBox.bpos2.x; x++)
                            for (int z = boundingBox.bpos1.z;
                                 z <= boundingBox.bpos2.z; z++) {
                                BlockPos pos(x, y, z);
                                auto localPos = pos - boundingBoxHistory.bpos1;
                                auto blockInstance =
                                    blockSource->getBlockInstance(pos);
                                history->storeBlock(blockInstance, localPos);
                            }
                    for (int y = boundingBoxLast.bpos1.y;
                         y <= boundingBoxLast.bpos2.y; y++)
                        for (int x = boundingBoxLast.bpos1.x;
                             x <= boundingBoxLast.bpos2.x; x++)
                            for (int z = boundingBoxLast.bpos1.z;
                                 z <= boundingBoxLast.bpos2.z; z++) {
                                BlockPos pos(x, y, z);
                                auto localPos = pos - boundingBoxHistory.bpos1;
                                if (!history->getSet(localPos).hasBlock) {
                                    auto blockInstance =
                                        blockSource->getBlockInstance(pos);
                                    history->storeBlock(blockInstance,
                                                        localPos);
                                }
                            }

                    long long i = 0;

                    std::string gMask = "";
                    if (mod.playerGMaskMap.find(xuid) !=
                        mod.playerGMaskMap.end()) {
                        gMask = mod.playerGMaskMap[xuid];
                    }

                    auto playerPos = origin.getPlayer()->getPosition();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    std::unordered_map<std::string, double> variables;
                    region->forEachBlockInRegion([&](const BlockPos& posk) {
                        auto pos = posk + faceVec;
                        setFunction(variables, f, boundingBox, playerPos, pos);
                        if (gMask == "" ||
                            cpp_eval::eval<double>(gMask.c_str(), variables,
                                                   f) > 0.5) {
                            auto localPos = posk - boundingBoxHistory.bpos1;
                            setBlockSimple(blockSource, posk);
                            i++;
                        }
                    });
                    region->forEachBlockInRegion([&](const BlockPos& posk) {
                        auto pos = posk + faceVec;
                        auto localPos = posk - boundingBoxHistory.bpos1;
                        auto block = history->getSet(localPos).getBlock();
                        setFunction(variables, f, boundingBox, playerPos, pos);
                        if ((arg_a && (block != BedrockBlocks::mAir)) || !arg_a)
                            if (gMask == "" ||
                                cpp_eval::eval<double>(gMask.c_str(), variables,
                                                       f) > 0.5) {
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
             ParamData("function", ParamType::String, "function"),
             ParamData("args", ParamType::String, true, "-h")},
            {{"block", "function", "args"},
             {"blockPattern", "function", "args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) !=
                        mod.playerRegionMap.end() &&
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

                    auto* region = mod.playerRegionMap[xuid];
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto blockSource = Level::getBlockSource(dimID);

                    auto history = mod.getPlayerNextHistory(xuid);
                    *history = Clipboard(boundingBox.bpos2 - boundingBox.bpos1);
                    history->playerRelPos.x = dimID;
                    history->playerPos = boundingBox.bpos1;
                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        auto localPos = pos - boundingBox.bpos1;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history->storeBlock(blockInstance, localPos);
                    });

                    long long i = 0;

                    std::string gMask = "";
                    if (mod.playerGMaskMap.find(xuid) !=
                        mod.playerGMaskMap.end()) {
                        gMask = mod.playerGMaskMap[xuid];
                    }

                    auto genfunc = results["function"].get<std::string>();

                    auto playerPos = origin.getPlayer()->getPosition();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    std::unordered_map<std::string, double> variables;

                    auto sizeDim = boundingBox.bpos2 - boundingBox.bpos1 + 3;

                    long long size = (sizeDim.x) * (sizeDim.y) * (sizeDim.z);

                    std::vector<bool> tmp(size, false);
                    std::vector<bool> caled(size, false);

                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        setFunction(variables, f, boundingBox, playerPos, pos);
                        if (gMask == "" ||
                            cpp_eval::eval<double>(gMask.c_str(), variables,
                                                   f) > 0.5) {
                            auto localPos = pos - boundingBox.bpos1 + 1;
                            if (cpp_eval::eval<double>(genfunc.c_str(),
                                                       variables, f) > 0.5) {
                                tmp[(localPos.y + sizeDim.y * localPos.z) *
                                        sizeDim.x +
                                    localPos.x] = true;
                            }
                            caled[(localPos.y + sizeDim.y * localPos.z) *
                                      sizeDim.x +
                                  localPos.x] = true;
                        }
                    });

                    if (arg_h) {
                        std::vector<bool> tmp2(size, false);
                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            auto localPos = pos - boundingBox.bpos1 + 1;
                            auto calPos = localPos;
                            if (tmp[(localPos.y + sizeDim.y * localPos.z) *
                                        sizeDim.x +
                                    localPos.x]) {
                                int counts = 0;

                                calPos = localPos + BlockPos(0, 1, 0);
                                if (!caled[(calPos.y + sizeDim.y * calPos.z) *
                                               sizeDim.x +
                                           calPos.x]) {
                                    setFunction(variables, f, boundingBox,
                                                playerPos,
                                                pos + BlockPos(0, 1, 0));
                                    tmp[(calPos.y + sizeDim.y * calPos.z) *
                                            sizeDim.x +
                                        calPos.x] =
                                        cpp_eval::eval<double>(genfunc.c_str(),
                                                               variables,
                                                               f) > 0.5;

                                    caled[(calPos.y + sizeDim.y * calPos.z) *
                                              sizeDim.x +
                                          calPos.x] = true;
                                }
                                if (tmp[(calPos.y + sizeDim.y * calPos.z) *
                                            sizeDim.x +
                                        calPos.x]) {
                                    counts++;
                                }

                                calPos = localPos + BlockPos(0, -1, 0);
                                if (!caled[(calPos.y + sizeDim.y * calPos.z) *
                                               sizeDim.x +
                                           calPos.x]) {
                                    setFunction(variables, f, boundingBox,
                                                playerPos,
                                                pos + BlockPos(0, -1, 0));
                                    tmp[(calPos.y + sizeDim.y * calPos.z) *
                                            sizeDim.x +
                                        calPos.x] =
                                        cpp_eval::eval<double>(genfunc.c_str(),
                                                               variables,
                                                               f) > 0.5;
                                    caled[(calPos.y + sizeDim.y * calPos.z) *
                                              sizeDim.x +
                                          calPos.x] = true;
                                }
                                if (tmp[(calPos.y + sizeDim.y * calPos.z) *
                                            sizeDim.x +
                                        calPos.x]) {
                                    counts++;
                                }

                                calPos = localPos + BlockPos(1, 0, 0);
                                if (!caled[(calPos.y + sizeDim.y * calPos.z) *
                                               sizeDim.x +
                                           calPos.x]) {
                                    setFunction(variables, f, boundingBox,
                                                playerPos,
                                                pos + BlockPos(1, 0, 0));
                                    tmp[(calPos.y + sizeDim.y * calPos.z) *
                                            sizeDim.x +
                                        calPos.x] =
                                        cpp_eval::eval<double>(genfunc.c_str(),
                                                               variables,
                                                               f) > 0.5;
                                    caled[(calPos.y + sizeDim.y * calPos.z) *
                                              sizeDim.x +
                                          calPos.x] = true;
                                }
                                if (tmp[(calPos.y + sizeDim.y * calPos.z) *
                                            sizeDim.x +
                                        calPos.x]) {
                                    counts++;
                                }

                                calPos = localPos + BlockPos(-1, 0, 0);
                                if (!caled[(calPos.y + sizeDim.y * calPos.z) *
                                               sizeDim.x +
                                           calPos.x]) {
                                    setFunction(variables, f, boundingBox,
                                                playerPos,
                                                pos + BlockPos(-1, 0, 0));
                                    tmp[(calPos.y + sizeDim.y * calPos.z) *
                                            sizeDim.x +
                                        calPos.x] =
                                        cpp_eval::eval<double>(genfunc.c_str(),
                                                               variables,
                                                               f) > 0.5;
                                    caled[(calPos.y + sizeDim.y * calPos.z) *
                                              sizeDim.x +
                                          calPos.x] = true;
                                }
                                if (tmp[(calPos.y + sizeDim.y * calPos.z) *
                                            sizeDim.x +
                                        calPos.x]) {
                                    counts++;
                                }

                                calPos = localPos + BlockPos(0, 0, 1);
                                if (!caled[(calPos.y + sizeDim.y * calPos.z) *
                                               sizeDim.x +
                                           calPos.x]) {
                                    setFunction(variables, f, boundingBox,
                                                playerPos,
                                                pos + BlockPos(0, 0, 1));
                                    tmp[(calPos.y + sizeDim.y * calPos.z) *
                                            sizeDim.x +
                                        calPos.x] =
                                        cpp_eval::eval<double>(genfunc.c_str(),
                                                               variables,
                                                               f) > 0.5;
                                    caled[(calPos.y + sizeDim.y * calPos.z) *
                                              sizeDim.x +
                                          calPos.x] = true;
                                }
                                if (tmp[(calPos.y + sizeDim.y * calPos.z) *
                                            sizeDim.x +
                                        calPos.x]) {
                                    counts++;
                                }

                                calPos = localPos + BlockPos(0, 0, -1);
                                if (!caled[(calPos.y + sizeDim.y * calPos.z) *
                                               sizeDim.x +
                                           calPos.x]) {
                                    setFunction(variables, f, boundingBox,
                                                playerPos,
                                                pos + BlockPos(0, 0, -1));
                                    tmp[(calPos.y + sizeDim.y * calPos.z) *
                                            sizeDim.x +
                                        calPos.x] =
                                        cpp_eval::eval<double>(genfunc.c_str(),
                                                               variables,
                                                               f) > 0.5;
                                    caled[(calPos.y + sizeDim.y * calPos.z) *
                                              sizeDim.x +
                                          calPos.x] = true;
                                }
                                if (tmp[(calPos.y + sizeDim.y * calPos.z) *
                                            sizeDim.x +
                                        calPos.x]) {
                                    counts++;
                                }
                                // std::cout << counts << " " << pos.x << " "
                                //           << pos.y << " " << pos.z <<
                                //           std::endl;
                                if (counts < 6)
                                    tmp2[(localPos.y + sizeDim.y * localPos.z) *
                                             sizeDim.x +
                                         localPos.x] = true;
                            }
                        });
                        tmp.assign(tmp2.begin(), tmp2.end());
                    }

                    if (results["block"].isSet) {
                        auto* block = const_cast<Block*>(
                            results["block"].get<Block const*>());
                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            auto localPos = pos - boundingBox.bpos1 + 1;
                            if (tmp[(localPos.y + sizeDim.y * localPos.z) *
                                        sizeDim.x +
                                    localPos.x]) {
                                setBlockSimple(blockSource, pos, block);
                                i++;
                            }
                        });
                    } else {
                        auto bps = results["blockPattern"].get<std::string>();
                        BlockPattern blockPattern(bps);
                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            auto localPos = pos - boundingBox.bpos1 + 1;
                            if (tmp[(localPos.y + sizeDim.y * localPos.z) *
                                        sizeDim.x +
                                    localPos.x]) {
                                setFunction(variables, f, boundingBox,
                                            playerPos, pos);
                                blockPattern.setBlock(variables, f, blockSource,
                                                      pos);
                                i++;
                            }
                        });
                    }
                    output.success(fmt::format(
                        "§gfunction: {}\n§a{} block(s) placed", genfunc, i));
                    ;
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
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) !=
                        mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto blockSource = Level::getBlockSource(dimID);

                    auto history = mod.getPlayerNextHistory(xuid);
                    *history = Clipboard(boundingBox.bpos2 - boundingBox.bpos1);
                    history->playerRelPos.x = dimID;
                    history->playerPos = boundingBox.bpos1;
                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        auto localPos = pos - boundingBox.bpos1;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history->storeBlock(blockInstance, localPos);
                    });

                    long long i = 0;

                    std::string gMask = "";
                    if (mod.playerGMaskMap.find(xuid) !=
                        mod.playerGMaskMap.end()) {
                        gMask = mod.playerGMaskMap[xuid];
                    }

                    auto playerPos = origin.getPlayer()->getPosition();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    std::unordered_map<std::string, double> variables;

                    if (results["block"].isSet) {
                        auto* block = const_cast<Block*>(
                            results["block"].get<Block const*>());
                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            setFunction(variables, f, boundingBox, playerPos,
                                        pos);
                            if (gMask == "" ||
                                cpp_eval::eval<double>(gMask.c_str(), variables,
                                                       f) > 0.5) {
                                int counts = 0;
                                if (pos.y < boundingBox.bpos2.y) {
                                    counts += region->contains(
                                        pos + BlockPos(0, 1, 0));
                                } else {
                                    counts += 1;
                                }
                                if (pos.y > boundingBox.bpos1.y) {
                                    counts += region->contains(
                                        pos + BlockPos(0, -1, 0));
                                } else {
                                    counts += 1;
                                }
                                counts +=
                                    region->contains(pos + BlockPos(1, 0, 0));
                                counts +=
                                    region->contains(pos + BlockPos(-1, 0, 0));
                                counts +=
                                    region->contains(pos + BlockPos(0, 0, 1));
                                counts +=
                                    region->contains(pos + BlockPos(0, 0, -1));
                                if (counts < 6) {
                                    setBlockSimple(blockSource, pos, block);
                                    i++;
                                }
                            }
                        });
                    } else {
                        auto bps = results["blockPattern"].get<std::string>();
                        BlockPattern blockPattern(bps);
                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            setFunction(variables, f, boundingBox, playerPos,
                                        pos);
                            if (gMask == "" ||
                                cpp_eval::eval<double>(gMask.c_str(), variables,
                                                       f) > 0.5) {
                                int counts = 0;
                                if (pos.y < boundingBox.bpos2.y) {
                                    counts += region->contains(
                                        pos + BlockPos(0, 1, 0));
                                } else {
                                    counts += 1;
                                }
                                if (pos.y > boundingBox.bpos1.y) {
                                    counts += region->contains(
                                        pos + BlockPos(0, -1, 0));
                                } else {
                                    counts += 1;
                                }
                                counts +=
                                    region->contains(pos + BlockPos(1, 0, 0));
                                counts +=
                                    region->contains(pos + BlockPos(-1, 0, 0));
                                counts +=
                                    region->contains(pos + BlockPos(0, 0, 1));
                                counts +=
                                    region->contains(pos + BlockPos(0, 0, -1));
                                if (counts < 6) {
                                    blockPattern.setBlock(variables, f,
                                                          blockSource, pos);
                                    i++;
                                }
                            }
                        });
                    }
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
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) !=
                        mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto blockSource = Level::getBlockSource(dimID);

                    auto history = mod.getPlayerNextHistory(xuid);
                    *history = Clipboard(boundingBox.bpos2 - boundingBox.bpos1);
                    history->playerRelPos.x = dimID;
                    history->playerPos = boundingBox.bpos1;
                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        auto localPos = pos - boundingBox.bpos1;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history->storeBlock(blockInstance, localPos);
                    });

                    long long i = 0;

                    std::string gMask = "";
                    if (mod.playerGMaskMap.find(xuid) !=
                        mod.playerGMaskMap.end()) {
                        gMask = mod.playerGMaskMap[xuid];
                    }

                    auto playerPos = origin.getPlayer()->getPosition();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    std::unordered_map<std::string, double> variables;

                    if (results["block"].isSet) {
                        auto* block = const_cast<Block*>(
                            results["block"].get<Block const*>());
                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            setFunction(variables, f, boundingBox, playerPos,
                                        pos);
                            if (gMask == "" ||
                                cpp_eval::eval<double>(gMask.c_str(), variables,
                                                       f) > 0.5) {
                                int counts = 0;
                                counts +=
                                    region->contains(pos + BlockPos(0, 1, 0));
                                counts +=
                                    region->contains(pos + BlockPos(0, -1, 0));
                                counts +=
                                    region->contains(pos + BlockPos(1, 0, 0));
                                counts +=
                                    region->contains(pos + BlockPos(-1, 0, 0));
                                counts +=
                                    region->contains(pos + BlockPos(0, 0, 1));
                                counts +=
                                    region->contains(pos + BlockPos(0, 0, -1));
                                if (counts < 6) {
                                    setBlockSimple(blockSource, pos, block);
                                    i++;
                                }
                            }
                        });
                    } else {
                        auto bps = results["blockPattern"].get<std::string>();
                        BlockPattern blockPattern(bps);
                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            setFunction(variables, f, boundingBox, playerPos,
                                        pos);
                            if (gMask == "" ||
                                cpp_eval::eval<double>(gMask.c_str(), variables,
                                                       f) > 0.5) {
                                int counts = 0;
                                counts +=
                                    region->contains(pos + BlockPos(0, 1, 0));
                                counts +=
                                    region->contains(pos + BlockPos(0, -1, 0));
                                counts +=
                                    region->contains(pos + BlockPos(1, 0, 0));
                                counts +=
                                    region->contains(pos + BlockPos(-1, 0, 0));
                                counts +=
                                    region->contains(pos + BlockPos(0, 0, 1));
                                counts +=
                                    region->contains(pos + BlockPos(0, 0, -1));
                                if (counts < 6) {
                                    blockPattern.setBlock(variables, f,
                                                          blockSource, pos);
                                    i++;
                                }
                            }
                        });
                    }
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
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) !=
                        mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto blockSource = Level::getBlockSource(dimID);

                    auto history = mod.getPlayerNextHistory(xuid);
                    *history = Clipboard(boundingBox.bpos2 - boundingBox.bpos1);
                    history->playerRelPos.x = dimID;
                    history->playerPos = boundingBox.bpos1;
                    region->forTopBlockInRegion([&](const BlockPos& pos) {
                        auto localPos = pos - boundingBox.bpos1;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history->storeBlock(blockInstance, localPos);
                    });

                    long long i = 0;

                    std::string gMask = "";
                    if (mod.playerGMaskMap.find(xuid) !=
                        mod.playerGMaskMap.end()) {
                        gMask = mod.playerGMaskMap[xuid];
                    }

                    auto playerPos = origin.getPlayer()->getPosition();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    std::unordered_map<std::string, double> variables;

                    if (results["block"].isSet) {
                        auto* block = const_cast<Block*>(
                            results["block"].get<Block const*>());
                        region->forTopBlockInRegion([&](const BlockPos& pos) {
                            setFunction(variables, f, boundingBox, playerPos,
                                        pos);
                            if (gMask == "" ||
                                cpp_eval::eval<double>(gMask.c_str(), variables,
                                                       f) > 0.5) {
                                setBlockSimple(blockSource, pos, block);
                                i++;
                            }
                        });
                    } else {
                        auto bps = results["blockPattern"].get<std::string>();
                        BlockPattern blockPattern(bps);
                        region->forTopBlockInRegion([&](const BlockPos& pos) {
                            setFunction(variables, f, boundingBox, playerPos,
                                        pos);
                            if (gMask == "" ||
                                cpp_eval::eval<double>(gMask.c_str(), variables,
                                                       f) > 0.5) {
                                blockPattern.setBlock(variables, f, blockSource,
                                                      pos);
                                i++;
                            }
                        });
                    }
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
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) !=
                        mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto blockSource = Level::getBlockSource(dimID);

                    auto history = mod.getPlayerNextHistory(xuid);
                    *history = Clipboard(boundingBox.bpos2 - boundingBox.bpos1);
                    history->playerRelPos.x = dimID;
                    history->playerPos = boundingBox.bpos1;
                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        auto localPos = pos - boundingBox.bpos1;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history->storeBlock(blockInstance, localPos);
                    });

                    std::string gMask = "";
                    if (mod.playerGMaskMap.find(xuid) !=
                        mod.playerGMaskMap.end()) {
                        gMask = mod.playerGMaskMap[xuid];
                    }

                    auto playerPos = origin.getPlayer()->getPosition();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    std::unordered_map<std::string, double> variables;

                    region->forTopBlockInRegion([&](const BlockPos& posk) {
                        BlockPos pos(posk.x, posk.y - 1, posk.z);
                        if (&blockSource->getBlock(pos) ==
                                VanillaBlocks::mStone &&
                            region->contains(pos)) {
                            setFunction(variables, f, boundingBox, playerPos,
                                        pos);
                            if (gMask == "" ||
                                cpp_eval::eval<double>(gMask.c_str(), variables,
                                                       f) > 0.5) {
                                setBlockSimple(blockSource, pos,
                                               VanillaBlocks::mGrass);
                            }
                        }
                        for (int mY = -2; mY >= -4; mY--) {
                            BlockPos pos(posk.x, posk.y + mY, posk.z);
                            if (&blockSource->getBlock(pos) ==
                                    VanillaBlocks::mStone &&
                                region->contains(pos)) {
                                setFunction(variables, f, boundingBox,
                                            playerPos, pos);
                                if (gMask == "" ||
                                    cpp_eval::eval<double>(
                                        gMask.c_str(), variables, f) > 0.5) {
                                    setBlockSimple(blockSource, pos,
                                                   const_cast<Block*>(
                                                       VanillaBlocks::mDirt));
                                }
                            }
                        }
                    });
                    output.success(fmt::format("§aregion naturalized"));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "hollow",                 // command name
            "hollows out selection",  // command description
            {},
            {ParamData("num", ParamType::Int,true, "num"),
             ParamData("block", ParamType::Block,true, "block"),
             ParamData("blockPattern", ParamType::String,true, "blockPattern")},
            {{"num", "block"}, {"num", "blockPattern"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) !=
                        mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    int layer = 0;
                    if (results["num"].isSet) {
                        layer = results["num"].get<int>();
                    }

                    auto* region = mod.playerRegionMap[xuid];
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto blockSource = Level::getBlockSource(dimID);

                    auto history = mod.getPlayerNextHistory(xuid);
                    *history = Clipboard(boundingBox.bpos2 - boundingBox.bpos1);
                    history->playerRelPos.x = dimID;
                    history->playerPos = boundingBox.bpos1;
                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        auto localPos = pos - boundingBox.bpos1;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history->storeBlock(blockInstance, localPos);
                    });

                    std::string gMask = "";
                    if (mod.playerGMaskMap.find(xuid) !=
                        mod.playerGMaskMap.end()) {
                        gMask = mod.playerGMaskMap[xuid];
                    }

                    auto sizeDim = boundingBox.bpos2 - boundingBox.bpos1 + 3;

                    long long size = (sizeDim.x) * (sizeDim.y) * (sizeDim.z);

                    std::vector<bool> tmp(size, false);

                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        int counts = 0;
                        counts +=
                            &blockSource->getBlock(pos + BlockPos(0, 1, 0)) !=
                            BedrockBlocks::mAir;
                        counts +=
                            &blockSource->getBlock(pos + BlockPos(0, -1, 0)) !=
                            BedrockBlocks::mAir;
                        counts +=
                            &blockSource->getBlock(pos + BlockPos(1, 0, 0)) !=
                            BedrockBlocks::mAir;
                        counts +=
                            &blockSource->getBlock(pos + BlockPos(-1, 0, 0)) !=
                            BedrockBlocks::mAir;
                        counts +=
                            &blockSource->getBlock(pos + BlockPos(0, 0, 1)) !=
                            BedrockBlocks::mAir;
                        counts +=
                            &blockSource->getBlock(pos + BlockPos(0, 0, -1)) !=
                            BedrockBlocks::mAir;
                        if (counts == 6) {
                            auto localPos = pos - boundingBox.bpos1 + 1;
                            tmp[(localPos.y + sizeDim.y * localPos.z) *
                                    sizeDim.x +
                                localPos.x] = true;
                        }
                    });

                    for (int manhattan = 0; manhattan < layer; manhattan++) {
                        std::vector<bool> tmp2(tmp);
                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            int counts = 0;
                            auto localPos = pos - boundingBox.bpos1 + 1;
                            if (tmp[(localPos.y + sizeDim.y * localPos.z) *
                                        sizeDim.x +
                                    localPos.x]) {
                                auto calPos = localPos + BlockPos(0, 1, 0);
                                counts +=
                                    tmp[(calPos.y + sizeDim.y * calPos.z) *
                                            sizeDim.x +
                                        calPos.x];
                                calPos = localPos + BlockPos(0, -1, 0);
                                counts +=
                                    tmp[(calPos.y + sizeDim.y * calPos.z) *
                                            sizeDim.x +
                                        calPos.x];
                                calPos = localPos + BlockPos(0, 0, 1);
                                counts +=
                                    tmp[(calPos.y + sizeDim.y * calPos.z) *
                                            sizeDim.x +
                                        calPos.x];
                                calPos = localPos + BlockPos(0, 0, -1);
                                counts +=
                                    tmp[(calPos.y + sizeDim.y * calPos.z) *
                                            sizeDim.x +
                                        calPos.x];
                                calPos = localPos + BlockPos(1, 0, 0);
                                counts +=
                                    tmp[(calPos.y + sizeDim.y * calPos.z) *
                                            sizeDim.x +
                                        calPos.x];
                                calPos = localPos + BlockPos(-1, 0, 0);
                                counts +=
                                    tmp[(calPos.y + sizeDim.y * calPos.z) *
                                            sizeDim.x +
                                        calPos.x];
                                if (counts < 6) {
                                    tmp2[(localPos.y + sizeDim.y * localPos.z) *
                                             sizeDim.x +
                                         localPos.x] = false;
                                }
                            }
                        });
                        tmp.assign(tmp2.begin(), tmp2.end());
                    }

                    auto playerPos = origin.getPlayer()->getPosition();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    std::unordered_map<std::string, double> variables;

                    long long i = 0;

                    if (!results["blockPattern"].isSet) {
                        Block* block;
                        if (results["block"].isSet) {
                            block = const_cast<Block*>(
                                results["block"].get<Block const*>());
                        } else {
                            block = const_cast<Block*>(BedrockBlocks::mAir);
                        }
                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            auto localPos = pos - boundingBox.bpos1 + 1;
                            if (gMask == "" ||
                                cpp_eval::eval<double>(gMask.c_str(), variables,
                                                       f) > 0.5) {
                                auto localPos = pos - boundingBox.bpos1 + 1;
                                if (tmp[(localPos.y + sizeDim.y * localPos.z) *
                                            sizeDim.x +
                                        localPos.x]) {
                                    setBlockSimple(blockSource, pos, block);
                                    i++;
                                }
                            }
                        });
                    } else {
                        auto bps = results["blockPattern"].get<std::string>();
                        BlockPattern blockPattern(bps);
                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            auto localPos = pos - boundingBox.bpos1 + 1;
                            if (gMask == "" ||
                                cpp_eval::eval<double>(gMask.c_str(), variables,
                                                       f) > 0.5) {
                                auto localPos = pos - boundingBox.bpos1 + 1;
                                if (tmp[(localPos.y + sizeDim.y * localPos.z) *
                                            sizeDim.x +
                                        localPos.x]) {
                                    setFunction(variables, f, boundingBox,
                                                playerPos, pos);
                                    blockPattern.setBlock(variables, f,
                                                          blockSource, pos);
                                    i++;
                                }
                            }
                        });
                    }
                    output.success(fmt::format("§a{} block(s) hollowed", i));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit

#endif  // WORLDEDIT_REGIONOPERATIONCOMMAND_H