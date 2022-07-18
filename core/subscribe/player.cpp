//
// Created by OEOTYAN on 2022/06/10.
//
#include "player.hpp"

#define MINIMUM__RESPONSE_TICK 3

namespace worldedit {

    bool playerLeftClick(Player* player, bool isLong, class ItemStack* item, BlockInstance& blockInstance) {
        static std::unordered_map<std::string, long long> tickMap;

        bool needDiscard = false;

        auto xuid = player->getXuid();
        long long tick = player->getLevel().getCurrentServerTick().t;
        if (tickMap.find(xuid) != tickMap.end()) {
            if (abs(tick - tickMap[xuid]) < MINIMUM__RESPONSE_TICK) {
                needDiscard = true;
            }
        }
        if (!isLong)
            tickMap[xuid] = tick;

        auto itemName = item->getTypeName();

        if (!(player->isOP() && player->isCreative())) {
            return true;
        } else if (blockInstance == BlockInstance::Null) {
            return false;
        } else if (itemName == "minecraft:wooden_axe") {
            if (!isLong) {
                if (!needDiscard) {
                    changeMainPos(player, blockInstance);
                }
                return false;
            }
        } else {
            auto& mod = worldedit::getMod();
            if (mod.playerHandToolMap.find(xuid) != mod.playerHandToolMap.end()) {
                auto& toolMap = mod.playerHandToolMap[xuid];
                itemName += std::to_string(item->getAuxValue());
                if (toolMap.find(itemName) != toolMap.end()) {
                    if (!needDiscard) {
                        toolMap[itemName]->leftClick(player, blockInstance);
                    }
                    return false;
                }
            }
        }
        return true;
    }

    bool playerRightClick(Player* player, bool isLong, class ItemStack* item, BlockInstance& blockInstance) {
        static std::unordered_map<std::string, long long> tickMap;

        bool needDiscard = false;

        auto xuid = player->getXuid();
        long long tick = player->getLevel().getCurrentServerTick().t;
        if (tickMap.find(xuid) != tickMap.end()) {
            if (abs(tick - tickMap[xuid]) < MINIMUM__RESPONSE_TICK) {
                needDiscard = true;
            }
        }
        tickMap[xuid] = tick;

        auto itemName = item->getTypeName();

        if (!(player->isOP() && player->isCreative())) {
            return true;
        } else if (blockInstance == BlockInstance::Null) {
            return true;
        } else if (item->getTypeName() == "minecraft:wooden_axe") {
            if (!isLong) {
                if (!needDiscard) {
                    changeVicePos(player, blockInstance);
                }
                return false;
            }
        } else {
            auto& mod = worldedit::getMod();
            itemName += std::to_string(item->getAuxValue());
            if (mod.playerHandToolMap.find(xuid) != mod.playerHandToolMap.end()) {
                auto& toolMap = mod.playerHandToolMap[xuid];
                if (toolMap.find(itemName) != toolMap.end()) {
                    if (!needDiscard) {
                        toolMap[itemName]->rightClick(player, blockInstance);
                    }
                    return false;
                }
            } else if (mod.playerBrushMap.find(xuid) != mod.playerBrushMap.end()) {
                auto& brushMap = mod.playerBrushMap[xuid];
                if (brushMap.find(itemName) != brushMap.end()) {
                    if (!needDiscard) {
                        brushMap[itemName]->set(player, blockInstance);
                    }
                    return false;
                }
            }
        }
        return true;
    }

    void playerSubscribe() {
        Event::PlayerUseItemOnEvent::subscribe([](const Event::PlayerUseItemOnEvent& ev) {
            return playerRightClick(ev.mPlayer, false, ev.mItemStack, *const_cast<BlockInstance*>(&ev.mBlockInstance));
        });
        Event::PlayerUseItemEvent::subscribe([](const Event::PlayerUseItemEvent& ev) {
            bool requiereWater = true;
            if (Level::getBlock(ev.mPlayer->getPosition().toBlockPos(), ev.mPlayer->getDimensionId()) !=
                BedrockBlocks::mAir) {
                requiereWater = false;
            }
            BlockInstance blockInstance = ev.mPlayer->getBlockFromViewVector(requiereWater, false, 2048.0f);
            return playerRightClick(ev.mPlayer, true, ev.mItemStack, blockInstance);
        });

        Event::PlayerDestroyBlockEvent::subscribe([](const Event::PlayerDestroyBlockEvent& ev) {
            return playerLeftClick(ev.mPlayer, false, ev.mPlayer->getHandSlot(),
                                   *const_cast<BlockInstance*>(&ev.mBlockInstance));
        });

        Event::PlayerOpenContainerEvent::subscribe([](const Event::PlayerOpenContainerEvent& ev) {
            bool requiereWater = true;
            if (Level::getBlock(ev.mPlayer->getPosition().toBlockPos(), ev.mPlayer->getDimensionId()) !=
                BedrockBlocks::mAir) {
                requiereWater = false;
            }
            BlockInstance blockInstance = ev.mPlayer->getBlockFromViewVector(requiereWater);
            return playerRightClick(ev.mPlayer, false, ev.mPlayer->getHandSlot(), blockInstance);
        });

        Event::PlayerPlaceBlockEvent::subscribe([](const Event::PlayerPlaceBlockEvent& ev) {
            bool requiereWater = true;
            if (Level::getBlock(ev.mPlayer->getPosition().toBlockPos(), ev.mPlayer->getDimensionId()) !=
                BedrockBlocks::mAir) {
                requiereWater = false;
            }
            BlockInstance blockInstance = ev.mPlayer->getBlockFromViewVector(requiereWater);
            return playerRightClick(ev.mPlayer, false, ev.mPlayer->getHandSlot(), blockInstance);
        });
    }
}  // namespace worldedit

THook(void,
      "?handle@ServerNetworkHandler@@UEAAXAEBVNetworkIdentifier@@"
      "AEBVAnimatePacket@@@Z",
      ServerNetworkHandler* serverNetworkHandler,
      NetworkIdentifier const& networkIdentifier,
      AnimatePacket const& animatePacket) {
    if (animatePacket.mAction == AnimatePacket::Action::Swing) {
        Player* player = serverNetworkHandler->getServerPlayer(networkIdentifier);
        bool requiereWater = true;
        if (Level::getBlock(player->getPosition().toBlockPos(), player->getDimensionId()) != BedrockBlocks::mAir) {
            requiereWater = false;
        }
        BlockInstance blockInstance = player->getBlockFromViewVector(requiereWater, false, 2048.0f);
        worldedit::playerLeftClick(player, true, player->getHandSlot(), blockInstance);
    }
    return original(serverNetworkHandler, networkIdentifier, animatePacket);
}