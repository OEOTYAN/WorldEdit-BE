//
// Created by OEOTYAN on 2022/06/10.
//
#include "player.hpp"

namespace worldedit {

    void playerSubscribe() {
        Event::PlayerUseItemOnEvent::subscribe([](const Event::PlayerUseItemOnEvent& ev) {
            auto itemName = ev.mItemStack->getNbt()->toBinaryNBT();
            // std::cout << ev.mItemStack->toString() << std::endl;
            // std::cout << ev.mItemStack->toDebugString() << std::endl;
            if (ev.mItemStack->getTypeName() == "minecraft:wooden_axe") {
                changeVicePos(ev.mPlayer, ev.mBlockInstance);
                return false;
            }
            auto& mod = worldedit::getMod();
            auto xuid = ev.mPlayer->getXuid();
            if (mod.playerHandToolMap.find(xuid) != mod.playerHandToolMap.end() &&
                mod.playerHandToolMap[xuid].find(itemName) != mod.playerHandToolMap[xuid].end()) {
                return false;
            }
            return true;
        });
        Event::PlayerUseItemEvent::subscribe([](const Event::PlayerUseItemEvent& ev) {
            // std::cout << ev.mPlayer->getRotation().toString() << std::endl;
            auto itemName = ev.mItemStack->getNbt()->toBinaryNBT();
            auto& mod = worldedit::getMod();
            auto xuid = ev.mPlayer->getXuid();
            if (mod.playerHandToolMap.find(xuid) != mod.playerHandToolMap.end() &&
                mod.playerHandToolMap[xuid].find(itemName) != mod.playerHandToolMap[xuid].end()) {
                bool requiereWater = true;
                if (Level::getBlock(ev.mPlayer->getPosition().toBlockPos(), ev.mPlayer->getDimensionId()) !=
                    BedrockBlocks::mAir) {
                    requiereWater = false;
                }
                BlockInstance blockInstance =
                    ev.mPlayer->getBlockFromViewVector(requiereWater, false, 2048.0f, true, false);
                mod.playerHandToolMap[xuid][itemName]->rightClick(ev.mPlayer, blockInstance);
                return false;
            }
            if (mod.playerBrushMap.find(xuid) != mod.playerBrushMap.end() &&
                mod.playerBrushMap[xuid].find(itemName) != mod.playerBrushMap[xuid].end()) {
                bool requiereWater = true;
                if (Level::getBlock(ev.mPlayer->getPosition().toBlockPos(), ev.mPlayer->getDimensionId()) !=
                    BedrockBlocks::mAir) {
                    requiereWater = false;
                }
                BlockInstance blockInstance =
                    ev.mPlayer->getBlockFromViewVector(requiereWater, false, 2048.0f, true, false);
                mod.playerBrushMap[xuid][itemName]->set(ev.mPlayer, blockInstance);
                return false;
            }
            return true;
        });

        Event::PlayerDestroyBlockEvent::subscribe([](const Event::PlayerDestroyBlockEvent& ev) {
            auto itemStack = ev.mPlayer->getHandSlot();
            auto itemName = itemStack->getNbt()->toBinaryNBT();
            // std::cout << itemStack->getTypeName() << std::endl;
            if (itemStack->getTypeName() == "minecraft:wooden_axe") {
                changeMainPos(ev.mPlayer, ev.mBlockInstance);
                return false;
            }
            auto& mod = worldedit::getMod();
            auto xuid = ev.mPlayer->getXuid();
            if (mod.playerHandToolMap.find(xuid) != mod.playerHandToolMap.end() &&
                mod.playerHandToolMap[xuid].find(itemName) != mod.playerHandToolMap[xuid].end()) {
                return false;
            }
            if (mod.playerBrushMap.find(xuid) != mod.playerBrushMap.end() &&
                mod.playerBrushMap[xuid].find(itemName) != mod.playerBrushMap[xuid].end()) {
                return false;
            }
            return true;
        });

        Event::PlayerOpenContainerEvent::subscribe([](const Event::PlayerOpenContainerEvent& ev) {
            auto itemStack = ev.mPlayer->getHandSlot();
            auto itemName = itemStack->getNbt()->toBinaryNBT();
            auto& mod = worldedit::getMod();
            auto xuid = ev.mPlayer->getXuid();
            if (mod.playerHandToolMap.find(xuid) != mod.playerHandToolMap.end() &&
                mod.playerHandToolMap[xuid].find(itemName) != mod.playerHandToolMap[xuid].end()) {
                return false;
            }
            if (mod.playerBrushMap.find(xuid) != mod.playerBrushMap.end() &&
                mod.playerBrushMap[xuid].find(itemName) != mod.playerBrushMap[xuid].end()) {
                return false;
            }
            return true;
        });

        Event::PlayerPlaceBlockEvent::subscribe([](const Event::PlayerPlaceBlockEvent& ev) {
            auto itemStack = ev.mPlayer->getHandSlot();
            auto itemName = itemStack->getNbt()->toBinaryNBT();
            auto& mod = worldedit::getMod();
            auto xuid = ev.mPlayer->getXuid();
            if (mod.playerHandToolMap.find(xuid) != mod.playerHandToolMap.end() &&
                mod.playerHandToolMap[xuid].find(itemName) != mod.playerHandToolMap[xuid].end()) {
                return false;
            }
            if (mod.playerBrushMap.find(xuid) != mod.playerBrushMap.end() &&
                mod.playerBrushMap[xuid].find(itemName) != mod.playerBrushMap[xuid].end()) {
                return false;
            }
            return true;
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
        auto& mod = worldedit::getMod();
        auto itemName = player->getHandSlot()->getNbt()->toBinaryNBT();
        auto xuid = player->getXuid();
        if (mod.playerHandToolMap.find(xuid) != mod.playerHandToolMap.end() &&
            mod.playerHandToolMap[xuid].find(itemName) != mod.playerHandToolMap[xuid].end()) {
            bool requiereWater = true;
            if (Level::getBlock(player->getPosition().toBlockPos(), player->getDimensionId()) !=
                BedrockBlocks::mAir) {
                requiereWater = false;
            }
            BlockInstance blockInstance = player->getBlockFromViewVector(requiereWater, false, 2048.0f, true, false);
            mod.playerHandToolMap[xuid][itemName]->leftClick(player, blockInstance);
        }
    }
    return original(serverNetworkHandler, networkIdentifier, animatePacket);
}