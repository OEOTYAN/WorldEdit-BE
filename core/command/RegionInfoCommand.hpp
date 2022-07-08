//
// Created by OEOTYAN on 2022/05/18.
//
#pragma once
#ifndef WORLDEDIT_REGIONINFOCOMMAND_H
#define WORLDEDIT_REGIONINFOCOMMAND_H
// #include "pch.h"
// #include <MC/Level.hpp>
#include <MC/Container.hpp>
#include <MC/ListTag.hpp>
// #include <MC/Block.hpp>
// #include <MC/BlockSource.hpp>
// #include <MC/Actor.hpp>
// #include <MC/Player.hpp>
// #include <MC/ServerPlayer.hpp>
// #include <MC/Dimension.hpp>
// #include <MC/ItemStack.hpp>
// #include "Version.h"
// #include "eval/Eval.h"
// #include "string/StringTool.h"
// #include <LLAPI.h>
// #include <ServerAPI.h>
// #include <EventAPI.h>
// #include <ScheduleAPI.h>
// #include <DynamicCommandAPI.h>
// #include "WorldEdit.h"
namespace worldedit {
    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;

    // size count distr

    void regionInfoCommandSetup() {
        DynamicCommand::setup(
            "size",                   // command name
            "calculate region size",  // command description
            {}, {ParamData("args", ParamType::SoftEnum, true, "-ca", "-ca")}, {{"args"}},
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
                    auto blockSource = Level::getBlockSource(dimID);
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
                            mod.playerRegionMap[xuid]->forEachBlockInRegion([&](const BlockPos& pos) { size++; });
                        } else {
                            auto dimID = mod.playerRegionMap[xuid]->getDimensionID();
                            mod.playerRegionMap[xuid]->forEachBlockInRegion([&](const BlockPos& pos) {
                                auto block = &blockSource->getBlock(pos);
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
            {ParamData("args", ParamType::SoftEnum, true, "-c", "-c"), ParamData("block", ParamType::Block, "block"),
             ParamData("data", ParamType::Int, true, "data")},
            {{"block", "data", "args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                int data = -1;
                auto blockname = results["block"].get<Block const*>()->getTypeName();
                if (results["data"].isSet) {
                    data = results["data"].getRaw<int>();
                }
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];
                    auto dimID = region->getDimensionID();
                    auto blockSource = Level::getBlockSource(dimID);
                    long long count = 0;
                    if (results["args"].isSet) {
                        // clipboard
                    } else {
                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            auto block = const_cast<Block*>(&blockSource->getBlock(pos));
                            if (block->getTypeName() == blockname) {
                                if (data == -1 || data == (int)(block->getTileData()))
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
            {}, {ParamData("args", ParamType::SoftEnum, true, "-cd", "-cd")}, {{"args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
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

                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];
                    auto dimID = region->getDimensionID();
                    auto blockSource = Level::getBlockSource(dimID);
                    std::unordered_map<std::string, long long> blocksMap;
                    std::vector<std::pair<std::string, long long>> blocksMap2;
                    blocksMap2.resize(0);
                    long long all = 0;
                    if (arg_c) {
                        region->forEachBlockInRegion([&](const BlockPos& pos) {
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
                                    if (count <= 0) {
                                        continue;
                                    all += count;
                                    std::string name;
                                    if (item->isBlock()) {
                                        auto block = const_cast<Block*>(item->getBlock());
                                        name = block->getTypeName();
                                        if (arg_d) {
                                            auto states = block->getNbt()->value().at("states").asCompoundTag()->toSNBT(
                                                0, SnbtFormat::Minimize);
                                            name += " [" + states.substr(1, states.length() - 2) + "]";
                                        }
                                    } else {
                                        name = item->getTypeName();
                                        if (arg_d) {
                                            name += " [";
                                            auto customName = item->getCustomName();
                                            if (customName != "") {
                                                name += "\"name\":\"" + customName+"\"";
                                            }
                                            name += "]";
                                        }
                                    }
                                    if (blocksMap.find(name) == blocksMap.end()) {
                                        blocksMap[name] = 0;
                                    }
                                    blocksMap[name] += count;

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
                    } else {
                        region->forEachBlockInRegion([&](const BlockPos& pos) {
                            all++;
                            auto block = const_cast<Block*>(&blockSource->getBlock(pos));
                            std::string blockName = block->getTypeName();

                            if (arg_d) {
                                auto states = block->getNbt()->value().at("states").asCompoundTag()->toSNBT(
                                    0, SnbtFormat::Minimize);
                                blockName += " [" + states.substr(1, states.length() - 2) + "]";
                            }
                            auto exBlock = const_cast<Block*>(&blockSource->getExtraBlock(pos));
                            if (!(exBlock == BedrockBlocks::mAir)) {
                                blockName += " & " + exBlock->getTypeName();

                                if (arg_d) {
                                    auto exStates = exBlock->getNbt()->value().at("states").asCompoundTag()->toSNBT(
                                        0, SnbtFormat::Minimize);
                                    blockName += +" [" + exStates.substr(1, exStates.length() - 2) + "]";
                                }
                            }
                            if (blocksMap.find(blockName) != blocksMap.end()) {
                                blocksMap[blockName] += 1;
                            } else {
                                blocksMap[blockName] = 1;
                            }
                        });
                    }
                    for (auto& block : blocksMap) {
                        std::string name = block.first;
                        worldedit::stringReplace(name, "minecraft:", "");
                        blocksMap2.emplace_back(std::pair<std::string, long long>(name, block.second));
                    }
                    std::sort(blocksMap2.begin(), blocksMap2.end(),
                              [](const std::pair<std::string, long long>& a,
                                 const std::pair<std::string, long long>& b) { return a.second > b.second; });
                    std::string res(fmt::format("§6total: §b{}", all));
                    for (auto& block : blocksMap2) {
                        res += fmt::format("\n§b{}      §6({}%%) §f{}", block.second,
                                           worldedit::fto_string(
                                               static_cast<double>(block.second) / static_cast<double>(all) * 100.0, 3),
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