//
// Created by OEOTYAN on 2022/05/17.
//
#pragma once

#include "Global.h"
// #include <EventAPI.h>
// #include <LoggerAPI.h>
// #include <MC/Level.hpp>
#include <MC/BlockInstance.hpp>
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

    void setBlockSimple(
        BlockSource* blockSource,
        const BlockPos& pos,
        Block* block = const_cast<Block*>(BedrockBlocks::mAir),
        Block* exblock = const_cast<Block*>(BedrockBlocks::mAir));

    inline void setFunction(
        std::unordered_map<::std::string, double>& variables,
        const Vec3& playerPos,
        const Vec2& playerRot) {
        variables["px"] = playerPos.x;
        variables["py"] = playerPos.y;
        variables["pz"] = playerPos.z;
        variables["pp"] = playerRot.x;
        variables["pt"] = playerRot.y;
    }

    template <typename functions>
    inline void setFunction(
        std::unordered_map<::std::string, double>& variables,
        functions& funcs,
        const BoundingBox& boundingBox,
        const Vec3& playerPos,
        const BlockPos& pos,
        const Vec3& center) {
        double lengthx = (boundingBox.max.x - boundingBox.min.x) * 0.5;
        double lengthy = (boundingBox.max.y - boundingBox.min.y) * 0.5;
        double lengthz = (boundingBox.max.z - boundingBox.min.z) * 0.5;
        double centerx = (boundingBox.max.x + boundingBox.min.x) * 0.5;
        double centery = (boundingBox.max.y + boundingBox.min.y) * 0.5;
        double centerz = (boundingBox.max.z + boundingBox.min.z) * 0.5;
        funcs.setPos(pos);
        variables["x"] = (pos.x - centerx) / lengthx;
        variables["y"] = (pos.y - centery) / lengthy;
        variables["z"] = (pos.z - centerz) / lengthz;
        variables["rx"] = pos.x;
        variables["ry"] = pos.y;
        variables["rz"] = pos.z;
        variables["ox"] = pos.x - playerPos.x;
        variables["oy"] = pos.y - playerPos.y;
        variables["oz"] = pos.z - playerPos.z;
        variables["cx"] = pos.x - floor(center.x);
        variables["cy"] = pos.y - floor(center.y);
        variables["cz"] = pos.z - floor(center.z);
    }

    bool changeVicePos(Player* player,
                       BlockInstance blockInstance,
                       bool output = true);

    bool changeMainPos(Player* player,
                       BlockInstance blockInstance,
                       bool output = true);

}  // namespace worldedit
