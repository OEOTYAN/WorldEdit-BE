//
// Created by OEOTYAN on 2022/05/17.
//
#pragma once
#ifndef WORLDEDIT_CHANGEREGION_H
#define WORLDEDIT_CHANGEREGION_H

#include "pch.h"
#include <EventAPI.h>
#include <LoggerAPI.h>
#include <MC/Level.hpp>
#include <MC/BlockInstance.hpp>
#include <MC/Block.hpp>
#include <MC/BlockSource.hpp>
#include <MC/Actor.hpp>
#include <MC/Player.hpp>
#include <MC/ServerPlayer.hpp>
#include <MC/ItemStack.hpp>
#include "Version.h"
#include "eval/Eval.h"
#include "string/StringTool.h"
#include <LLAPI.h>
#include <ServerAPI.h>
#include <EventAPI.h>
#include <ScheduleAPI.h>
#include <DynamicCommandAPI.h>
#include "particle/Graphics.h"
#include "WorldEdit.h"

namespace worldedit {
    void setBlock(const char* funcstr,
                  BlockSource* blockSource,
                  const BlockPos& pos,
                  Block* block,
                  Block* exblock = const_cast<Block*>(BedrockBlocks::mAir)) {
        static EvalFunctions mGMaskFunction;
        mGMaskFunction.setbs(blockSource);
        mGMaskFunction.setPos(pos);
        static std::map<std::string, double> mGlobalVariables;
        mGlobalVariables["x"] = pos.x;
        mGlobalVariables["y"] = pos.y;
        mGlobalVariables["z"] = pos.z;
        if (cpp_eval::eval<double>(funcstr, mGlobalVariables, mGMaskFunction) >
            0.5) {
            auto blockInstance = blockSource->getBlockInstance(pos);
            if (blockInstance.hasContainer()) {
                blockInstance.getContainer()->removeAllItems();
            }
            blockSource->setExtraBlock(pos, *exblock, 2);
            blockSource->setBlock(pos, *block, 2, nullptr, nullptr);
        }
    }
    void setBlock(BlockSource* blockSource,
                  const BlockPos& pos,
                  Block* block,
                  Block* exblock = const_cast<Block*>(BedrockBlocks::mAir)) {
        auto blockInstance = blockSource->getBlockInstance(pos);
        if (blockInstance.hasContainer()) {
            blockInstance.getContainer()->removeAllItems();
        }
        blockSource->setExtraBlock(pos, *exblock, 2);
        blockSource->setBlock(pos, *block, 2, nullptr, nullptr);
    }
    bool changeVicePos(Player* player,
                       BlockInstance blockInstance,
                       bool output = true) {
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
                       bool output = true) {
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
        if (mod.playerMainPosMap.find(xuid) == mod.playerMainPosMap.end() ||
            mod.playerMainPosMap[xuid].first != pos)
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
                return true;
            } else {
                if (output)
                    player->sendFormattedText("§cFirst position set failed");
            }
        return false;
    }

}  // namespace worldedit

#endif  // WORLDEDIT_CHANGEREGION_H