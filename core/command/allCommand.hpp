//
// Created by OEOTYAN on 2022/05/16.
//
#pragma once
#ifndef WORLDEDIT_ALLCOMMAND_H
#define WORLDEDIT_ALLCOMMAND_H
#include "store/BlockPattern.hpp"
#include "command/RegionCommand.hpp"
#include "command/HistoryCommand.hpp"
#include "command/HandToolCommand.hpp"
#include "command/ClipboardCommand.hpp"
#include "command/RegionInfoCommand.hpp"

namespace worldedit {
    // Direct setup of dynamic command with necessary information
    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;
    void commandsSetup() {
        regionCommandSetup();
        HistoryCommandSetup();
        HandToolCommandSetup();
        clipboardCommandSetup();
        regionInfoCommandSetup();

        DynamicCommand::setup(
            "gmask",            // command name
            "set global mask",  // command description
            {}, {ParamData("mask", ParamType::String, true, "mask")},
            {{"mask"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (results["mask"].isSet) {
                    auto str = results["mask"].getRaw<std::string>();
                    mod.playerGMaskMap[xuid] = str;
                    output.success(
                        fmt::format("§aglobal mask set to: §g{}", str));
                } else {
                    output.success(
                        fmt::format("§aYour global mask is cleared"));
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "set",        // command name
            "set block",  // command description
            {
                // enums{enumName, {values...}}
                // {"region",
                //  {"cuboid", "expand", "poly", "sphere", "convex", "clear"}},
            },
            {// parameters(type, name, [optional], [enumOptions(also
             // enumName)], [identifier]) identifier: used to identify
             // unique parameter data, if idnetifier is not set,
             //   it is set to be the same as enumOptions or name
             //   (identifier = enumOptions.empty() ? name:enumOptions)
             ParamData("block", ParamType::Block, "block"),
             ParamData("data", ParamType::Int, true, "data")},
            {
                // overloads{ (type == Enum ? enumOptions : name) ...}
                {"block", "data"},  // testenum <add|remove>
            },
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto blockname =
                    results["block"].get<Block const*>()->getTypeName();
                int data = 0;
                if (results["data"].isSet) {
                    data = results["data"].getRaw<int>();
                }
                auto block = Block::create(blockname, (unsigned short)data);
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
                    mod.playerRegionMap[xuid]->forEachBlockInRegion(
                        [&](const BlockPos& pos) {
                            auto blockInstance =
                                blockSource->getBlockInstance(pos);
                            if (blockInstance.hasContainer()) {
                                blockInstance.getContainer()->removeAllItems();
                            }
                            blockSource->setExtraBlock(pos,
                                                       *BedrockBlocks::mAir, 2);
                            blockSource->setBlock(pos, *block, 2, nullptr,
                                                  nullptr);
                            i++;
                        });
                    output.success(fmt::format("§a{} block(s) placed", i));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "gen",                     // command name
            "generate with function",  // command description
            {
                // enums{enumName, {values...}}
                {"pat", {"pat"}}
                // {"region",
                //  {"cuboid", "expand", "poly", "sphere", "convex", "clear"}},
            },
            {// parameters(type, name, [optional], [enumOptions(also
             // enumName)], [identifier]) identifier: used to identify
             // unique parameter data, if idnetifier is not set,
             //   it is set to be the same as enumOptions or name
             //   (identifier = enumOptions.empty() ? name:enumOptions)
             ParamData("function", ParamType::String, "function"),
             ParamData("block", ParamType::Block, "block"),
             ParamData("blockPattern", ParamType::String, "blockPattern"),
             ParamData("data", ParamType::Int, true, "data"),
             ParamData("datapattern", ParamType::String, "datapattern"),
             ParamData("pat", ParamType::Enum, "pat")},
            {{"block", "function", "data"},
             {"block", "function", "datapattern"},
             {"pat", "blockPattern", "function", "datapattern"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto xuid = origin.getPlayer()->getXuid();
                auto playerPos = origin.getPlayer()->getPosition();
                auto& mod = worldedit::getMod();
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

                    auto function = results["function"].get<std::string>();
                    std::string blockname = "minecraft:air";
                    if (results["block"].isSet) {
                        blockname =
                            results["block"].get<Block const*>()->getTypeName();
                    }
                    long long i = 0;
                    std::string s(function);
                    std::string origin(s);
                    std::string origin2 = "";
                    std::unordered_map<std::string, double> variables;
                    EvalFunctions f;
                    f.setbs(blockSource);
                    auto box = mod.playerRegionMap[xuid]->getBoundBox();
                    double lengthx = (box.bpos2.x - box.bpos1.x) * 0.5;
                    double lengthy = (box.bpos2.y - box.bpos1.y) * 0.5;
                    double lengthz = (box.bpos2.z - box.bpos1.z) * 0.5;
                    double centerx = (box.bpos2.x + box.bpos1.x) * 0.5;
                    double centery = (box.bpos2.y + box.bpos1.y) * 0.5;
                    double centerz = (box.bpos2.z + box.bpos1.z) * 0.5;
                    if (results["datapattern"].isSet) {
                        auto dataf = results["datapattern"].get<std::string>();
                        std::string s2(dataf);
                        origin2 = s2;
                        if (results["blockPattern"].isSet) {
                            auto idf = results["blockPattern"].get<std::string>();
                            std::string s3(idf);
                            BlockPattern blockPattern(idf);
                            mod.playerRegionMap[xuid]->forEachBlockInRegion(
                                [&](const BlockPos& pos) {
                                    f.setPos(pos);
                                    variables["x"] =
                                        (pos.x - centerx) / lengthx;
                                    variables["y"] =
                                        (pos.y - centery) / lengthy;
                                    variables["z"] =
                                        (pos.z - centerz) / lengthz;
                                    variables["rx"] = pos.x;
                                    variables["ry"] = pos.y;
                                    variables["rz"] = pos.z;
                                    variables["ox"] =
                                        pos.x - floor(playerPos.x);
                                    variables["oy"] =
                                        pos.y - floor(playerPos.y);
                                    variables["oz"] =
                                        pos.z - floor(playerPos.z);
                                    variables["cx"] = pos.x - box.bpos1.x;
                                    variables["cy"] = pos.y - box.bpos1.y;
                                    variables["cz"] = pos.z - box.bpos1.z;
                                    if (cpp_eval::eval<double>(
                                            s.c_str(), variables, f) > 0.5) {
                                        blockSource->setExtraBlock(
                                            pos, *BedrockBlocks::mAir, 2);
                                        auto blockInstance =
                                            blockSource->getBlockInstance(pos);
                                        if (blockInstance.hasContainer()) {
                                            blockInstance.getContainer()
                                                ->removeAllItems();
                                        }
                                        blockSource->setBlock(
                                            pos, *blockPattern.getBlock(variables,f), 2,
                                            nullptr, nullptr);
                                        i++;
                                    }
                                });
                        } else {
                            mod.playerRegionMap[xuid]->forEachBlockInRegion(
                                [&](const BlockPos& pos) {
                                    f.setPos(pos);
                                    variables["x"] =
                                        (pos.x - centerx) / lengthx;
                                    variables["y"] =
                                        (pos.y - centery) / lengthy;
                                    variables["z"] =
                                        (pos.z - centerz) / lengthz;
                                    variables["rx"] = pos.x;
                                    variables["ry"] = pos.y;
                                    variables["rz"] = pos.z;
                                    variables["ox"] =
                                        pos.x - floor(playerPos.x);
                                    variables["oy"] =
                                        pos.y - floor(playerPos.y);
                                    variables["oz"] =
                                        pos.z - floor(playerPos.z);
                                    variables["cx"] = pos.x - box.bpos1.x;
                                    variables["cy"] = pos.y - box.bpos1.y;
                                    variables["cz"] = pos.z - box.bpos1.z;
                                    if (cpp_eval::eval<double>(
                                            s.c_str(), variables, f) > 0.5) {
                                        blockSource->setExtraBlock(
                                            pos, *BedrockBlocks::mAir, 2);
                                        blockSource->setBlock(
                                            pos,
                                            *Block::create(
                                                blockname,
                                                (unsigned short)round(
                                                    cpp_eval::eval<double>(
                                                        s2.c_str(), variables,
                                                        f))),
                                            2, nullptr, nullptr);
                                        i++;
                                    }
                                });
                        }
                    } else {
                        int data = 0;
                        if (results["data"].isSet) {
                            data = results["data"].getRaw<int>();
                        }
                        auto block =
                            Block::create(blockname, (unsigned short)data);
                        auto blockSource = Level::getBlockSource(dimID);
                        mod.playerRegionMap[xuid]->forEachBlockInRegion(
                            [&](const BlockPos& pos) {
                                f.setPos(pos);
                                variables["x"] = (pos.x - centerx) / lengthx;
                                variables["y"] = (pos.y - centery) / lengthy;
                                variables["z"] = (pos.z - centerz) / lengthz;
                                if (cpp_eval::eval<double>(s.c_str(), variables,
                                                           f) > 0.5) {
                                    blockSource->setExtraBlock(
                                        pos, *BedrockBlocks::mAir, 2);
                                    blockSource->setBlock(pos, *block, 2,
                                                          nullptr, nullptr);
                                    i++;
                                }
                            });
                    }
                    worldedit::stringReplace(origin, "%", "%%");
                    worldedit::stringReplace(origin2, "%", "%%");
                    output.success(fmt::format(
                        "§gfunction:{}\n§gpattern:{}\n§a{} block(s) placed",
                        origin, origin2, i));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit

#endif  // WORLDEDIT_ALLCOMMAND_H