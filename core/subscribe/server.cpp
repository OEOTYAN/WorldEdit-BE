//
// Created by OEOTYAN on 2022/07/18.
//
#include "server.hpp"
#include "WorldEdit.h"
#include <EventAPI.h>
#include <ScheduleAPI.h>
#include <MC/VanillaBlocks.hpp>

namespace worldedit {
    void serverSubscribe() {
        Event::ServerStartedEvent::subscribe([](const Event::ServerStartedEvent) {
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