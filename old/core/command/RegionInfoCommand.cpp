#include "WorldEdit.h"
#include "allCommand.hpp"
#include "mc/ItemStack.hpp"
#include "region/Regions.h"
#include "utils/StringTool.h"
#include <mc/BlockActorDataPacket.hpp>
#include <mc/CommandBlockNameResult.hpp>
#include <mc/Container.hpp>
#include <mc/Dimension.hpp>
#include <mc/ListTag.hpp>
namespace we {
using ParamType = DynamicCommand::ParameterType;
using ParamData = DynamicCommand::ParameterData;

// resendpack size count distr

void regionInfoCommandSetup() {
    DynamicCommand::setup(
        "resendpack",                                   // command name
        tr("worldedit.command.description.resendpack"), // command description
        {},
        {},
        {{}},
        // dynamic command callback
        [](DynamicCommand const&                                    command,
           CommandOrigin const&                                     origin,
           CommandOutput&                                           output,
           std::unordered_map<std::string, DynamicCommand::Result>& results) {
            auto player = origin.getPlayer();
            if (player == nullptr) {
                output.trError("worldedit.error.noplayer");
                return;
            }
            auto  xuid       = player->getXuid();
            auto& playerData = getPlayersData(xuid);
            if (playerData.region != nullptr && playerData.region->hasSelected()) {
                auto& region      = playerData.region;
                auto  dimID       = region->getDimensionID();
                auto  blockSource = &player->getDimensionBlockSource();

                region->forEachBlockInRegion([&](BlockPos const& pos) {
                    auto exblock = &blockSource->getExtraBlock(pos);
                    auto block   = &blockSource->getBlock(pos);
                    auto be      = blockSource->getBlockEntity(pos);
                    std::unique_ptr<BlockActorDataPacket> bepacket = nullptr;
                    if (be) {
                        bepacket = be->getServerUpdatePacket(*blockSource);
                    }

                    blockSource->getDimension().forEachPlayer(
                        [&](Player& player) -> bool {
                            player.sendUpdateBlockPacket(
                                pos,
                                *exblock,
                                UpdateBlockFlags::BlockUpdateAll,
                                UpdateBlockLayer::UpdateBlockLiquid
                            );
                            player.sendUpdateBlockPacket(
                                pos,
                                *block,
                                UpdateBlockFlags::BlockUpdateAll,
                                UpdateBlockLayer::UpdateBlockDefault
                            );
                            if (bepacket != nullptr) {
                                player.sendNetworkPacket(*(bepacket.get()));
                            }
                            player.sendUpdateBlockPacket(
                                pos,
                                *exblock,
                                UpdateBlockFlags::BlockUpdateAll,
                                UpdateBlockLayer::UpdateBlockLiquid
                            );
                            return true;
                        }
                    );
                });

                output.trSuccess("worldedit.resendpack.success");
            } else {
                output.trError("worldedit.error.incomplete-region");
            }
        },
        CommandPermissionLevel::GameMasters
    );

    DynamicCommand::setup(
        "size",                                   // command name
        tr("worldedit.command.description.size"), // command description
        {},
        {ParamData("args", ParamType::SoftEnum, true, "-ca", "-ca")},
        {{"args"}},
        // dynamic command callback
        [](DynamicCommand const&                                    command,
           CommandOrigin const&                                     origin,
           CommandOutput&                                           output,
           std::unordered_map<std::string, DynamicCommand::Result>& results) {
            auto player = origin.getPlayer();
            if (player == nullptr) {
                output.trError("worldedit.error.noplayer");
                return;
            }
            auto  xuid       = player->getXuid();
            auto& playerData = getPlayersData(xuid);
            if (playerData.region != nullptr && playerData.region->hasSelected()) {
                auto&     region      = playerData.region;
                auto      dimID       = region->getDimensionID();
                auto      blockSource = &player->getDimensionBlockSource();
                long long size        = 0;
                bool      arg_a = false, arg_c = false;
                if (results["args"].isSet) {
                    auto str = results["args"].getRaw<std::string>();
                    if (str.find("-") == std::string::npos) {
                        output.trError("worldedit.command.error.args", str);
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
                        playerData.region->forEachBlockInRegion([&](BlockPos const& pos) {
                            size++;
                        });
                    } else {
                        auto dimID = playerData.region->getDimensionID();
                        playerData.region->forEachBlockInRegion([&](BlockPos const& pos) {
                            auto block = &blockSource->getBlock(pos);
                            if (!(block == BedrockBlocks::mAir)) size++;
                        });
                    }
                } else {
                    // clipboard
                }
                output.trSuccess("worldedit.size.success", size);
            } else {
                output.trError("worldedit.error.incomplete-region");
            }
        },
        CommandPermissionLevel::GameMasters
    );

    DynamicCommand::setup(
        "count",                                   // command name
        tr("worldedit.command.description.count"), // command description
        {
            {"-c", {"-c"}},
    },
        {ParamData("args", ParamType::SoftEnum, true, "-c", "-c"),
         ParamData("block", ParamType::Block, "block"),
         ParamData("data", ParamType::Int, true, "data")},
        {{"block", "data", "args"}},
        // dynamic command callback
        [](DynamicCommand const&                                    command,
           CommandOrigin const&                                     origin,
           CommandOutput&                                           output,
           std::unordered_map<std::string, DynamicCommand::Result>& results) {
            auto player = origin.getPlayer();
            if (player == nullptr) {
                output.trError("worldedit.error.noplayer");
                return;
            }
            auto xuid = player->getXuid();

            auto& playerData = getPlayersData(xuid);
            int   data       = -1;
            if (results["data"].isSet) {
                data = results["data"].getRaw<int>();
            }
            auto blockname = results["block"]
                                 .get<CommandBlockName>()
                                 .resolveBlock(data >= 0 ? data : 0)
                                 .getBlock()
                                 ->getTypeName();
            if (playerData.region != nullptr && playerData.region->hasSelected()) {
                auto&     region      = playerData.region;
                auto      dimID       = region->getDimensionID();
                auto      blockSource = &player->getDimensionBlockSource();
                long long count       = 0;
                if (results["args"].isSet) {
                    // clipboard
                } else {
                    region->forEachBlockInRegion([&](BlockPos const& pos) {
                        auto block = &blockSource->getBlock(pos);
                        if (block->getTypeName() == blockname) {
                            if (data == -1 || data == (int)(block->getTileData()))
                                count++;
                        }
                    });
                }
                output.trSuccess("worldedit.count.success", count);
            } else {
                output.trError("worldedit.error.incomplete-region");
            }
        },
        CommandPermissionLevel::GameMasters
    );

    DynamicCommand::setup(
        "distr",                                   // command name
        tr("worldedit.command.description.distr"), // command description
        {},
        {ParamData("args", ParamType::SoftEnum, true, "-cd", "-cd")},
        {{"args"}},
        // dynamic command callback
        [](DynamicCommand const&                                    command,
           CommandOrigin const&                                     origin,
           CommandOutput&                                           output,
           std::unordered_map<std::string, DynamicCommand::Result>& results) {
            auto player = origin.getPlayer();
            if (player == nullptr) {
                output.trError("worldedit.error.noplayer");
                return;
            }
            auto xuid = player->getXuid();

            auto& playerData = getPlayersData(xuid);
            bool  arg_d = false, arg_c = false;
            if (results["args"].isSet) {
                auto str = results["args"].getRaw<std::string>();
                if (str.find("-") == std::string::npos) {
                    output.trError("worldedit.command.error.args", str);
                    return;
                }
                if (str.find("d") != std::string::npos) {
                    arg_d = true;
                }
                if (str.find("c") != std::string::npos) {
                    arg_c = true;
                }
            }

            if (playerData.region != nullptr && playerData.region->hasSelected()) {
                auto& region      = playerData.region;
                auto  dimID       = region->getDimensionID();
                auto  blockSource = &player->getDimensionBlockSource();
                phmap::flat_hash_map<std::string, long long>   blocksMap;
                std::vector<std::pair<std::string, long long>> blocksMap2;
                blocksMap2.resize(0);
                long long all = 0;
                if (arg_c) {
                    region->forEachBlockInRegion([&](BlockPos const& pos) {
                        auto blockInstance = blockSource->getBlockInstance(pos);

                        if (blockInstance.hasContainer()) {
                            auto container = blockInstance.getContainer();
                            std::queue<std::unique_ptr<CompoundTag>> itemQueue;
                            for (auto& item : container->getAllSlots()) {
                                if (item->getCount() > 0) {
                                    itemQueue.emplace(
                                        const_cast<ItemStack*>(item)->getNbt()
                                    );
                                }
                            }
                            while (!itemQueue.empty()) {
                                auto* item =
                                    ItemStack::create(std::move(itemQueue.front()));
                                itemQueue.pop();
                                auto count = item->getCount();
                                if (count > 0) {
                                    all += count;
                                    std::string name;
                                    if (item->isBlock()) {
                                        auto block = item->getBlock();
                                        name       = block->getTypeName();
                                        if (arg_d) {
                                            auto states =
                                                block->getNbt()
                                                    ->value()
                                                    .at("states")
                                                    .asCompoundTag()
                                                    ->toSNBT(0, SnbtFormat::Minimize);
                                            name += " ["
                                                  + states.substr(1, states.length() - 2)
                                                  + "]";
                                        }
                                    } else {
                                        name = item->getTypeName();
                                        if (arg_d) {
                                            name            += " [";
                                            auto customName  = item->getCustomName();
                                            if (customName != "") {
                                                name += "\"name\":\"" + customName + "\"";
                                            }
                                            name += "]";
                                        }
                                    }
                                    if (!blocksMap.contains(name)) {
                                        blocksMap[name] = 0;
                                    }
                                    blocksMap[name] += count;

                                    auto iNbt = item->getNbt();
                                    if (iNbt->contains("tag", Tag::Type::Compound)
                                        && iNbt->getCompound("tag")
                                               ->contains("Items", Tag::Type::List)) {
                                        iNbt->getCompound("tag")
                                            ->getList("Items")
                                            ->forEachCompoundTag(
                                                [&](class CompoundTag const& mItem) {
                                                    itemQueue.emplace(mItem.clone());
                                                }
                                            );
                                    }
                                }
                                delete item;
                                item = nullptr;
                            }
                        }
                    });
                } else {
                    region->forEachBlockInRegion([&](BlockPos const& pos) {
                        all++;
                        auto        block     = &blockSource->getBlock(pos);
                        std::string blockName = block->getTypeName();

                        if (arg_d) {
                            auto states = block->getNbt()
                                              ->value()
                                              .at("states")
                                              .asCompoundTag()
                                              ->toSNBT(0, SnbtFormat::Minimize);
                            blockName +=
                                " [" + states.substr(1, states.length() - 2) + "]";
                        }
                        auto exBlock = &blockSource->getExtraBlock(pos);
                        if (!(exBlock == BedrockBlocks::mAir)) {
                            blockName += " & " + exBlock->getTypeName();

                            if (arg_d) {
                                auto exStates = exBlock->getNbt()
                                                    ->value()
                                                    .at("states")
                                                    .asCompoundTag()
                                                    ->toSNBT(0, SnbtFormat::Minimize);
                                blockName += +" ["
                                           + exStates.substr(1, exStates.length() - 2)
                                           + "]";
                            }
                        }
                        if (blocksMap.contains(blockName)) {
                            blocksMap[blockName] += 1;
                        } else {
                            blocksMap[blockName] = 1;
                        }
                    });
                }
                for (auto& block : blocksMap) {
                    std::string name = block.first;
                    worldedit::stringReplace(name, "minecraft:", "");
                    blocksMap2.emplace_back(
                        std::pair<std::string, long long>(name, block.second)
                    );
                }
                std::sort(
                    blocksMap2.begin(),
                    blocksMap2.end(),
                    [](const std::pair<std::string, long long>& a,
                       const std::pair<std::string, long long>& b) {
                        return a.second > b.second;
                    }
                );
                std::string res(fmt::format(tr("worldedit.distr.total"), all));
                for (auto& block : blocksMap2) {
                    res += fmt::format(
                        "\n§b{}      §6({}%%) §f{}",
                        block.second,
                        worldedit::fto_string(
                            static_cast<double>(block.second) / static_cast<double>(all)
                                * 100.0,
                            3
                        ),
                        block.first
                    );
                }
                output.trSuccess(res);
            } else {
                output.trError("worldedit.error.incomplete-region");
            }
        },
        CommandPermissionLevel::GameMasters
    );
}
} // namespace we