//
// Created by OEOTYAN on 2022/07/18.
//
#include "server.hpp"
#include "WorldEdit.h"
#include <EventAPI.h>
#include <ScheduleAPI.h>

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
            // auto mapm = getBlockColorssMap();
            // std::ofstream outStr(WE_DIR + "structures/" + "filename" + ".txt", std::ios::out);
            // for(auto aa : mapm) {
            //     outStr << "\"" << aa.first << "\" : {\n" << aa.second << "\n},\n";
            // }
            // outStr.close();

            return true;
        });
    }
}  // namespace worldedit