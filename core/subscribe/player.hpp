//
// Created by OEOTYAN on 2022/05/16.
//
#pragma once
#include "Global.h"
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
#include "region/ChangeRegion.hpp"
// #include <MC/ServerNetworkHandler.hpp>
// #include <MC/NetworkPacketEventCoordinator.hpp>
// #include <MC/PacketHeader.hpp>
// #include <MC/Packet.hpp>
//#include <MC/NetworkIdentifier.hpp>
#include <MC/AnimatePacket.hpp>
#include "WorldEdit.h"

namespace worldedit {
    void playerSubscribe();
    bool playerLeftClick(Player* player,
                        const bool isLong,
                         class ItemStack* item,
                         BlockInstance& blockInstance,
                         FaceID mFace);
    bool playerRightClick(Player* player,
                          const bool isLong,
                          class ItemStack* item,
                          BlockInstance& blockInstance,
                          FaceID mFace);
}  // namespace worldedit