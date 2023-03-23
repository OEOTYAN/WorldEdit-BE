//
// Created by OEOTYAN on 2022/05/23.
//
#include "allCommand.hpp"
#include "WorldEdit.h"

namespace worldedit {
    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;

    // undo redo maxhistorylength clearhistory

    void historyCommandSetup() {
        DynamicCommand::setup(
            "maxhistorylength",                                    // command name
            tr("worldedit.command.description.maxhistorylength"),  // command description
            {}, {ParamData("num", ParamType::Int, false, "num")}, {{"num"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                auto& playerData = getPlayersData(xuid);
                playerData.maxHistoryLength = results["num"].get<int>();
                output.trSuccess("worldedit.history.length", playerData.maxHistoryLength);
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "clearhistory",                                    // command name
            tr("worldedit.command.description.clearhistory"),  // command description
            {}, {}, {{}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                auto& playerData = getPlayersData(xuid);
                playerData.clearHistory();
                output.trSuccess("worldedit.clearhistory.cleared");
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "undo",                                    // command name
            tr("worldedit.command.description.undo"),  // command description
            {}, {ParamData("num", ParamType::Int, true, "num")}, {{"num"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                auto& playerData = getPlayersData(xuid);
                int ut = 1;
                if (results["num"].isSet) {
                    ut = results["num"].get<int>();
                }
                for (int undoTimes = 0; undoTimes < ut; undoTimes++) {
                    auto res = playerData.getUndoHistory();
                    auto flag = res.second;
                    if (res.second == -1) {
                        output.trError("worldedit.undo.none");
                        return;
                    } else if (res.second == 0) {
                        output.trError("worldedit.undo.none");
                        return;
                    }
                    int dimID = res.first->playerRelPos.x;
                    long long i = 0;

                    Clipboard tmp(res.first->board);
                    auto blockSource = &player->getDimensionBlockSource();
                    res.first->forEachBlockInClipboard([&](const BlockPos& pos) {
                        auto worldPos = pos + res.first->playerPos;
                        auto blockInstance = blockSource->getBlockInstance(worldPos);
                        tmp.storeBlock(blockInstance, pos);
                    });
                    tmp.playerRelPos.x = dimID;
                    tmp.playerPos = res.first->playerPos;

                    res.first->forEachBlockInClipboard([&](const BlockPos& pos) {
                        auto worldPos = pos + res.first->playerPos;

                        i += res.first->blockslist[res.first->getIter(pos)].setBlockWithoutcheckGMask(
                            worldPos, blockSource, playerData);
                    });
                    *res.first = tmp;
                    res.first->used = true;
                    output.trSuccess("worldedit.undo.count", i);
                }
                output.trSuccess("worldedit.undo.undone", ut);
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "redo",                                    // command name
            tr("worldedit.command.description.redo"),  // command description
            {}, {ParamData("num", ParamType::Int, true, "num")}, {{"num"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                auto& playerData = getPlayersData(xuid);
                int rt = 1;
                if (results["num"].isSet) {
                    rt = results["num"].get<int>();
                }
                for (int redoTimes = 0; redoTimes < rt; redoTimes++) {
                    auto res = playerData.getRedoHistory();
                    auto flag = res.second;
                    if (res.second == -1) {
                        output.trError("worldedit.redo.none");
                        return;
                    } else if (res.second == 0) {
                        output.trError("worldedit.redo.none");
                        return;
                    }
                    int dimID = res.first->playerRelPos.x;
                    long long i = 0;

                    Clipboard tmp(res.first->board);
                    auto blockSource = &player->getDimensionBlockSource();
                    res.first->forEachBlockInClipboard([&](const BlockPos& pos) {
                        auto worldPos = pos + res.first->playerPos;
                        auto blockInstance = blockSource->getBlockInstance(worldPos);
                        tmp.storeBlock(blockInstance, pos);
                    });
                    tmp.playerRelPos.x = dimID;
                    tmp.playerPos = res.first->playerPos;

                    res.first->forEachBlockInClipboard([&](const BlockPos& pos) {
                        auto worldPos = pos + res.first->playerPos;
                        i += res.first->blockslist[res.first->getIter(pos)].setBlockWithoutcheckGMask(
                            worldPos, blockSource, playerData);
                    });
                    *res.first = tmp;
                    res.first->used = true;
                    output.trSuccess("worldedit.redo.count", i);
                }
                output.trSuccess("worldedit.redo.redone", rt);
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit