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
            "maxhistorylength",        // command name
            "set max history length",  // command description
            {}, {ParamData("num", ParamType::Int, false, "num")}, {{"num"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                mod.maxHistoryLength = results["num"].get<int>();
                output.success(fmt::format("§amax history length set to {}", mod.maxHistoryLength));
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "clearhistory",        // command name
            "clear your history",  // command description
            {}, {}, {{}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                mod.playerHistoryMap.erase(xuid);
                output.success(fmt::format("§ahistory cleared"));
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "undo",  // command name
            "undo",  // command description
            {}, {ParamData("num", ParamType::Int, true, "num")}, {{"num"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                int ut = 1;
                if (results["num"].isSet) {
                    ut = results["num"].get<int>();
                }
                for (int undoTimes = 0; undoTimes < ut; undoTimes++) {
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
                        auto blockInstance = blockSource->getBlockInstance(worldPos);
                        tmp.storeBlock(blockInstance, pos);
                    });
                    tmp.playerRelPos.x = dimID;
                    tmp.playerPos = res.first->playerPos;

                    res.first->forEachBlockInClipboard([&](const BlockPos& pos) {
                        auto worldPos = pos + res.first->playerPos;
                        res.first->blockslist[res.first->getIter(pos)].setBlock(worldPos, blockSource);
                        ++i;
                    });
                    *res.first = tmp;
                    res.first->used = true;
                    output.success(fmt::format("§a{} block(s) undone", i));
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "redo",  // command name
            "redo",  // command description
            {}, {ParamData("num", ParamType::Int, true, "num")}, {{"num"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                int rt = 1;
                if (results["num"].isSet) {
                    rt = results["num"].get<int>();
                }
                for (int redoTimes = 0; redoTimes < rt; redoTimes++) {
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
                        auto blockInstance = blockSource->getBlockInstance(worldPos);
                        tmp.storeBlock(blockInstance, pos);
                    });
                    tmp.playerRelPos.x = dimID;
                    tmp.playerPos = res.first->playerPos;

                    res.first->forEachBlockInClipboard([&](const BlockPos& pos) {
                        auto worldPos = pos + res.first->playerPos;
                        res.first->blockslist[res.first->getIter(pos)].setBlock(worldPos, blockSource);
                        ++i;
                    });
                    *res.first = tmp;
                    res.first->used = true;
                    output.success(fmt::format("§a{} block(s) redone", i));
                }
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit