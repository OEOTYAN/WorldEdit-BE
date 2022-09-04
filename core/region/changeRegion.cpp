//
// Created by OEOTYAN on 2022/06/10.
//
#include "ChangeRegion.hpp"
#include "WorldEdit.h"
#include <ScheduleAPI.h>

#define MINIMUM__RESPONSE_TICK 3

namespace worldedit {

    void setBlockSimple(BlockSource* blockSource, const BlockPos& pos, Block* block, Block* exblock) {
        auto& mod = worldedit::getMod();
        CommandUtils::clearBlockEntityContents(*blockSource, pos);
        blockSource->setExtraBlock(pos, *BedrockBlocks::mAir, 16 + mod.updateArg);
        if (mod.updateExArg % 2 == 1) {
            blockSource->setBlock(pos, *block, mod.updateArg, nullptr, nullptr);
        } else {
            blockSource->setBlockNoUpdate(pos.x, pos.y, pos.z, *block);
        }
        if (block != StaticVanillaBlocks::mBubbleColumn) {
            if (exblock != BedrockBlocks::mAir) {
                blockSource->setExtraBlock(pos, *exblock, 16 + mod.updateArg);
            }
        } else {
            blockSource->setExtraBlock(pos, *StaticVanillaBlocks::mFlowingWater, 16 + mod.updateArg);
            if (mod.updateExArg % 2 == 1) {
                blockSource->setBlock(pos, *block, mod.updateArg, nullptr, nullptr);
            } else {
                blockSource->setBlockNoUpdate(pos.x, pos.y, pos.z, *block);
            }
        }
    }

    bool changeVicePos(Player* player, BlockInstance blockInstance, bool output) {
        static std::unordered_map<std::string, long long> tickMap;
        if (blockInstance == BlockInstance::Null) {
            return false;
        }
        auto xuid = player->getXuid();
        long long tick = player->getLevel().getCurrentServerTick().t;
        if (tickMap.find(xuid) != tickMap.end()) {
            if (abs(tick - tickMap[xuid]) < MINIMUM__RESPONSE_TICK)
                return true;
        }
        tickMap[xuid] = tick;

        if (blockInstance == BlockInstance::Null) {
            if (output)
                player->sendFormattedText("§cSecond position set failed");
            return false;
        }
        auto& mod = worldedit::getMod();
        if (mod.playerRegionMap.find(xuid) == mod.playerRegionMap.end()) {
            mod.playerRegionMap[xuid] = new worldedit::CuboidRegion();
        }
        auto pos = blockInstance.getPosition();
        if (player->getDimensionId() != mod.playerRegionMap[xuid]->getDimensionID()) {
            mod.playerRegionMap[xuid]->selecting = false;
        }
        if (mod.playerRegionMap[xuid]->setVicePos(pos, player->getDimensionId())) {
            if (output)
                player->sendFormattedText("§aSecond position set to ({}, {}, {})", pos.x, pos.y, pos.z);
            mod.playerVicePosMap[xuid].first = pos;
            mod.playerVicePosMap[xuid].second.first = 0;
            mod.playerVicePosMap[xuid].second.second = player->getDimensionId();
            return true;
        } else {
            if (output)
                player->sendFormattedText("§cSecond position set failed");
        }
        return false;
    }

    bool changeMainPos(Player* player, BlockInstance blockInstance, bool output) {
        static std::unordered_map<std::string, long long> tickMap;
        if (blockInstance == BlockInstance::Null) {
            return false;
        }
        auto xuid = player->getXuid();
        long long tick = player->getLevel().getCurrentServerTick().t;
        if (tickMap.find(xuid) != tickMap.end()) {
            if (abs(tick - tickMap[xuid]) < MINIMUM__RESPONSE_TICK)
                return true;
        }
        tickMap[xuid] = tick;

        if (blockInstance == BlockInstance::Null) {
            if (output)
                player->sendFormattedText("§cFirst position set failed");
            return false;
        }
        auto& mod = worldedit::getMod();
        if (mod.playerRegionMap.find(xuid) == mod.playerRegionMap.end()) {
            mod.playerRegionMap[xuid] = new worldedit::CuboidRegion();
        }
        auto pos = blockInstance.getPosition();
        if (player->getDimensionId() != mod.playerRegionMap[xuid]->getDimensionID()) {
            mod.playerRegionMap[xuid]->selecting = false;
        }
        if (mod.playerRegionMap[xuid]->setMainPos(pos, player->getDimensionId())) {
            if (output)
                player->sendFormattedText("§aFirst position set to ({}, {}, {})", pos.x, pos.y, pos.z);
            mod.playerMainPosMap[xuid].first = pos;
            mod.playerMainPosMap[xuid].second.first = 0;
            mod.playerMainPosMap[xuid].second.second = player->getDimensionId();
            if (mod.playerRegionMap[xuid]->needResetVice) {
                mod.playerVicePosMap.erase(xuid);
            }
            return true;
        } else {
            if (output)
                player->sendFormattedText("§cFirst position set failed");
        }
        return false;
    }

}  // namespace worldedit