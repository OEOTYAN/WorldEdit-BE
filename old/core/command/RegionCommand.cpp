#include "WorldEdit.h"
#include "allCommand.hpp"
#include "region/Regions.h"
#include <mc/Dimension.hpp>

namespace we {
using ParamType = DynamicCommand::ParameterType;
using ParamData = DynamicCommand::ParameterData;

// chunk contract expand shift outset inset

void regionCommandSetup() {
    DynamicCommand::setup(
        "chunk",                                   // command name
        tr("worldedit.command.description.chunk"), // command description
        {},
        {},
        {{}},
        // dynamic command callback
        [](DynamicCommand const&                                    command,
           CommandOrigin const&                                     origin,
           CommandOutput&                                           output,
           std::unordered_map<std::string, DynamicCommand::Result>& results) {
            auto player = origin.getPlayer();
            if (player == nullptr) {
                output.trError("worldedit.error.noplayer");
                return;
            }
            auto  xuid        = player->getXuid();
            auto& playerData  = getPlayersData(xuid);
            auto  heightRange = player->getDimensionConst().getHeightRange();
            if (playerData.region != nullptr && playerData.region->hasSelected()) {
                playerData.region = nullptr;
            }
            BlockPos pos = (origin.getWorldPosition()).toBlockPos();
            pos.x        = (pos.x >> 4) << 4;
            pos.z        = (pos.z >> 4) << 4;
            pos.y        = heightRange.min;

            BlockInstance blockInstance;
            blockInstance = Level::getBlockInstance(pos, player->getDimensionId());
            playerData.changeMainPos(blockInstance, false);

            pos.x         += 15;
            pos.z         += 15;
            pos.y          = heightRange.max - 1;
            blockInstance  = Level::getBlockInstance(pos, player->getDimensionId());
            playerData.changeVicePos(blockInstance, false);

            output.trSuccess("worldedit.chunk.success");
        },
        CommandPermissionLevel::GameMasters
    );

    DynamicCommand::setup(
        "contract",                                   // command name
        tr("worldedit.command.description.contract"), // command description
        {
            {"dir", {"me", "back", "up", "down", "south", "north", "east", "west"}},
    },
        {
            ParamData("dir", ParamType::Enum, true, "dir"),
            ParamData("num", ParamType::Int, false, "num"),
            ParamData("numback", ParamType::Int, true, "numback"),
        },
        {{"num", "dir", "numback"}},
        // dynamic command callback
        [](DynamicCommand const&                                    command,
           CommandOrigin const&                                     origin,
           CommandOutput&                                           output,
           std::unordered_map<std::string, DynamicCommand::Result>& results) {
            auto player = origin.getPlayer();
            if (player == nullptr) {
                output.trError("worldedit.error.noplayer");
                return;
            }
            auto  xuid       = player->getXuid();
            auto& playerData = getPlayersData(xuid);
            if (playerData.region != nullptr && playerData.region->hasSelected()) {
                int               l = results["num"].get<int>();
                worldedit::FACING facing;
                if (results["dir"].isSet) {
                    auto str = results["dir"].getRaw<std::string>();
                    if (str == "me") {
                        facing = worldedit::dirToFacing(player->getViewVector(1.0f));
                    } else if (str == "back") {
                        facing = worldedit::dirToFacing(player->getViewVector(1.0f));
                        facing = worldedit::invFacing(facing);
                    } else {
                        facing = worldedit::dirStringToFacing(str);
                    }
                } else {
                    facing = worldedit::dirToFacing(player->getViewVector(1.0f));
                }
                std::vector<BlockPos> list;
                list.resize(0);
                list.push_back(worldedit::facingToPos(facing, l));
                if (results["numback"].isSet) {
                    list.push_back(worldedit::facingToPos(
                        worldedit::invFacing(facing),
                        results["numback"].get<int>()
                    ));
                }
                auto res = playerData.region->contract(list);
                if (res.second) {
                    output.trSuccess(res.first);
                } else {
                    output.trError(res.first);
                }
            } else {
                output.trError("worldedit.error.incomplete-region");
            }
        },
        CommandPermissionLevel::GameMasters
    );

    DynamicCommand::setup(
        "expand",                                   // command name
        tr("worldedit.command.description.expand"), // command description
        {
            {"dir",  {"me", "back", "up", "down", "south", "north", "east", "west"}},
            {"vert", {"vert"}                                                      }
    },
        {
            ParamData("dir", ParamType::Enum, true, "dir"),
            ParamData("num", ParamType::Int, false, "num"),
            ParamData("numback", ParamType::Int, true, "numback"),
            ParamData("vert", ParamType::Enum, "vert"),
        },
        {{"num", "dir", "numback"}, {"vert"}},
        // dynamic command callback
        [](DynamicCommand const&                                    command,
           CommandOrigin const&                                     origin,
           CommandOutput&                                           output,
           std::unordered_map<std::string, DynamicCommand::Result>& results) {
            auto player = origin.getPlayer();
            if (player == nullptr) {
                output.trError("worldedit.error.noplayer");
                return;
            }
            auto  xuid       = player->getXuid();
            auto& playerData = getPlayersData(xuid);
            if (playerData.region != nullptr && playerData.region->hasSelected()) {
                std::vector<BlockPos> list;
                list.resize(0);
                if (results["vert"].isSet) {
                    auto  heightRange = player->getDimensionConst().getHeightRange();
                    auto& region      = playerData.region;
                    auto  boundingBox = region->getBoundBox();
                    list.push_back(
                        BlockPos(0, std::min(heightRange.min - boundingBox.min.y, 0), 0)
                    );
                    list.push_back(BlockPos(
                        0,
                        std::max(heightRange.max - boundingBox.max.y - 1, 0),
                        0
                    ));
                } else {
                    int               l = results["num"].get<int>();
                    worldedit::FACING facing;
                    if (results["dir"].isSet) {
                        auto str = results["dir"].getRaw<std::string>();
                        if (str == "me") {
                            facing = worldedit::dirToFacing(player->getViewVector(1.0f));
                        } else if (str == "back") {
                            facing = worldedit::dirToFacing(player->getViewVector(1.0f));
                            facing = worldedit::invFacing(facing);
                        } else {
                            facing = worldedit::dirStringToFacing(str);
                        }
                    } else {
                        facing = worldedit::dirToFacing(player->getViewVector(1.0f));
                    }
                    list.push_back(worldedit::facingToPos(facing, l));
                    if (results["numback"].isSet) {
                        list.push_back(worldedit::facingToPos(
                            worldedit::invFacing(facing),
                            results["numback"].get<int>()
                        ));
                    }
                }
                auto res = playerData.region->expand(list);
                if (res.second) {
                    output.trSuccess(res.first);
                } else {
                    output.trError(res.first);
                }
            } else {
                output.trError("worldedit.error.incomplete-region");
            }
        },
        CommandPermissionLevel::GameMasters
    );

    DynamicCommand::setup(
        "shift",                                   // command name
        tr("worldedit.command.description.shift"), // command description
        {
            {"dir", {"me", "back", "up", "down", "south", "north", "east", "west"}},
    },
        {ParamData("dir", ParamType::Enum, true, "dir"),
         ParamData("num", ParamType::Int, false, "num")},
        {{"num", "dir"}},
        // dynamic command callback
        [](DynamicCommand const&                                    command,
           CommandOrigin const&                                     origin,
           CommandOutput&                                           output,
           std::unordered_map<std::string, DynamicCommand::Result>& results) {
            auto player = origin.getPlayer();
            if (player == nullptr) {
                output.trError("worldedit.error.noplayer");
                return;
            }
            auto  xuid       = player->getXuid();
            auto& playerData = getPlayersData(xuid);
            if (playerData.region != nullptr && playerData.region->hasSelected()) {
                int               l = results["num"].get<int>();
                worldedit::FACING facing;
                if (results["dir"].isSet) {
                    auto str = results["dir"].getRaw<std::string>();
                    if (str == "me") {
                        facing = worldedit::dirToFacing(player->getViewVector(1.0f));
                    } else if (str == "back") {
                        facing = worldedit::dirToFacing(player->getViewVector(1.0f));
                        facing = worldedit::invFacing(facing);
                    } else {
                        facing = worldedit::dirStringToFacing(str);
                    }
                } else {
                    facing = worldedit::dirToFacing(player->getViewVector(1.0f));
                }
                auto res = playerData.region->shift(worldedit::facingToPos(facing, l));
                if (res.second) {
                    output.trSuccess(res.first);
                } else {
                    output.trError(res.first);
                }
            } else {
                output.trError("worldedit.error.incomplete-region");
            }
        },
        CommandPermissionLevel::GameMasters
    );

    DynamicCommand::setup(
        "outset",                                   // command name
        tr("worldedit.command.description.outset"), // command description
        {
    },
        {ParamData(
             "args",
             ParamType::SoftEnum,
             true,
             "-hv"
             "-hv"
         ),
         ParamData("num", ParamType::Int, false, "num")},
        {{"num", "args"}},
        // dynamic command callback
        [](DynamicCommand const&                                    command,
           CommandOrigin const&                                     origin,
           CommandOutput&                                           output,
           std::unordered_map<std::string, DynamicCommand::Result>& results) {
            auto player = origin.getPlayer();
            if (player == nullptr) {
                output.trError("worldedit.error.noplayer");
                return;
            }
            auto  xuid       = player->getXuid();
            auto& playerData = getPlayersData(xuid);
            if (playerData.region != nullptr && playerData.region->hasSelected()) {
                int                   l = results["num"].get<int>();
                std::vector<BlockPos> list;
                list.resize(0);
                if (results["args"].isSet) {
                    auto str = results["args"].getRaw<std::string>();
                    if (str == "-h") {
                        list.push_back({0, 0, -l});
                        list.push_back({0, 0, l});
                        list.push_back({-l, 0, 0});
                        list.push_back({l, 0, 0});
                    } else if (str == "-v") {
                        list.push_back({0, -l, 0});
                        list.push_back({0, l, 0});
                    } else {
                        output.trError("worldedit.command.error.args", str);
                        return;
                    }
                } else {
                    list.push_back({-l, 0, 0});
                    list.push_back({l, 0, 0});
                    list.push_back({0, -l, 0});
                    list.push_back({0, l, 0});
                    list.push_back({0, 0, -l});
                    list.push_back({0, 0, l});
                }
                auto res = playerData.region->expand(list);
                if (res.second) {
                    output.trSuccess(res.first);
                } else {
                    output.trError(res.first);
                }
            } else {
                output.trError("worldedit.error.incomplete-region");
            }
        },
        CommandPermissionLevel::GameMasters
    );

    DynamicCommand::setup(
        "inset",                                   // command name
        tr("worldedit.command.description.inset"), // command description
        {
    },
        {ParamData(
             "args",
             ParamType::SoftEnum,
             true,
             "-hv"
             "-hv"
         ),
         ParamData("num", ParamType::Int, false, "num")},
        {{"num", "args"}},
        // dynamic command callback
        [](DynamicCommand const&                                    command,
           CommandOrigin const&                                     origin,
           CommandOutput&                                           output,
           std::unordered_map<std::string, DynamicCommand::Result>& results) {
            auto player = origin.getPlayer();
            if (player == nullptr) {
                output.trError("worldedit.error.noplayer");
                return;
            }
            auto  xuid       = player->getXuid();
            auto& playerData = getPlayersData(xuid);
            if (playerData.region != nullptr && playerData.region->hasSelected()) {
                int                   l = results["num"].get<int>();
                std::vector<BlockPos> list;
                list.resize(0);
                if (results["args"].isSet) {
                    auto str = results["args"].getRaw<std::string>();
                    if (str == "-h") {
                        list.push_back({0, 0, -l});
                        list.push_back({0, 0, l});
                        list.push_back({-l, 0, 0});
                        list.push_back({l, 0, 0});
                    } else if (str == "-v") {
                        list.push_back({0, -l, 0});
                        list.push_back({0, l, 0});
                    } else {
                        output.trError("worldedit.command.error.args", str);
                        return;
                    }
                } else {
                    list.push_back({-l, 0, 0});
                    list.push_back({l, 0, 0});
                    list.push_back({0, -l, 0});
                    list.push_back({0, l, 0});
                    list.push_back({0, 0, -l});
                    list.push_back({0, 0, l});
                }
                auto res = playerData.region->contract(list);
                if (res.second) {
                    output.trSuccess(res.first);
                } else {
                    output.trError(res.first);
                }
            } else {
                output.trError("worldedit.error.incomplete-region");
            }
        },
        CommandPermissionLevel::GameMasters
    );
}
} // namespace we