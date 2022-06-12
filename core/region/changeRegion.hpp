//
// Created by OEOTYAN on 2022/05/17.
//
#pragma once
#ifndef WORLDEDIT_CHANGEREGION_H
#define WORLDEDIT_CHANGEREGION_H

#include "pch.h"
// #include <EventAPI.h>
// #include <LoggerAPI.h>
// #include <MC/Level.hpp>
#include <MC/BlockInstance.hpp>
#include <MC/VanillaBlocks.hpp>
// #include <MC/Block.hpp>
// #include <MC/BlockSource.hpp>
// #include <MC/Actor.hpp>
#include <MC/Player.hpp>
// #include <MC/ServerPlayer.hpp>
// #include <MC/ItemStack.hpp>
// #include "Version.h"
#include "eval/Eval.h"
// #include "string/StringTool.h"
// #include <LLAPI.h>
// #include <ServerAPI.h>
// #include <EventAPI.h>
// #include <ScheduleAPI.h>
// #include <DynamicCommandAPI.h>
// #include "particle/Graphics.h"

namespace worldedit {

    void setBlockSimple(BlockSource*    blockSource,
                        const BlockPos& pos,
                        Block*          block   = const_cast<Block*>(BedrockBlocks::mAir),
                        Block*          exblock = const_cast<Block*>(BedrockBlocks::mAir));

    template <typename functions>
    void setFunction(std::unordered_map<::std::string, double>& variables,
                     functions&                                 funcs,
                     const BoundingBox&                         boundingBox,
                     const Vec3&                                playerPos,
                     const BlockPos&                            pos,
                     const Vec3&                                center) {
        double lengthx = (boundingBox.max.x - boundingBox.min.x) * 0.5;
        double lengthy = (boundingBox.max.y - boundingBox.min.y) * 0.5;
        double lengthz = (boundingBox.max.z - boundingBox.min.z) * 0.5;
        double centerx = (boundingBox.max.x + boundingBox.min.x) * 0.5;
        double centery = (boundingBox.max.y + boundingBox.min.y) * 0.5;
        double centerz = (boundingBox.max.z + boundingBox.min.z) * 0.5;
        funcs.setPos(pos);
        variables["x"]  = (pos.x - centerx) / lengthx;
        variables["y"]  = (pos.y - centery) / lengthy;
        variables["z"]  = (pos.z - centerz) / lengthz;
        variables["rx"] = pos.x;
        variables["ry"] = pos.y;
        variables["rz"] = pos.z;
        variables["ox"] = pos.x - floor(playerPos.x);
        variables["oy"] = pos.y - floor(playerPos.y);
        variables["oz"] = pos.z - floor(playerPos.z);
        variables["cx"] = pos.x - floor(center.x);
        variables["cy"] = pos.y - floor(center.y);
        variables["cz"] = pos.z - floor(center.z);
    }

    bool changeVicePos(Player* player, BlockInstance blockInstance, bool output = true);

    bool changeMainPos(Player* player, BlockInstance blockInstance, bool output = true);

}  // namespace worldedit

#endif  // WORLDEDIT_CHANGEREGION_H