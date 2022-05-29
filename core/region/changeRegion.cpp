#include "changeRegion.hpp"

namespace worldedit {
    void setBlockSimple(BlockSource* blockSource,
                        const BlockPos& pos,
                        Block* block,
                        Block* exblock) {
        CommandUtils::clearBlockEntityContents(*blockSource, pos);
        blockSource->setExtraBlock(pos, *BedrockBlocks::mAir, 2);
        blockSource->setBlock(pos, *block, 2, nullptr, nullptr);
        if (block != VanillaBlocks::mBubbleColumn) {
            if (exblock != BedrockBlocks::mAir)
                blockSource->setExtraBlock(pos, *exblock, 2);
        } else {
            blockSource->setExtraBlock(pos, *VanillaBlocks::mFlowingWater, 2);
            blockSource->setBlock(pos, *block, 2, nullptr, nullptr);
        }
    }

    bool changeVicePos(Player* player,
                       BlockInstance blockInstance,
                       bool output) {
        if (blockInstance == BlockInstance::Null) {
            if (output)
                player->sendFormattedText("§cSecond position set failed");
            return false;
        }
        auto& mod = worldedit::getMod();
        auto xuid = player->getXuid();
        if (mod.playerRegionMap.find(xuid) == mod.playerRegionMap.end()) {
            mod.playerRegionMap[xuid] = new worldedit::CuboidRegion();
        }
        auto pos = blockInstance.getPosition();
        if (mod.playerVicePosMap.find(xuid) == mod.playerVicePosMap.end() ||
            mod.playerVicePosMap[xuid].first != pos)
            if (mod.playerRegionMap[xuid]->setVicePos(
                    pos, player->getDimensionId())) {
                if (output)
                    player->sendFormattedText(
                        "§aSecond position set to ({}, {}, {})", pos.x, pos.y,
                        pos.z);
                mod.playerVicePosMap[xuid].first = pos;
                mod.playerVicePosMap[xuid].second.first = 0;
                mod.playerVicePosMap[xuid].second.second =
                    player->getDimensionId();
                return true;
            } else {
                if (output)
                    player->sendFormattedText("§cSecond position set failed");
            }
        return false;
    }

    bool changeMainPos(Player* player,
                       BlockInstance blockInstance,
                       bool output) {
        if (blockInstance == BlockInstance::Null) {
            if (output)
                player->sendFormattedText("§cFirst position set failed");
            return false;
        }
        auto& mod = worldedit::getMod();
        auto xuid = player->getXuid();
        if (mod.playerRegionMap.find(xuid) == mod.playerRegionMap.end()) {
            mod.playerRegionMap[xuid] = new worldedit::CuboidRegion();
        }
        auto pos = blockInstance.getPosition();
        // if (mod.playerMainPosMap.find(xuid) == mod.playerMainPosMap.end() ||
            // mod.playerMainPosMap[xuid].first != pos)
            if (mod.playerRegionMap[xuid]->setMainPos(
                    pos, player->getDimensionId())) {
                if (output)
                    player->sendFormattedText(
                        "§aFirst position set to ({}, {}, {})", pos.x, pos.y,
                        pos.z);
                mod.playerMainPosMap[xuid].first = pos;
                mod.playerMainPosMap[xuid].second.first = 0;
                mod.playerMainPosMap[xuid].second.second =
                    player->getDimensionId();
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