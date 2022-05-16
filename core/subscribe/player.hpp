//
// Created by OEOTYAN on 2022/05/16.
//
#pragma once
#ifndef WORLDEDIT_PLAYER_H
#define WORLDEDIT_PLAYER_H
#include "pch.h"
#include <EventAPI.h>
#include <LoggerAPI.h>
#include <MC/Level.hpp>
#include <MC/BlockInstance.hpp>
#include <MC/Block.hpp>
#include <MC/BlockSource.hpp>
#include <MC/Actor.hpp>
#include <MC/Player.hpp>
#include <MC/ServerPlayer.hpp>
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
    void playerSubscribe() {
        Event::PlayerUseItemOnEvent::subscribe(
            [](const Event::PlayerUseItemOnEvent& ev) {
                if (ev.mItemStack->getTypeName() == "minecraft:wooden_axe") {
                    auto& mod = worldedit::getMod();
                    auto xuid = ev.mPlayer->getXuid();
                    if (mod.playerRegionMap.count(xuid) == 0) {
                        mod.playerRegionMap[xuid] =
                            new worldedit::CuboidRegion();
                    }
                    auto blockInstance = ev.mBlockInstance;
                    auto pos = blockInstance.getPosition();
                    if (mod.playerVicePosMap.count(xuid) == 0 ||
                        mod.playerVicePosMap[xuid].first != pos)
                        if (mod.playerRegionMap[xuid]->setVicePos(
                                pos, ev.mPlayer->getDimensionId())) {
                            ev.mPlayer->sendFormattedText(
                                "§aSecond position set to ({}, {}, {})", pos.x,
                                pos.y, pos.z);
                            mod.playerVicePosMap[xuid].first = pos;
                            mod.playerVicePosMap[xuid].second.first = 0;
                            mod.playerVicePosMap[xuid].second.second =
                                ev.mPlayer->getDimensionId();
                        } else {
                            ev.mPlayer->sendFormattedText(
                                "§cSecond position set failed");
                        }
                    return false;
                }
                // std::cout << ev.mItemStack->getTypeName() << std::endl;
                // auto blockInstance = ev.mBlockInstance;
                // std::cout << blockInstance.getBlock()->getTypeName() <<
                // std::endl;
                return true;
            });

        Event::PlayerDestroyBlockEvent::subscribe(
            [](const Event::PlayerDestroyBlockEvent& ev) {
                if (ev.mPlayer->getHandSlot()->getTypeName() ==
                    "minecraft:wooden_axe") {
                    auto& mod = worldedit::getMod();
                    auto xuid = ev.mPlayer->getXuid();
                    if (mod.playerRegionMap.count(xuid) == 0) {
                        mod.playerRegionMap[xuid] =
                            new worldedit::CuboidRegion();
                    }
                    auto blockInstance = ev.mBlockInstance;
                    auto pos = blockInstance.getPosition();
                    if (mod.playerRegionMap[xuid]->setMainPos(
                            pos, ev.mPlayer->getDimensionId())) {
                        ev.mPlayer->sendFormattedText(
                            "§aFirst position set to ({}, {}, {})", pos.x,
                            pos.y, pos.z);
                        mod.playerMainPosMap[xuid].first = pos;
                        mod.playerMainPosMap[xuid].second.first = 0;
                        mod.playerMainPosMap[xuid].second.second =
                            ev.mPlayer->getDimensionId();
                    } else {
                        ev.mPlayer->sendFormattedText(
                            "§cFirst position set failed");
                    }
                    return false;
                }

                return true;
            });
    }
}  // namespace worldedit

#endif  // WORLDEDIT_PLAYER_H