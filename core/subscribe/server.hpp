//
// Created by OEOTYAN on 2022/05/16.
//
#pragma once
#ifndef WORLDEDIT_SERVER_H
#define WORLDEDIT_SERVER_H

// #include "pch.h"
// #include <EventAPI.h>
// #include <LoggerAPI.h>
// #include <MC/Level.hpp>
// #include <MC/BlockInstance.hpp>
// #include <MC/Block.hpp>
// #include <MC/BlockSource.hpp>
// #include <MC/Actor.hpp>
// #include <MC/Player.hpp>
// #include <MC/ServerPlayer.hpp>
// #include <MC/ItemStack.hpp>
// #include "Version.h"
// #include "eval/Eval.h"
// #include "string/StringTool.h"
// #include <LLAPI.h>
// #include <ServerAPI.h>
// #include <EventAPI.h>
#include <ScheduleAPI.h>
// #include <DynamicCommandAPI.h>
// #include "WorldEdit.h"

namespace worldedit {
    void serverSubscribe() {
        Event::ServerStartedEvent::subscribe([](const Event::ServerStartedEvent) {
            // Logger logger("MyPlugin");
            // logger.warn << "Warning! Fail to do sth." << logger.endl;
            auto& mod = worldedit::getMod();
            Schedule::repeat(
                [&]() {
                    auto allPlayers = Level::getAllPlayers();

                    for (auto& player : allPlayers) {
                        auto xuid = player->getXuid();
                        if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end()) {
                            mod.playerRegionMap[xuid]->renderRegion();
                        }
                    }
                    mod.renderMVpos();
                },
                1);
            return true;
        });
    }
}  // namespace worldedit

#endif  // WORLDEDIT_SERVER_H