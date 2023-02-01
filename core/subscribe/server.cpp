//
// Created by OEOTYAN on 2022/07/18.
//
#include "server.hpp"
#include "WorldEdit.h"
#include <EventAPI.h>
#include <ScheduleAPI.h>
#include <mc/Level.hpp>
#include <mc/Player.hpp>
// #include <mc/HashedString.hpp>
// #include <mc/BlockSerializationUtils.hpp>
#include "region/Region.h"
#include "command/allCommand.hpp"

namespace worldedit {
    void serverSubscribe() {
        Event::ServerStartedEvent::subscribe([](const Event::ServerStartedEvent) {
            // for (auto& k : BlockSerializationUtils::BLOCK_REPLACE_DATA_MAP){
            //     std::cout << k.first.getString() << std::endl;
            // }

            javaBlockMapInit();

            commandsSetup();

            blockColorMapInit();

            auto& playerDataMap = getPlayersDataMap();
            Schedule::repeat(
                [&]() {
                    Global<Level>->forEachPlayer([&](Player& player) -> bool {
                        auto xuid = player.getXuid();
                        if (playerDataMap.find(xuid) != playerDataMap.end()) {
                            auto& data = playerDataMap[xuid];
                            if (data.region != nullptr && data.region->hasSelected()) {
                                data.region->renderRegion();
                            }
                        }

                        if (playerDataMap.find(xuid) != playerDataMap.end()) {
                            auto& data = playerDataMap[xuid];
                            if (data.mainPosDim >= 0) {
                                if (data.mainPosTime <= 0) {
                                    data.mainPosTime = 40;
                                    globalPT().drawCuboid(AABB(Vec3(data.mainPos) - Vec3(0.07f, 0.07f, 0.07f),
                                                               Vec3(data.mainPos) + Vec3(1.07f, 1.07f, 1.07f)),
                                                          data.mainPosDim, mce::ColorPalette::RED);
                                }
                                --data.mainPosTime;
                            }
                            if (data.vicePosDim >= 0) {
                                if (data.vicePosDim >= 0 && data.vicePosTime <= 0) {
                                    data.vicePosTime = 40;
                                    globalPT().drawCuboid(AABB(Vec3(data.vicePos) - Vec3(0.07f, 0.07f, 0.07f),
                                                               Vec3(data.vicePos) + Vec3(1.07f, 1.07f, 1.07f)),
                                                          data.vicePosDim, mce::ColorPalette::VATBLUE);
                                }
                                --data.vicePosTime;
                            }
                        }

                        return true;
                    });
                },
                1);
            // auto mapm = getBlockColorssMap();
            // std::ofstream outStr(WE_DIR + "structures/" + "filename" +
            // ".txt", std::ios::out); for(auto aa : mapm) {
            //     outStr << "\"" << aa.first << "\" : {\n" << aa.second <<
            //     "\n},\n";
            // }
            // outStr.close();

            return true;
        });
    }
}  // namespace worldedit