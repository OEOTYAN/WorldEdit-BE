//
// Created by OEOTYAN on 2022/05/27.
//
#include "allCommand.hpp"
#include "store/BlockPattern.hpp"
#include "eval/blur.hpp"
#include "image/Image.h"
#include "eval/Bresenham.hpp"
#include "filesys/download.h"
#include "MC/StructureTemplate.hpp"
#include "MC/ItemInstance.hpp"
#include "MC/Container.hpp"
#include "MC/ItemStack.hpp"
#include "MC/CompoundTag.hpp"
#include "MC/ListTag.hpp"
#include "WorldEdit.h"

namespace worldedit {

    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;

    // set line rep center stack move gen walls faces overlay naturalize hollow
    // smooth image curve | deform

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
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];
                    auto center = region->getCenter();
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto blockSource = &player->getRegion();

                    if (mod.maxHistoryLength > 0) {
                        auto history = mod.getPlayerNextHistory(xuid);
                        *history = Clipboard(boundingBox.max - boundingBox.min);
                        history->playerRelPos.x = dimID;
                        history->playerPos = boundingBox.min;

                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            auto localPos = pos - boundingBox.min;
                            auto blockInstance = blockSource->getBlockInstance(pos);
                            history->storeBlock(blockInstance, localPos);
                        });
                    }

                    long long i = 0;

                    INNERIZE_GMASK

                    auto playerPos = player->getPosition();
                    auto playerRot = player->getRotation();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;
                    setFunction(variables, playerPos, playerRot);

                    std::string bps = "minecraft:air";
                    if (results["blockPattern"].isSet) {
                        bps = results["blockPattern"].get<std::string>();
                    } else if (results["block"].isSet) {
                        bps = results["block"].get<Block const*>()->getTypeName();
                    }
                    BlockPattern blockPattern(bps, xuid, region);

                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        setFunction(variables, f, boundingBox, playerPos, pos, center);
                        gMaskLambda(f, variables,
                                    [&]() mutable { i += blockPattern.setBlock(variables, f, blockSource, pos); });
                    });

                    output.success(fmt::format("§a{} block(s) placed", i));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "line",                                 // command name
            "Draws lines through selected points",  // command description
            {},
            {
                ParamData("block", ParamType::Block, "block"),
                ParamData("blockPattern", ParamType::String, "blockPattern"),
                ParamData("radius", ParamType::Int, true, "radius"),
                ParamData("args", ParamType::SoftEnum, true, "-h", "-h"),
            },
            {{"block", "radius", "args"}, {"blockPattern", "radius", "args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];

                    if (region->regionType != RegionType::CUBOID && region->regionType != CONVEX) {
                        output.error("You need to select a cuboid or convex region");
                        return;
                    }

                    auto center = region->getCenter();
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto blockSource = &player->getRegion();

                    auto radius = 0;
                    if (results["radius"].isSet) {
                        radius = results["radius"].get<int>();
                    }

                    boundingBox.min -= radius;
                    boundingBox.max += radius;

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

                    if (mod.maxHistoryLength > 0) {
                        auto history = mod.getPlayerNextHistory(xuid);
                        *history = Clipboard(boundingBox.max - boundingBox.min);
                        history->playerRelPos.x = dimID;
                        history->playerPos = boundingBox.min;

                        boundingBox.forEachBlockInBox([&](const BlockPos& pos) {
                            auto localPos = pos - boundingBox.min;
                            auto blockInstance = blockSource->getBlockInstance(pos);
                            history->storeBlock(blockInstance, localPos);
                        });
                    }

                    auto sizeDim = boundingBox.max - boundingBox.min + 3;

                    long long size = (sizeDim.x) * (sizeDim.y) * (sizeDim.z);
                    try {
                        std::vector<bool> tmp(size, false);
                        region->forEachLine([&](const BlockPos& pos1, const BlockPos& pos2) {
                            plotLine(pos1, pos2, [&](const BlockPos& pos) {
                                BoundingBox(pos - radius, pos + radius).forEachBlockInBox([&](const BlockPos& posk) {
                                    if ((pos - posk).length() <= 0.5 + radius) {
                                        auto localPos = posk - boundingBox.min + 1;
                                        tmp.at((localPos.y + sizeDim.y * localPos.z) * sizeDim.x + localPos.x) = true;
                                    }
                                });
                            });
                        });

                        if (arg_h) {
                            std::vector<bool> tmp2(size, false);
                            boundingBox.forEachBlockInBox([&](const BlockPos& pos) {
                                auto localPos = pos - boundingBox.min + 1;
                                if (tmp[(localPos.y + sizeDim.y * localPos.z) * sizeDim.x + localPos.x]) {
                                    int counts = 0;

                                    for (auto& calPos : localPos.getNeighbors()) {
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

                        long long i = 0;

                        INNERIZE_GMASK

                        auto playerPos = player->getPosition();
                        auto playerRot = player->getRotation();

                        EvalFunctions f;
                        f.setbs(blockSource);
                        f.setbox(boundingBox);
                        std::unordered_map<std::string, double> variables;
                        setFunction(variables, playerPos, playerRot);

                        std::string bps = "minecraft:air";
                        if (results["blockPattern"].isSet) {
                            bps = results["blockPattern"].get<std::string>();
                        } else if (results["block"].isSet) {
                            bps = results["block"].get<Block const*>()->getTypeName();
                        }
                        BlockPattern blockPattern(bps, xuid, region);

                        boundingBox.forEachBlockInBox([&](const BlockPos& pos) {
                            setFunction(variables, f, boundingBox, playerPos, pos, center);
                            gMaskLambda(f, variables, [&]() mutable {
                                auto localPos = pos - boundingBox.min + 1;
                                if (tmp[(localPos.y + sizeDim.y * localPos.z) * sizeDim.x + localPos.x]) {
                                    i += blockPattern.setBlock(variables, f, blockSource, pos);
                                }
                            });
                        });

                        output.success(fmt::format("§a{} block(s) placed", i));
                    } catch (std::bad_alloc) {
                        output.error("Out of memory");
                        return;
                    }

                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "curve",                                   // command name
            "Draws a spline through selected points",  // command description
            {},
            {
                ParamData("block", ParamType::Block, "block"),
                ParamData("blockPattern", ParamType::String, "blockPattern"),
                ParamData("radius", ParamType::Int, true, "radius"),
                ParamData("args", ParamType::SoftEnum, true, "-hcr", "-hcr"),
            },
            {{"block", "radius", "args"}, {"blockPattern", "radius", "args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];

                    if (region->regionType != RegionType::CONVEX && region->regionType != RegionType::LOFT) {
                        output.error("You need to select a convex or loft region");
                        return;
                    }

                    auto center = region->getCenter();
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto blockSource = &player->getRegion();

                    auto radius = 0;
                    if (results["radius"].isSet) {
                        radius = results["radius"].get<int>();
                    }
                    bool arg_h = false, arg_r = false, arg_c = false;
                    if (results["args"].isSet) {
                        auto str = results["args"].getRaw<std::string>();
                        if (str.find("-") == std::string::npos) {
                            output.error("wrong args");
                            return;
                        }
                        if (str.find("h") != std::string::npos) {
                            arg_h = true;
                        }
                        if (str.find("r") != std::string::npos) {
                            arg_r = true;
                        }
                        if (str.find("c") != std::string::npos) {
                            arg_c = true;
                        }
                    }

                    long long i = 0;

                    INNERIZE_GMASK

                    auto playerPos = player->getPosition();
                    auto playerRot = player->getRotation();

                    std::string bps = "minecraft:air";
                    if (results["blockPattern"].isSet) {
                        bps = results["blockPattern"].get<std::string>();
                    } else if (results["block"].isSet) {
                        bps = results["block"].get<Block const*>()->getTypeName();
                    }
                    BlockPattern blockPattern(bps, xuid, region);

                    EvalFunctions f;
                    f.setbs(blockSource);
                    std::unordered_map<std::string, double> variables;
                    setFunction(variables, playerPos, playerRot);

                    if (region->regionType == RegionType::LOFT) {
                        try {
                            if (mod.maxHistoryLength > 0) {
                                auto history = mod.getPlayerNextHistory(xuid);
                                *history = Clipboard(boundingBox.max - boundingBox.min);
                                history->playerRelPos.x = dimID;
                                history->playerPos = boundingBox.min;

                                boundingBox.forEachBlockInBox([&](const BlockPos& pos) {
                                    auto localPos = pos - boundingBox.min;
                                    auto blockInstance = blockSource->getBlockInstance(pos);
                                    history->storeBlock(blockInstance, localPos);
                                });
                            }
                            f.setbox(boundingBox);
                            auto* loft = static_cast<LoftRegion*>(region);
                            loft->forEachBlockInLines(radius, !arg_h, [&](const BlockPos& pos) {
                                setFunction(variables, f, boundingBox, playerPos, pos, center);
                                gMaskLambda(f, variables, [&]() mutable {
                                    i += blockPattern.setBlock(variables, f, blockSource, pos);
                                });
                            });

                        } catch (std::bad_alloc) {
                            output.error("Out of memory");
                            return;
                        }
                    } else {
                        std::vector<Node> nodes;
                        nodes.clear();
                        if (arg_r) {
                            for (auto& pos : static_cast<ConvexRegion*>(region)->poss) {
                                nodes.push_back(Node(pos, RNG::rand<double>() * 2 - 1, RNG::rand<double>() * 2 - 1,
                                                     RNG::rand<double>() * 2 - 1));
                            }
                        } else {
                            for (auto& pos : static_cast<ConvexRegion*>(region)->poss) {
                                nodes.push_back(Node(pos));
                            }
                        }
                        const double quality = 16;

                        auto interpol = KochanekBartelsInterpolation(nodes, arg_c);
                        const double splinelength = interpol.arcLength(0, 1);
                        const double step = 1.0 / splinelength / quality;
                        std::vector<Vec3> points;
                        points.clear();
                        for (double t = 0; t <= 1; t += step) {
                            auto pos = interpol.getPosition(t);
                            boundingBox = boundingBox.merge(pos.toBlockPos());
                            points.push_back(pos);
                        }
                        boundingBox.min -= radius + 1;
                        boundingBox.max += radius + 1;
                        try {
                            if (mod.maxHistoryLength > 0) {
                                auto history = mod.getPlayerNextHistory(xuid);
                                *history = Clipboard(boundingBox.max - boundingBox.min);
                                history->playerRelPos.x = dimID;
                                history->playerPos = boundingBox.min;

                                boundingBox.forEachBlockInBox([&](const BlockPos& pos) {
                                    auto localPos = pos - boundingBox.min;
                                    auto blockInstance = blockSource->getBlockInstance(pos);
                                    history->storeBlock(blockInstance, localPos);
                                });
                            }

                            auto sizeDim = boundingBox.max - boundingBox.min + 3;

                            long long size = (sizeDim.x) * (sizeDim.y) * (sizeDim.z);

                            std::vector<bool> tmp(size, false);
                            for (auto& pos : points) {
                                BoundingBox(pos.toBlockPos() - (radius + 1), pos.toBlockPos() + (radius + 1))
                                    .forEachBlockInBox([&](const BlockPos& posk) {
                                        if ((pos.toBlockPos() - posk).length() <= 0.5 + radius) {
                                            auto localPos = posk - boundingBox.min + 1;
                                            tmp.at((localPos.y + sizeDim.y * localPos.z) * sizeDim.x + localPos.x) =
                                                true;
                                        }
                                    });
                            }

                            if (arg_h) {
                                std::vector<bool> tmp2(size, false);
                                boundingBox.forEachBlockInBox([&](const BlockPos& pos) {
                                    auto localPos = pos - boundingBox.min + 1;
                                    if (tmp[(localPos.y + sizeDim.y * localPos.z) * sizeDim.x + localPos.x]) {
                                        int counts = 0;

                                        for (auto& calPos : localPos.getNeighbors()) {
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

                            f.setbox(boundingBox);
                            boundingBox.forEachBlockInBox([&](const BlockPos& pos) {
                                setFunction(variables, f, boundingBox, playerPos, pos, center);
                                gMaskLambda(f, variables, [&]() mutable {
                                    auto localPos = pos - boundingBox.min + 1;
                                    if (tmp[(localPos.y + sizeDim.y * localPos.z) * sizeDim.x + localPos.x]) {
                                        i += blockPattern.setBlock(variables, f, blockSource, pos);
                                    }
                                });
                            });
                        } catch (std::bad_alloc) {
                            output.error("Out of memory");
                            return;
                        }
                    }
                    output.success(fmt::format("§a{} block(s) placed", i));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "spike",                                 // command name
            "Make a spike through selected points",  // command description
            {},
            {
                ParamData("block", ParamType::Block, "block"),
                ParamData("blockPattern", ParamType::String, "blockPattern"),
                ParamData("radius1", ParamType::Int, false, "radius1"),
                ParamData("radius2", ParamType::Int, false, "radius2"),
                ParamData("radius3", ParamType::Int, true, "radius3"),
                ParamData("args", ParamType::SoftEnum, true, "-hcr", "-hcr"),
            },
            {
                {"block", "radius1", "radius2", "radius3", "args"},
                {"blockPattern", "radius1", "radius2", "radius3", "args"},
            },
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];

                    if (region->regionType != CONVEX) {
                        output.error("You need to select a convex region");
                        return;
                    }

                    auto center = region->getCenter();
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto blockSource = &player->getRegion();

                    auto radius1 = 0;
                    if (results["radius1"].isSet) {
                        radius1 = results["radius1"].get<int>();
                    }
                    auto radius2 = 0;
                    if (results["radius2"].isSet) {
                        radius2 = results["radius2"].get<int>();
                    }
                    auto radius3 = -1;
                    if (results["radius3"].isSet) {
                        radius3 = results["radius3"].get<int>();
                    }
                    auto radius = std::max(radius3, std::max(radius1, radius2));

                    auto getRadius = [&](double& t) {
                        if (radius3 >= 0) {
                            if (t < 0.5) {
                                return radius1 + 2 * t * (radius2 - radius1);
                            } else {
                                return radius2 + 2 * (t - 0.5) * (radius3 - radius2);
                            }
                        } else {
                            return radius1 + t * (radius2 - radius1);
                        }
                    };

                    bool arg_h = false, arg_r = false, arg_c = false;
                    if (results["args"].isSet) {
                        auto str = results["args"].getRaw<std::string>();
                        if (str.find("-") == std::string::npos) {
                            output.error("wrong args");
                            return;
                        }
                        if (str.find("h") != std::string::npos) {
                            arg_h = true;
                        }
                        if (str.find("r") != std::string::npos) {
                            arg_r = true;
                        }
                        if (str.find("c") != std::string::npos) {
                            arg_c = true;
                        }
                    }

                    std::vector<Node> nodes;
                    nodes.clear();
                    if (arg_r) {
                        for (auto& pos : static_cast<ConvexRegion*>(region)->poss) {
                            nodes.push_back(Node(pos, RNG::rand<double>() * 2 - 1, RNG::rand<double>() * 2 - 1,
                                                 RNG::rand<double>() * 2 - 1));
                        }
                    } else {
                        for (auto& pos : static_cast<ConvexRegion*>(region)->poss) {
                            nodes.push_back(Node(pos));
                        }
                    }
                    const double quality = 16;

                    auto interpol = KochanekBartelsInterpolation(nodes, arg_c);
                    const double splinelength = interpol.arcLength(0, 1);
                    const double step = 1.0 / splinelength / quality;
                    std::vector<Vec3> points;
                    points.clear();
                    std::vector<double> radiuses;
                    radiuses.clear();
                    for (double t = 0; t <= 1; t += step) {
                        auto pos = interpol.getPosition(t);
                        boundingBox = boundingBox.merge(pos.toBlockPos());
                        points.push_back(pos);
                        radiuses.push_back(std::max(getRadius(t), 0.1));
                    }
                    boundingBox.min -= radius + 1;
                    boundingBox.max += radius + 1;
                    try {
                        if (mod.maxHistoryLength > 0) {
                            auto history = mod.getPlayerNextHistory(xuid);
                            *history = Clipboard(boundingBox.max - boundingBox.min);
                            history->playerRelPos.x = dimID;
                            history->playerPos = boundingBox.min;

                            boundingBox.forEachBlockInBox([&](const BlockPos& pos) {
                                auto localPos = pos - boundingBox.min;
                                auto blockInstance = blockSource->getBlockInstance(pos);
                                history->storeBlock(blockInstance, localPos);
                            });
                        }

                        auto sizeDim = boundingBox.max - boundingBox.min + 3;

                        long long size = (sizeDim.x) * (sizeDim.y) * (sizeDim.z);

                        std::vector<bool> tmp(size, false);
                        int iter = 0;
                        for (auto& pos : points) {
                            BoundingBox(pos.toBlockPos() - (radius + 1), pos.toBlockPos() + (radius + 1))
                                .forEachBlockInBox([&](const BlockPos& posk) {
                                    if ((pos - (posk.toVec3() + 0.5)).length() <= 0.5 + radiuses[iter]) {
                                        auto localPos = posk - boundingBox.min + 1;
                                        tmp.at((localPos.y + sizeDim.y * localPos.z) * sizeDim.x + localPos.x) = true;
                                    }
                                });
                            iter++;
                        }

                        if (arg_h) {
                            std::vector<bool> tmp2(size, false);
                            boundingBox.forEachBlockInBox([&](const BlockPos& pos) {
                                auto localPos = pos - boundingBox.min + 1;
                                if (tmp[(localPos.y + sizeDim.y * localPos.z) * sizeDim.x + localPos.x]) {
                                    int counts = 0;

                                    for (auto& calPos : localPos.getNeighbors()) {
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

                        long long i = 0;

                        INNERIZE_GMASK

                        auto playerPos = player->getPosition();
                        auto playerRot = player->getRotation();

                        EvalFunctions f;
                        f.setbs(blockSource);
                        f.setbox(boundingBox);
                        std::unordered_map<std::string, double> variables;
                        setFunction(variables, playerPos, playerRot);

                        std::string bps = "minecraft:air";
                        if (results["blockPattern"].isSet) {
                            bps = results["blockPattern"].get<std::string>();
                        } else if (results["block"].isSet) {
                            bps = results["block"].get<Block const*>()->getTypeName();
                        }
                        BlockPattern blockPattern(bps, xuid, region);

                        boundingBox.forEachBlockInBox([&](const BlockPos& pos) {
                            setFunction(variables, f, boundingBox, playerPos, pos, center);
                            gMaskLambda(f, variables, [&]() mutable {
                                auto localPos = pos - boundingBox.min + 1;
                                if (tmp[(localPos.y + sizeDim.y * localPos.z) * sizeDim.x + localPos.x]) {
                                    i += blockPattern.setBlock(variables, f, blockSource, pos);
                                }
                            });
                        });

                        output.success(fmt::format("§a{} block(s) placed", i));
                    } catch (std::bad_alloc) {
                        output.error("Out of memory");
                        return;
                    }

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
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];
                    auto center = region->getCenter();
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto blockSource = &player->getRegion();

                    if (mod.maxHistoryLength > 0) {
                        auto history = mod.getPlayerNextHistory(xuid);
                        *history = Clipboard(boundingBox.max - boundingBox.min);
                        history->playerRelPos.x = dimID;
                        history->playerPos = boundingBox.min;

                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            auto localPos = pos - boundingBox.min;
                            auto blockInstance = blockSource->getBlockInstance(pos);
                            history->storeBlock(blockInstance, localPos);
                        });
                    }

                    long long i = 0;

                    INNERIZE_GMASK

                    auto playerPos = player->getPosition();
                    auto playerRot = player->getRotation();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;
                    setFunction(variables, playerPos, playerRot);

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
                        gMaskLambda(f, variables, [&]() mutable {
                            if (blockFilter.hasBlock(const_cast<Block*>(&blockSource->getBlock(pos)))) {
                                i += blockPattern.setBlock(variables, f, blockSource, pos);
                            }
                        });
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
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];
                    auto dimID = region->getDimensionID();
                    auto center = region->getCenter();
                    BoundingBox boundingBox;
                    boundingBox.min.x = static_cast<int>(floor(center.x - 0.49));
                    boundingBox.min.y = static_cast<int>(floor(center.y - 0.49));
                    boundingBox.min.z = static_cast<int>(floor(center.z - 0.49));
                    boundingBox.max.x = static_cast<int>(floor(center.x + 0.49));
                    boundingBox.max.y = static_cast<int>(floor(center.y + 0.49));
                    boundingBox.max.z = static_cast<int>(floor(center.z + 0.49));
                    auto blockSource = &player->getRegion();

                    if (mod.maxHistoryLength > 0) {
                        auto history = mod.getPlayerNextHistory(xuid);
                        *history = Clipboard(boundingBox.max - boundingBox.min);
                        history->playerRelPos.x = dimID;
                        history->playerPos = boundingBox.min;

                        boundingBox.forEachBlockInBox([&](const BlockPos& pos) {
                            auto localPos = pos - boundingBox.min;
                            auto blockInstance = blockSource->getBlockInstance(pos);
                            history->storeBlock(blockInstance, localPos);
                        });
                    }

                    auto playerPos = player->getPosition();
                    auto playerRot = player->getRotation();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;
                    setFunction(variables, playerPos, playerRot);

                    INNERIZE_GMASK

                    std::string bps = "minecraft:air";
                    if (results["blockPattern"].isSet) {
                        bps = results["blockPattern"].get<std::string>();
                    } else if (results["block"].isSet) {
                        bps = results["block"].get<Block const*>()->getTypeName();
                    }
                    BlockPattern blockPattern(bps, xuid, region);
                    boundingBox.forEachBlockInBox([&](const BlockPos& pos) {
                        setFunction(variables, f, boundingBox, playerPos, pos, center);

                        gMaskLambda(f, variables,
                                    [&]() mutable { blockPattern.setBlock(variables, f, blockSource, pos); });
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
            {ParamData("times", ParamType::Int, true, "times"), ParamData("dir", ParamType::Enum, true, "dir"),
             ParamData("args", ParamType::SoftEnum, true, "-sale", "-sale")},
            {{"times", "dir", "args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto times = 1;
                    if (results["times"].isSet) {
                        times = results["times"].get<int>();
                    }
                    bool arg_a = false, arg_s = false, arg_l = false, arg_e = false;
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
                        if (str.find("l") != std::string::npos) {
                            arg_l = true;
                        }
                        if (str.find("e") != std::string::npos) {
                            arg_e = true;
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

                    auto* region = mod.playerRegionMap[xuid];
                    auto center = region->getCenter();
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto boundingBoxLast = boundingBox;
                    auto blockSource = &player->getRegion();

                    BlockPos movingVec;
                    if (arg_l) {
                        movingVec = faceVec;
                    } else {
                        movingVec = faceVec * (boundingBox.max - boundingBox.min + 1);
                    }

                    boundingBoxLast.min = boundingBoxLast.min + movingVec * times;
                    boundingBoxLast.max = boundingBoxLast.max + movingVec * times;
                    auto boundingBoxHistory = boundingBoxLast.merge(boundingBox);

                    auto history = mod.getPlayerNextHistory(xuid);
                    *history = Clipboard(boundingBoxHistory.max - boundingBoxHistory.min);
                    history->playerRelPos.x = dimID;
                    history->playerPos = boundingBoxHistory.min;

                    boundingBoxHistory.forEachBlockInBox([&](const BlockPos& pos) {
                        auto localPos = pos - boundingBoxHistory.min;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history->storeBlock(blockInstance, localPos);
                    });

                    long long i = 0;

                    INNERIZE_GMASK

                    auto playerPos = player->getPosition();
                    auto playerRot = player->getRotation();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;
                    setFunction(variables, playerPos, playerRot);
                    region->forEachBlockInRegion([&](const BlockPos& posk) {
                        auto localPos = posk - boundingBoxHistory.min;
                        for (int j = 1; j <= times; ++j) {
                            auto pos = posk + movingVec * j;
                            setFunction(variables, f, boundingBox, playerPos, pos, center);
                            if ((arg_a && (&blockSource->getBlock(posk) != BedrockBlocks::mAir)) || !arg_a)
                                gMaskLambda(f, variables, [&]() mutable {
                                    auto& set = history->getSet(localPos);
                                    if (set.hasBlock)
                                        if ((arg_l && (set.block != BedrockBlocks::mAir)) || !arg_l) {
                                            i += set.setBlock(pos, blockSource);
                                        }
                                });
                        }
                    });
                    if (arg_e) {
                        auto st = StructureTemplate("worldedit_stack_cmd_tmp");
                        auto setting = StructureSettings();
                        setting.setIgnoreBlocks(true);
                        setting.setIgnoreEntities(false);
                        setting.setStructureSize(boundingBox.max - boundingBox.min + 1);
                        st.fillFromWorld(*blockSource, boundingBox.min + BlockPos(0, 1, 0), setting);
                        auto& palette = Global<Level>->getBlockPalette();
                        for (int j = 1; j <= times; ++j) {
                            auto pos = boundingBox.min + BlockPos(0, 1, 0) + movingVec * j;
                            st.placeInWorld(*blockSource, palette, pos, setting, nullptr, false);
                        }
                    }
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
            {ParamData("dis", ParamType::Int, true, "dis"), ParamData("dir", ParamType::Enum, true, "dir"),
             ParamData("args", ParamType::SoftEnum, true, "-sa", "-sa"),
             ParamData("block", ParamType::Block, true, "block"),
             ParamData("blockPattern", ParamType::String, true, "blockPattern")},
            {{"dis", "dir", "block", "args"}, {"dis", "dir", "blockPattern", "args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto dis = 1;
                    if (results["dis"].isSet) {
                        dis = results["dis"].get<int>();
                    }
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

                    auto* region = mod.playerRegionMap[xuid];
                    auto center = region->getCenter();
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto boundingBoxLast = boundingBox;
                    auto blockSource = &player->getRegion();

                    boundingBoxLast.min = boundingBoxLast.min + faceVec;
                    boundingBoxLast.max = boundingBoxLast.max + faceVec;
                    auto boundingBoxHistory = boundingBoxLast.merge(boundingBox);

                    auto history = mod.getPlayerNextHistory(xuid);
                    *history = Clipboard(boundingBoxHistory.max - boundingBoxHistory.min);
                    history->playerRelPos.x = dimID;
                    history->playerPos = boundingBoxHistory.min;

                    boundingBox.forEachBlockInBox([&](const BlockPos& pos) {
                        auto localPos = pos - boundingBoxHistory.min;
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

                    INNERIZE_GMASK

                    auto playerPos = player->getPosition();
                    auto playerRot = player->getRotation();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;
                    setFunction(variables, playerPos, playerRot);

                    std::string bps = "minecraft:air";
                    if (results["blockPattern"].isSet) {
                        bps = results["blockPattern"].get<std::string>();
                    } else if (results["block"].isSet) {
                        bps = results["block"].get<Block const*>()->getTypeName();
                    }
                    BlockPattern blockPattern(bps, xuid, region);
                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        setFunction(variables, f, boundingBox, playerPos, pos, center);
                        gMaskLambda(f, variables,
                                    [&]() mutable { blockPattern.setBlock(variables, f, blockSource, pos); });
                    });

                    region->forEachBlockInRegion([&](const BlockPos& posk) {
                        auto pos = posk + faceVec;
                        auto localPos = posk - boundingBoxHistory.min;
                        auto block = history->getSet(localPos).getBlock();
                        setFunction(variables, f, boundingBox, playerPos, pos, center);
                        if ((arg_a && (block != BedrockBlocks::mAir)) || !arg_a)
                            gMaskLambda(f, variables, [&]() mutable {
                                auto* set = &history->getSet(localPos);
                                if (set->hasBlock) {
                                    i += set->setBlock(pos, blockSource);
                                }
                            });
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
             ParamData("args", ParamType::SoftEnum, true, "-h", "-h")},
            {{"block", "function", "args"}, {"blockPattern", "function", "args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
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

                    auto* region = mod.playerRegionMap[xuid];
                    auto center = region->getCenter();
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto blockSource = &player->getRegion();

                    if (mod.maxHistoryLength > 0) {
                        auto history = mod.getPlayerNextHistory(xuid);
                        *history = Clipboard(boundingBox.max - boundingBox.min);
                        history->playerRelPos.x = dimID;
                        history->playerPos = boundingBox.min;

                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            auto localPos = pos - boundingBox.min;
                            auto blockInstance = blockSource->getBlockInstance(pos);
                            history->storeBlock(blockInstance, localPos);
                        });
                    }

                    long long i = 0;

                    INNERIZE_GMASK

                    auto genfunc = results["function"].get<std::string>();

                    auto playerPos = player->getPosition();
                    auto playerRot = player->getRotation();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;
                    setFunction(variables, playerPos, playerRot);

                    auto sizeDim = boundingBox.max - boundingBox.min + 3;

                    long long size = (sizeDim.x) * (sizeDim.y) * (sizeDim.z);

                    try {
                        std::vector<bool> tmp(size, false);
                        std::vector<bool> caled(size, false);

                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            setFunction(variables, f, boundingBox, playerPos, pos, center);
                            auto localPos = pos - boundingBox.min + 1;
                            if (cpp_eval::eval<double>(genfunc, variables, f) > 0.5) {
                                tmp[(localPos.y + sizeDim.y * localPos.z) * sizeDim.x + localPos.x] = true;
                            }
                            caled[(localPos.y + sizeDim.y * localPos.z) * sizeDim.x + localPos.x] = true;
                        });

                        if (arg_h) {
                            std::vector<bool> tmp2(size, false);
                            region->forEachBlockInRegion([&](const BlockPos& pos) {
                                auto localPos = pos - boundingBox.min + 1;
                                if (tmp[(localPos.y + sizeDim.y * localPos.z) * sizeDim.x + localPos.x]) {
                                    int counts = 0;

                                    for (auto& calPos : localPos.getNeighbors()) {
                                        if (!caled[(calPos.y + sizeDim.y * calPos.z) * sizeDim.x + calPos.x]) {
                                            setFunction(variables, f, boundingBox, playerPos,
                                                        calPos + boundingBox.min - 1, center);
                                            tmp[(calPos.y + sizeDim.y * calPos.z) * sizeDim.x + calPos.x] =
                                                cpp_eval::eval<double>(genfunc, variables, f) > 0.5;

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
                                gMaskLambda(f, variables, [&]() mutable {
                                    setFunction(variables, f, boundingBox, playerPos, pos, center);
                                    i += blockPattern.setBlock(variables, f, blockSource, pos);
                                });
                            }
                        });

                        output.success(fmt::format("§gfunction: {}\n§a{} block(s) placed", genfunc, i));
                    } catch (std::bad_alloc) {
                        output.error("Out of memory");
                        return;
                    }
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
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];
                    auto center = region->getCenter();
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto blockSource = &player->getRegion();

                    if (mod.maxHistoryLength > 0) {
                        auto history = mod.getPlayerNextHistory(xuid);
                        *history = Clipboard(boundingBox.max - boundingBox.min);
                        history->playerRelPos.x = dimID;
                        history->playerPos = boundingBox.min;

                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            auto localPos = pos - boundingBox.min;
                            auto blockInstance = blockSource->getBlockInstance(pos);
                            history->storeBlock(blockInstance, localPos);
                        });
                    }

                    long long i = 0;

                    INNERIZE_GMASK

                    auto playerPos = player->getPosition();
                    auto playerRot = player->getRotation();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;
                    setFunction(variables, playerPos, playerRot);

                    std::string bps = "minecraft:air";
                    if (results["blockPattern"].isSet) {
                        bps = results["blockPattern"].get<std::string>();
                    } else if (results["block"].isSet) {
                        bps = results["block"].get<Block const*>()->getTypeName();
                    }
                    BlockPattern blockPattern(bps, xuid, region);
                    if (region->regionType == RegionType::LOFT) {
                        auto* loft = static_cast<LoftRegion*>(region);
                        loft->forEachBlockInLines(2, true, [&](const BlockPos& pos) {
                            setFunction(variables, f, boundingBox, playerPos, pos, center);
                            gMaskLambda(f, variables,
                                        [&]() mutable { i += blockPattern.setBlock(variables, f, blockSource, pos); });
                        });
                        if (!(loft->circle)) {
                            loft->forEachBlockInLines(2, false, [&](const BlockPos& pos) {
                                setFunction(variables, f, boundingBox, playerPos, pos, center);
                                gMaskLambda(f, variables, [&]() mutable {
                                    i += blockPattern.setBlock(variables, f, blockSource, pos);
                                });
                            });
                        }
                    } else {
                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            setFunction(variables, f, boundingBox, playerPos, pos, center);
                            gMaskLambda(f, variables, [&]() mutable {
                                int counts = 0;
                                for (auto& calPos : pos.getNeighbors()) {
                                    if (calPos.y <= boundingBox.max.y && calPos.y >= boundingBox.min.y) {
                                        counts += region->contains(calPos);
                                    } else {
                                        counts += 1;
                                    }
                                }
                                if (counts < 6) {
                                    i += blockPattern.setBlock(variables, f, blockSource, pos);
                                }
                            });
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
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];
                    auto center = region->getCenter();
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto blockSource = &player->getRegion();

                    if (mod.maxHistoryLength > 0) {
                        auto history = mod.getPlayerNextHistory(xuid);
                        *history = Clipboard(boundingBox.max - boundingBox.min);
                        history->playerRelPos.x = dimID;
                        history->playerPos = boundingBox.min;

                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            auto localPos = pos - boundingBox.min;
                            auto blockInstance = blockSource->getBlockInstance(pos);
                            history->storeBlock(blockInstance, localPos);
                        });
                    }

                    long long i = 0;

                    INNERIZE_GMASK

                    auto playerPos = player->getPosition();
                    auto playerRot = player->getRotation();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;
                    setFunction(variables, playerPos, playerRot);
                    std::string bps = "minecraft:air";
                    if (results["blockPattern"].isSet) {
                        bps = results["blockPattern"].get<std::string>();
                    } else if (results["block"].isSet) {
                        bps = results["block"].get<Block const*>()->getTypeName();
                    }
                    BlockPattern blockPattern(bps, xuid, region);
                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        setFunction(variables, f, boundingBox, playerPos, pos, center);
                        gMaskLambda(f, variables, [&]() mutable {
                            int counts = 0;
                            for (auto& calPos : pos.getNeighbors()) {
                                counts += region->contains(calPos);
                            }
                            if (counts < 6) {
                                i += blockPattern.setBlock(variables, f, blockSource, pos);
                            }
                        });
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
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];
                    auto center = region->getCenter();
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto blockSource = &player->getRegion();

                    if (mod.maxHistoryLength > 0) {
                        auto history = mod.getPlayerNextHistory(xuid);
                        *history = Clipboard(boundingBox.max - boundingBox.min);
                        history->playerRelPos.x = dimID;
                        history->playerPos = boundingBox.min;

                        region->forTopBlockInRegion([&](const BlockPos& pos) {
                            auto localPos = pos - boundingBox.min;
                            auto blockInstance = blockSource->getBlockInstance(pos);
                            history->storeBlock(blockInstance, localPos);
                        });
                    }

                    long long i = 0;

                    INNERIZE_GMASK

                    auto playerPos = player->getPosition();
                    auto playerRot = player->getRotation();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;
                    setFunction(variables, playerPos, playerRot);
                    std::string bps = "minecraft:air";
                    if (results["blockPattern"].isSet) {
                        bps = results["blockPattern"].get<std::string>();
                    } else if (results["block"].isSet) {
                        bps = results["block"].get<Block const*>()->getTypeName();
                    }
                    BlockPattern blockPattern(bps, xuid, region);
                    region->forTopBlockInRegion([&](const BlockPos& pos) {
                        setFunction(variables, f, boundingBox, playerPos, pos, center);
                        gMaskLambda(f, variables,
                                    [&]() mutable { i += blockPattern.setBlock(variables, f, blockSource, pos); });
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
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];
                    auto center = region->getCenter();
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto blockSource = &player->getRegion();

                    if (mod.maxHistoryLength > 0) {
                        auto history = mod.getPlayerNextHistory(xuid);
                        *history = Clipboard(boundingBox.max - boundingBox.min);
                        history->playerRelPos.x = dimID;
                        history->playerPos = boundingBox.min;

                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            auto localPos = pos - boundingBox.min;
                            auto blockInstance = blockSource->getBlockInstance(pos);
                            history->storeBlock(blockInstance, localPos);
                        });
                    }

                    INNERIZE_GMASK

                    auto playerPos = player->getPosition();
                    auto playerRot = player->getRotation();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;
                    setFunction(variables, playerPos, playerRot);

                    region->forTopBlockInRegion([&](const BlockPos& posk) {
                        BlockPos pos(posk.x, posk.y - 1, posk.z);
                        if (&blockSource->getBlock(pos) == StaticVanillaBlocks::mStone && region->contains(pos)) {
                            setFunction(variables, f, boundingBox, playerPos, pos, center);
                            gMaskLambda(f, variables, [&]() mutable {
                                setBlockSimple(blockSource, pos, const_cast<Block*>(StaticVanillaBlocks::mGrass));
                            });
                        }
                        for (int mY = -2; mY >= -4; mY--) {
                            BlockPos pos(posk.x, posk.y + mY, posk.z);
                            if (&blockSource->getBlock(pos) == StaticVanillaBlocks::mStone && region->contains(pos)) {
                                setFunction(variables, f, boundingBox, playerPos, pos, center);
                                gMaskLambda(f, variables, [&]() mutable {
                                    setBlockSimple(blockSource, pos, const_cast<Block*>(StaticVanillaBlocks::mDirt));
                                });
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
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    int layer = 0;
                    if (results["num"].isSet) {
                        layer = results["num"].get<int>();
                    }

                    auto* region = mod.playerRegionMap[xuid];
                    auto center = region->getCenter();
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto blockSource = &player->getRegion();

                    if (mod.maxHistoryLength > 0) {
                        auto history = mod.getPlayerNextHistory(xuid);
                        *history = Clipboard(boundingBox.max - boundingBox.min);
                        history->playerRelPos.x = dimID;
                        history->playerPos = boundingBox.min;

                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            auto localPos = pos - boundingBox.min;
                            auto blockInstance = blockSource->getBlockInstance(pos);
                            history->storeBlock(blockInstance, localPos);
                        });
                    }

                    INNERIZE_GMASK

                    auto sizeDim = boundingBox.max - boundingBox.min + 3;

                    long long size = (sizeDim.x) * (sizeDim.y) * (sizeDim.z);

                    try {
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
                                int counts = 0;
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

                        auto playerPos = player->getPosition();
                        auto playerRot = player->getRotation();
                        EvalFunctions f;
                        f.setbs(blockSource);
                        f.setbox(boundingBox);
                        std::unordered_map<std::string, double> variables;
                        setFunction(variables, playerPos, playerRot);

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
                            gMaskLambda(f, variables, [&]() mutable {
                                auto localPos = pos - boundingBox.min + 1;
                                if (tmp[(localPos.y + sizeDim.y * localPos.z) * sizeDim.x + localPos.x]) {
                                    i += blockPattern.setBlock(variables, f, blockSource, pos);
                                }
                            });
                        });
                        output.success(fmt::format("§a{} block(s) hollowed", i));
                    } catch (std::bad_alloc) {
                        output.error("Out of memory");
                        return;
                    }
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "smooth",            // command name
            "smooth selection",  // command description
            {}, {ParamData("num", ParamType::Int, true, "num"), ParamData("mask", ParamType::String, true, "mask")},
            {{"num", "mask"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    int ksize = 5;
                    if (results["num"].isSet) {
                        ksize = results["num"].get<int>();
                    }

                    auto* region = mod.playerRegionMap[xuid];
                    auto center = region->getCenter();
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto blockSource = &player->getRegion();

                    if (mod.maxHistoryLength > 0) {
                        auto history = mod.getPlayerNextHistory(xuid);
                        *history = Clipboard(boundingBox.max - boundingBox.min);
                        history->playerRelPos.x = dimID;
                        history->playerPos = boundingBox.min;

                        boundingBox.forEachBlockInBox([&](const BlockPos& pos) {
                            auto localPos = pos - boundingBox.min;
                            auto blockInstance = blockSource->getBlockInstance(pos);
                            history->storeBlock(blockInstance, localPos);
                        });
                    }

                    std::string mask = "";
                    if (results["mask"].isSet) {
                        mask = results["mask"].get<std::string>();
                    }

                    auto heightMap = region->getHeightMap(mask);
                    int sizex = boundingBox.max.x - boundingBox.min.x + 1;
                    int sizez = boundingBox.max.z - boundingBox.min.z + 1;

                    auto smoothedHeightMap = blur2D(heightMap, ksize, sizex, sizez);
                    region->applyHeightMap(smoothedHeightMap, mask);

                    output.success(fmt::format("§aRegion smoothed"));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "exportstr",              // command name
            "export as mcstructure",  // command description
            {},
            {
                ParamData("strname", ParamType::String, false, "strname"),
                ParamData("args", ParamType::SoftEnum, true, "-l", "-l"),
            },
            {{"strname", "args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];
                    auto center = region->getCenter();
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto blockSource = &player->getRegion();

                    std::string filename;
                    filename = results["strname"].get<std::string>();

                    auto st = StructureTemplate(filename);
                    auto setting = StructureSettings();
                    setting.setIgnoreBlocks(false);
                    setting.setIgnoreEntities(false);
                    setting.setStructureSize(boundingBox.max - boundingBox.min + 1);
                    st.fillFromWorld(*blockSource, boundingBox.min + BlockPos(0, 1, 0), setting);
                    auto structure = st.toTag()->toBinaryNBT();

                    std::ofstream outStr(WE_DIR + "structures/" + filename + ".mcstructure",
                                         std::ios::out | std::ios::binary);
                    outStr << structure;
                    outStr.close();
                    output.success(fmt::format("§aStructure exported"));

                    bool arg_l = false;
                    if (results["args"].isSet) {
                        auto str = results["args"].getRaw<std::string>();
                        if (str.find("-") == std::string::npos) {
                            output.error("wrong args");
                            return;
                        }
                        if (str.find("l") != std::string::npos) {
                            arg_l = true;
                        }
                    }

                    if (arg_l) {
                        std::unordered_map<std::string, long long> blocksMap;
                        std::vector<std::pair<std::string, long long>> blocksMap2;
                        blocksMap2.resize(0);
                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            auto block = const_cast<Block*>(&blockSource->getBlock(pos));
                            std::string blockName;
                            if (!(block == BedrockBlocks::mAir)) {
                                blockName = block->getTypeName();

                                auto states = block->getNbt()->value().at("states").asCompoundTag()->toSNBT(
                                    0, SnbtFormat::Minimize);
                                blockName += " [" + states.substr(1, states.length() - 2) + "]";
                            }

                            auto exBlock = const_cast<Block*>(&blockSource->getExtraBlock(pos));
                            std::string exBlockName;
                            if (!(exBlock == BedrockBlocks::mAir)) {
                                exBlockName = exBlock->getTypeName();

                                auto exStates = exBlock->getNbt()->value().at("states").asCompoundTag()->toSNBT(
                                    0, SnbtFormat::Minimize);
                                exBlockName += " [" + exStates.substr(1, exStates.length() - 2) + "]";
                            }
                            if (blockName != "") {
                                if (blocksMap.find(blockName) != blocksMap.end()) {
                                    blocksMap[blockName] += 1;
                                } else {
                                    blocksMap[blockName] = 1;
                                }
                            }
                            if (exBlockName != "") {
                                if (blocksMap.find(exBlockName) != blocksMap.end()) {
                                    blocksMap[exBlockName] += 1;
                                } else {
                                    blocksMap[exBlockName] = 1;
                                }
                            }
                            auto blockInstance = blockSource->getBlockInstance(pos);

                            if (blockInstance.hasContainer()) {
                                auto container = blockInstance.getContainer();
                                std::queue<std::unique_ptr<CompoundTag>> itemQueue;
                                for (auto& item : container->getAllSlots()) {
                                    if (item->getCount() > 0) {
                                        itemQueue.emplace(const_cast<ItemStack*>(item)->getNbt());
                                    }
                                }
                                while (!itemQueue.empty()) {
                                    auto* item = ItemStack::create(std::move(itemQueue.front()));
                                    itemQueue.pop();
                                    auto count = item->getCount();
                                    if (count > 0) {
                                        std::string name;
                                        if (item->isBlock()) {
                                            auto block = const_cast<Block*>(item->getBlock());
                                            name = block->getTypeName();

                                            auto states = block->getNbt()->value().at("states").asCompoundTag()->toSNBT(
                                                0, SnbtFormat::Minimize);
                                            name += " [" + states.substr(1, states.length() - 2) + "]";

                                        } else {
                                            name = item->getTypeName();

                                            name += " [";
                                            auto customName = item->getCustomName();
                                            if (customName != "") {
                                                name += "\"name\":\"" + customName + "\"";
                                            }
                                            name += "]";
                                        }
                                        if (blocksMap.find(name) == blocksMap.end()) {
                                            blocksMap[name] = count;
                                        } else {
                                            blocksMap[name] += count;
                                        }

                                        auto iNbt = item->getNbt();
                                        auto* vmap = &iNbt->value();
                                        if (vmap->find("tag") != vmap->end()) {
                                            auto* imap = &vmap->at("tag").asCompoundTag()->value();
                                            if (imap->find("Items") != imap->end()) {
                                                auto* cmap = &imap->at("Items").asListTag()->value();
                                                for (auto& mItem : *cmap) {
                                                    itemQueue.emplace(mItem->asCompoundTag()->clone());
                                                }
                                            }
                                        }
                                    }
                                    delete item;
                                }
                            }
                        });

                        for (auto& block : blocksMap) {
                            std::string name = block.first;
                            blocksMap2.emplace_back(std::pair<std::string, long long>(name, block.second));
                        }
                        std::sort(blocksMap2.begin(), blocksMap2.end(),
                                  [](const std::pair<std::string, long long>& a,
                                     const std::pair<std::string, long long>& b) { return a.second > b.second; });
                        std::ofstream outStr(WE_DIR + "structures/" + filename + "_list.txt", std::ios::out);
                        for (auto& block : blocksMap2) {
                            outStr << fmt::format("{} {}\n", block.second, block.first);
                        }
                        outStr.close();
                        output.success(fmt::format("§aMaterial list exported"));
                    }
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "image",      // command name
            "set image",  // command description
            {
                {"fliptype", {"none", "flipu", "flipv", "flipuv"}},
                {"rotation", {"none", "rotate90", "rotate180", "rotate270"}},
                {"file", {"file"}},
                {"link", {"link"}},
            },
            {
                ParamData("filename", ParamType::SoftEnum, "filename"),
                ParamData("url", ParamType::String, "url"),
                ParamData("fliptype", ParamType::Enum, true, "fliptype"),
                ParamData("rotation", ParamType::Enum, true, "rotation"),
                ParamData("file", ParamType::Enum, "file"),
                ParamData("link", ParamType::Enum, "link"),
            },
            {
                {"file", "filename", "fliptype", "rotation"},
                {"link", "url", "fliptype", "rotation"},
            },
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];
                    auto center = region->getCenter();
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto size = boundingBox.max - boundingBox.min;
                    auto blockSource = &player->getRegion();

                    // if (region->regionType != RegionType::LOFT && size.x != 0 && size.y != 0 && size.z != 0) {
                    //     output.error("You need to select a slice");
                    //     return;
                    // }

                    int flipInt = 0;

                    if (results["fliptype"].isSet) {
                        auto fliptype = results["fliptype"].getRaw<std::string>();
                        if (fliptype == "flipu") {
                            flipInt = 1;
                        } else if (fliptype == "flipv") {
                            flipInt = 2;
                        } else if (fliptype == "flipuv") {
                            flipInt = 3;
                        }
                    }

                    int rotationInt = 0;

                    if (results["rotation"].isSet) {
                        auto rotation = results["rotation"].getRaw<std::string>();
                        if (rotation == "rotate90") {
                            rotationInt = 1;
                        } else if (rotation == "rotate180") {
                            rotationInt = 2;
                        } else if (rotation == "rotate270") {
                            rotationInt = 3;
                        }
                    }

                    if (mod.maxHistoryLength > 0) {
                        auto history = mod.getPlayerNextHistory(xuid);
                        *history = Clipboard(boundingBox.max - boundingBox.min);
                        history->playerRelPos.x = dimID;
                        history->playerPos = boundingBox.min;

                        boundingBox.forEachBlockInBox([&](const BlockPos& pos) {
                            auto localPos = pos - boundingBox.min;
                            auto blockInstance = blockSource->getBlockInstance(pos);
                            history->storeBlock(blockInstance, localPos);
                        });
                    }

                    INNERIZE_GMASK

                    auto blockColorMap = getBlockColorMap();

                    auto playerPos = player->getPosition();
                    auto playerRot = player->getRotation();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;
                    setFunction(variables, playerPos, playerRot);

                    long long i = 0;

                    std::string filename;
                    if (results["filename"].isSet) {
                        filename = results["filename"].get<std::string>();

                        if (filename.find(".png") == std::string::npos) {
                            filename += ".png";
                        }

                        filename = WE_DIR + "image/" + filename;
                    } else /* if (results["link"].isSet)*/ {
                        if (downloadImage(results["url"].get<std::string>())) {
                            filename = WE_DIR + "imgtemp/0.png";
                        } else {
                            output.error("Failed to download image");
                            return;
                        }
                    }

                    auto texture2D = loadpng(filename);

                    Sampler sampler(SamplerType::Bilinear, EdgeType::CLAMP);

                    std::function<void(double&, double&)> rotate;

                    switch (rotationInt) {
                        case 1:
                            rotate = [&](double& u, double& v) {
                                std::swap(u, v);
                                u = 1 - u;
                            };
                            break;
                        case 2:
                            rotate = [&](double& u, double& v) {
                                u = 1 - u;
                                v = 1 - v;
                            };
                            break;
                        case 3:
                            rotate = [&](double& u, double& v) {
                                std::swap(u, v);
                                v = 1 - v;
                            };
                            break;
                        default:
                            rotate = [&](double& u, double& v) {};
                    }

                    region->forEachBlockUVInRegion([&](const BlockPos& pos, double u, double v) {
                        setFunction(variables, f, boundingBox, playerPos, pos, center);
                        gMaskLambda(f, variables, [&]() mutable {
                            if (flipInt != 3) {
                                if (flipInt != 1) {
                                    u = 1 - u;
                                }
                                if (flipInt != 2) {
                                    v = 1 - v;
                                }
                            }

                            rotate(u, v);

                            auto color = texture2D.sample(sampler, u, v);

                            double minDist = DBL_MAX;
                            int minTile = 15;
                            for (auto& i : blockColorMap) {
                                auto dst = i.first.distanceTo(color);
                                if (dst < minDist) {
                                    minDist = dst;
                                    minTile = i.second;
                                }
                            }
                            if (RNG::rand<double>() <= color.a) {
                                setBlockSimple(blockSource, pos, Block::create("minecraft:concrete", minTile));
                                ++i;
                            }
                        });
                    });

                    output.success(fmt::format("§apng setted"));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "removeitem",             // command name
            "remove item in region",  // command description
            {},
            {ParamData("item", ParamType::Item, "item"), ParamData("num", ParamType::Int, true, "num"),
             ParamData("data", ParamType::Int, true, "data")},
            {{"item", "num", "data"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];
                    auto center = region->getCenter();
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto blockSource = &player->getRegion();

                    if (mod.maxHistoryLength > 0) {
                        auto history = mod.getPlayerNextHistory(xuid);
                        *history = Clipboard(boundingBox.max - boundingBox.min);
                        history->playerRelPos.x = dimID;
                        history->playerPos = boundingBox.min;

                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            auto localPos = pos - boundingBox.min;
                            auto blockInstance = blockSource->getBlockInstance(pos);
                            history->storeBlock(blockInstance, localPos);
                        });
                    }
                    int num = -1;
                    if (results["num"].isSet) {
                        num = results["num"].get<int>();
                    }
                    int data = -2144000000;
                    if (results["data"].isSet) {
                        data = results["data"].get<int>();
                    }
                    long long i = 0;

                    INNERIZE_GMASK

                    auto playerPos = player->getPosition();
                    auto playerRot = player->getRotation();
                    EvalFunctions f;
                    f.setbs(blockSource);
                    f.setbox(boundingBox);
                    std::unordered_map<std::string, double> variables;
                    setFunction(variables, playerPos, playerRot);

                    auto cmdItem = results["item"]
                                       .getRaw<CommandItem>()
                                       .createInstance(1, data, nullptr, true)
                                       .value_or(ItemInstance::EMPTY_ITEM);
                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        setFunction(variables, f, boundingBox, playerPos, pos, center);
                        gMaskLambda(f, variables, [&]() mutable {
                            auto blockInstance = blockSource->getBlockInstance(pos);
                            if (blockInstance.hasContainer()) {
                                auto container = blockInstance.getContainer();
                                for (auto& constItem : container->getAllSlots()) {
                                    auto item = const_cast<ItemStack*>(constItem);
                                    int count = item->getCount();
                                    if (count <= 0) {
                                        continue;
                                    }
                                    auto iNbt = item->getNbt();
                                    auto* vmap = &iNbt->value();
                                    if (vmap->find("tag") != vmap->end()) {
                                        if (count != 1) {
                                            continue;
                                        }
                                        auto* imap = &vmap->at("tag").asCompoundTag()->value();
                                        if (imap->find("Items") != imap->end()) {
                                            auto* cmap = &imap->at("Items").asListTag()->value();
                                            std::vector<Tag*> afterVal;
                                            afterVal.clear();
                                            for (auto& mItem : *cmap) {
                                                auto* shulkItem = ItemStack::create(mItem->asCompoundTag()->clone());
                                                int count2 = shulkItem->getCount();

                                                if (count2 <= 0 || !(shulkItem->sameItem(cmdItem)) ||
                                                    (data >= -2140000000 && data != shulkItem->getAuxValue())) {
                                                    continue;
                                                }
                                                int removeNum2 = num > 0 ? std::min(num, count2) : count2;
                                                i += removeNum2;
                                                if (num > 0) {
                                                    num -= removeNum2;
                                                }
                                                if (removeNum2 == count2) {
                                                    continue;
                                                }
                                                shulkItem->remove(removeNum2);
                                                afterVal.push_back(shulkItem->getNbt().get());
                                                delete shulkItem;
                                            }
                                            if (afterVal.size() < 1) {
                                                imap->erase("Items");
                                                if (imap->size() < 1) {
                                                    vmap->erase("tag");
                                                }
                                            } else {
                                                *cmap = afterVal;
                                            }
                                            item->setNbt(iNbt.get());
                                            continue;
                                        }
                                    }

                                    if (!(item->sameItem(cmdItem)) ||
                                        (data >= -2140000000 && data != item->getAuxValue())) {
                                        continue;
                                    }

                                    int removeNum = num > 0 ? std::min(num, count) : count;
                                    item->remove(removeNum);
                                    i += removeNum;
                                    if (num > 0) {
                                        num -= removeNum;
                                    }
                                    if (num == 0) {
                                        return;
                                    }
                                }
                            }
                        });
                    });

                    output.success(fmt::format("§a{} item(s) removed", i));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit