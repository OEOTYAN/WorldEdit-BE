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
#include "region/changeRegion.hpp"

namespace worldedit {
    void playerSubscribe() {
        Event::PlayerUseItemOnEvent::subscribe(
            [](const Event::PlayerUseItemOnEvent& ev) {
                if (ev.mItemStack->getTypeName() == "minecraft:wooden_axe") {
                    changeVicePos(ev.mPlayer, ev.mBlockInstance);
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
                    changeMainPos(ev.mPlayer, ev.mBlockInstance);
                    return false;
                }

                return true;
            });
    }
}  // namespace worldedit

#endif  // WORLDEDIT_PLAYER_H