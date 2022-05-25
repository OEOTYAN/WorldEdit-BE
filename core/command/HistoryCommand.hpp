//
// Created by OEOTYAN on 2022/05/23.
//
#pragma once
#ifndef WORLDEDIT_HISTORYCOMMAND_H
#define WORLDEDIT_HISTORYCOMMAND_H

#include "pch.h"
#include <EventAPI.h>
#include <LoggerAPI.h>
#include <MC/Level.hpp>
#include <MC/BlockInstance.hpp>
#include <MC/Block.hpp>
#include <MC/BlockActor.hpp>
#include <MC/BlockSource.hpp>
#include <MC/CompoundTag.hpp>
#include <MC/Actor.hpp>
#include <MC/Player.hpp>
#include <MC/ServerPlayer.hpp>
#include <MC/Dimension.hpp>
#include <MC/ItemStack.hpp>
#include "Version.h"
#include "string/StringTool.h"
#include <LLAPI.h>
#include <ServerAPI.h>
#include <EventAPI.h>
#include <ScheduleAPI.h>
#include <DynamicCommandAPI.h>
#include "WorldEdit.h"

namespace worldedit {
    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;

    // undo redo maxhistorylength clearhistory

    void HistoryCommandSetup() {
        DynamicCommand::setup(
            "maxhistorylength",        // command name
            "set max history length",  // command description
            {}, {ParamData("num", ParamType::Int, false, "num")}, {{"num"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                mod.maxHistoryLength = results["num"].get<int>();
                output.success(fmt::format("§amax history length set to {}",
                                           mod.maxHistoryLength));
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "clearhistory",   // command name
            "clear your history",  // command description
            {}, {}, {{}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                mod.playerHistoryMap.erase(xuid);
                output.success(fmt::format("§ahistory cleared"));
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "undo",  // command name
            "undo",  // command description
            {}, {}, {{}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                auto res = mod.getPlayerUndoHistory(xuid);
                auto flag = res.second;
                if (res.second == -1) {
                    output.error(fmt::format("§aYou don't have a history yet"));
                    return;
                } else if (res.second == 0) {
                    output.error(fmt::format("Undo limit reached"));
                    return;
                }
                int dimID = res.first->playerRelPos.x;
                long long i = 0;

                Clipboard tmp(res.first->board);
                auto blockSource = Level::getBlockSource(dimID);
                res.first->forEachBlockInClipboard([&](const BlockPos& pos) {
                    auto worldPos = pos + res.first->playerPos;
                    auto blockInstance =
                        blockSource->getBlockInstance(worldPos);
                    tmp.storeBlock(blockInstance, pos);
                });
                tmp.playerRelPos.x = dimID;
                tmp.playerPos = res.first->playerPos;

                res.first->forEachBlockInClipboard([&](const BlockPos& pos) {
                    auto worldPos = pos + res.first->playerPos;
                    res.first->blockslist[res.first->getIter(pos)].setBlock(
                        worldPos, dimID);
                    i++;
                });
                *res.first = tmp;
                output.success(fmt::format("§a{} block(s) undone", i));
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "redo",  // command name
            "redo",  // command description
            {}, {}, {{}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                auto res = mod.getPlayerRedoHistory(xuid);
                auto flag = res.second;
                if (res.second == -1) {
                    output.error(fmt::format("§aYou don't have a history yet"));
                    return;
                } else if (res.second == 0) {
                    output.error(fmt::format("Redo limit reached"));
                    return;
                }
                int dimID = res.first->playerRelPos.x;
                long long i = 0;

                Clipboard tmp(res.first->board);
                auto blockSource = Level::getBlockSource(dimID);
                res.first->forEachBlockInClipboard([&](const BlockPos& pos) {
                    auto worldPos = pos + res.first->playerPos;
                    auto blockInstance =
                        blockSource->getBlockInstance(worldPos);
                    tmp.storeBlock(blockInstance, pos);
                });
                tmp.playerRelPos.x = dimID;
                tmp.playerPos = res.first->playerPos;

                res.first->forEachBlockInClipboard([&](const BlockPos& pos) {
                    auto worldPos = pos + res.first->playerPos;
                    res.first->blockslist[res.first->getIter(pos)].setBlock(
                        worldPos, dimID);
                    i++;
                });
                *res.first = tmp;
                output.success(fmt::format("§a{} block(s) redone", i));
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit

#endif  // WORLDEDIT_HISTORYCOMMAND_H