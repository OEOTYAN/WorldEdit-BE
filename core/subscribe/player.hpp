//
// Created by OEOTYAN on 2022/05/16.
//
#pragma once
#ifndef WORLDEDIT_PLAYER_H
#define WORLDEDIT_PLAYER_H
#include "pch.h"
#include <EventAPI.h>
// #include <LoggerAPI.h>
// #include <MC/Level.hpp>
// #include <MC/BlockInstance.hpp>
// #include <MC/Block.hpp>
// #include <MC/BlockSource.hpp>
#include <MC/BedrockBlocks.hpp>
// #include <MC/Actor.hpp>
// #include <MC/Player.hpp>
#include <MC/ServerPlayer.hpp>
#include <MC/ItemStack.hpp>
// #include "Version.h"
// #include "eval/Eval.h"
// #include "string/StringTool.h"
// #include <LLAPI.h>
// #include <ServerAPI.h>
// #include <EventAPI.h>
// #include <ScheduleAPI.h>
// #include <DynamicCommandAPI.h>
#include "region/changeRegion.hpp"
// #include <MC/ServerNetworkHandler.hpp>
// #include <MC/NetworkPacketEventCoordinator.hpp>
// #include <MC/PacketHeader.hpp>
// #include <MC/Packet.hpp>
//#include <MC/NetworkIdentifier.hpp>
#include <MC/AnimatePacket.hpp>
// #include "WorldEdit.h"

namespace worldedit {
    void playerSubscribe() {
        Event::PlayerUseItemOnEvent::subscribe(
            [](const Event::PlayerUseItemOnEvent& ev) {
                auto itemName = ev.mItemStack->getTypeName();
                if (itemName == "minecraft:wooden_axe") {
                    changeVicePos(ev.mPlayer, ev.mBlockInstance);
                    return false;
                }
                auto& mod = worldedit::getMod();
                auto xuid = ev.mPlayer->getXuid();
                if (mod.playerHandToolMap.find(xuid) !=
                        mod.playerHandToolMap.end() &&
                    mod.playerHandToolMap[xuid].find(itemName) !=
                        mod.playerHandToolMap[xuid].end()) {
                    // auto tmpType = mod.playerHandToolMap[xuid][itemName];
                    // if (tmpType == "farwand" || tmpType=="info") {
                    return false;
                    // }
                }
                return true;
            });
        Event::PlayerUseItemEvent::subscribe(
            [](const Event::PlayerUseItemEvent& ev) {
                auto itemName = ev.mItemStack->getTypeName();
                auto& mod = worldedit::getMod();
                auto xuid = ev.mPlayer->getXuid();
                if (mod.playerHandToolMap.find(xuid) !=
                        mod.playerHandToolMap.end() &&
                    mod.playerHandToolMap[xuid].find(itemName) !=
                        mod.playerHandToolMap[xuid].end()) {
                    bool requiereWater = true;
                    if (Level::getBlock(BlockPos(ev.mPlayer->getPosition()),
                                        ev.mPlayer->getDimensionId()) !=
                        BedrockBlocks::mAir) {
                        requiereWater = false;
                    }
                    BlockInstance blockInstance =
                        ev.mPlayer->getBlockFromViewVector(
                            requiereWater, false, 2048.0f, true, false);
                    auto tmpType = mod.playerHandToolMap[xuid][itemName];
                    if (tmpType == "farwand") {
                        changeVicePos(ev.mPlayer, blockInstance);
                    } else if (tmpType == "tree") {
                    } else if (tmpType == "deltree") {
                    } else if (tmpType == "cycler") {
                    } else if (tmpType == "info") {
                        auto block = blockInstance.getBlock();
                        auto& exblock = const_cast<Block&>(
                            blockInstance.getBlockSource()->getExtraBlock(
                                blockInstance.getPosition()));
                        std::cout << "block:\n"
                                  << block->getNbt()->toPrettySNBT()
                                  << std::endl;
                        ev.mPlayer->sendText(
                            "block:\n" + block->getNbt()->toPrettySNBT(true));
                        if (&exblock != BedrockBlocks::mAir) {
                            std::cout << "exBlock:\n"
                                      << exblock.getNbt()->toPrettySNBT()
                                      << std::endl;
                            ev.mPlayer->sendText(
                                "exBlock:\n" +
                                exblock.getNbt()->toPrettySNBT(true));
                        }
                        if (blockInstance.hasBlockEntity()) {
                            std::cout << "blockEntity:\n"
                                      << blockInstance.getBlockEntity()
                                             ->getNbt()
                                             ->toPrettySNBT()
                                      << std::endl;
                            ev.mPlayer->sendText("blockEntity:\n" +
                                                 blockInstance.getBlockEntity()
                                                     ->getNbt()
                                                     ->toPrettySNBT(true));
                        }
                    } else if (tmpType == "flood") {
                    }
                    return false;
                }
                return true;
            });

        Event::PlayerDestroyBlockEvent::subscribe(
            [](const Event::PlayerDestroyBlockEvent& ev) {
                auto itemName = ev.mPlayer->getHandSlot()->getTypeName();
                if (itemName == "minecraft:wooden_axe") {
                    changeMainPos(ev.mPlayer, ev.mBlockInstance);
                    return false;
                }
                auto& mod = worldedit::getMod();
                auto xuid = ev.mPlayer->getXuid();
                if (mod.playerHandToolMap.find(xuid) !=
                        mod.playerHandToolMap.end() &&
                    mod.playerHandToolMap[xuid].find(itemName) !=
                        mod.playerHandToolMap[xuid].end()) {
                    // if (mod.playerHandToolMap[xuid][itemName] == "farwand") {
                    return false;
                    //}
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
        Player* player =
            serverNetworkHandler->getServerPlayer(networkIdentifier);
        auto& mod = worldedit::getMod();
        auto itemName = player->getHandSlot()->getTypeName();
        auto xuid = player->getXuid();
        if (mod.playerHandToolMap.find(xuid) != mod.playerHandToolMap.end() &&
            mod.playerHandToolMap[xuid].find(itemName) !=
                mod.playerHandToolMap[xuid].end()) {
            if (mod.playerHandToolMap[xuid][itemName] == "farwand") {
                bool requiereWater = true;
                if (Level::getBlock(BlockPos(player->getPosition()),
                                    player->getDimensionId()) !=
                    BedrockBlocks::mAir) {
                    requiereWater = false;
                }
                BlockInstance blockInstance = player->getBlockFromViewVector(
                    requiereWater, false, 2048.0f, true, false);
                worldedit::changeMainPos(player, blockInstance);
            }
        }
    }
    return original(serverNetworkHandler, networkIdentifier, animatePacket);
}

#endif  // WORLDEDIT_PLAYER_H