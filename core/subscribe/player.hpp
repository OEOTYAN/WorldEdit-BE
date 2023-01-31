//
// Created by OEOTYAN on 2022/05/16.
//
#pragma once
#include "Global.h"
#include <EventAPI.h>
// #include <LoggerAPI.h>
// #include <mc/Level.hpp>
// #include <mc/BlockInstance.hpp>
// #include <mc/Block.hpp>
// #include <mc/BlockSource.hpp>
#include <mc/BedrockBlocks.hpp>
// #include <mc/Actor.hpp>
// #include <mc/Player.hpp>
#include <mc/ServerPlayer.hpp>
#include <mc/ItemStack.hpp>
#include <mc/AnimatePacket.hpp>
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