#include "WorldEdit.h"
#include "allCommand.hpp"
#include "region/Regions.h"
#include <mc/Dimension.hpp>

namespace we {
using ParamType = DynamicCommand::ParameterType;
using ParamData = DynamicCommand::ParameterData;

// sel pos1 pos2 hpos1 hpos2 chunk contract expand shift outset inset loftcircle

void regionCommandSetup() {
    DynamicCommand::setup(
        "sel",                                   // command name
        tr("worldedit.command.description.sel"), // command description
        {
  // enums{enumName, {values...}}
            {"region",
             {"cuboid", "extend", "poly", "sphere", "cylinder", "convex", "loft", "clear"}
            },
            {"remove", {"remove"}                                                         },
            {"rm",     {"near", "last"}                                                   }
    },
        {
            // parameters(type, name, [optional], [enumOptions(also
            // enumName)], [identifier]) identifier: used to identify unique
            // parameter data, if idnetifier is not set,
            //   it is set to be the same as enumOptions or name (identifier
            //   = enumOptions.empty() ? name:enumOptions)
            ParamData("region", ParamType::Enum, "region"),
            ParamData("remove", ParamType::Enum, "remove"),
            ParamData("rm", ParamType::Enum, "rm"),
        },
        {
            // overloads{ (type == Enum ? enumOptions : name) ...}
            {"region"},
            {"remove", "rm"}, // testenum <add|remove>
        },
        // dynamic command callback
        [](DynamicCommand const&                                    command,
           CommandOrigin const&                                     origin,
           CommandOutput&                                           output,
           std::unordered_map<std::string, DynamicCommand::Result>& results) {
            std::string action;
            if (results["region"].isSet) {
                action = results["region"].get<std::string>();
            } else {
                action = results["rm"].get<std::string>();
            }

            auto player = origin.getPlayer();
            if (player == nullptr) {
                output.trError("worldedit.error.noplayer");
                return;
            }
            auto  xuid       = player->getXuid();
            auto& playerData = getPlayersData(xuid);

            if (results["remove"].isSet) {
                auto dimID = origin.getPlayer()->getDimensionId();
                if (!(playerData.region != nullptr && playerData.region->hasSelected())) {
                    output.trError("worldedit.error.incomplete-region");
                    return;
                }
                if (action == "last") {
                    if (playerData.region->removePoint(dimID)) {
                        output.trSuccess("worldedit.remove.last");
                        return;
                    } else {
                        output.trError("worldedit.remove.none");
                        return;
                    }
                } else {
                    if (playerData.region->removePoint(
                            dimID,
                            (origin.getPlayer()->getPosition()).toBlockPos()
                        )) {
                        output.trSuccess("worldedit.remove.near");
                        return;
                    } else {
                        output.trError("worldedit.remove.none");
                        return;
                    }
                }
            } else if (action == "clear") {
                if (playerData.region != nullptr) {
                    playerData.region->selecting = false;
                }
                output.trSuccess("worldedit.region.clear");
            } else {
                BoundingBox tmpbox;
                if (playerData.region != nullptr) {
                    tmpbox = playerData.region->getBoundBox();
                }
                switch (do_hash2(action)) {
                case do_hash2("cuboid"):
                    playerData.region = worldedit::Region::createRegion(
                        worldedit::CUBOID,
                        tmpbox,
                        origin.getPlayer()->getDimensionId()
                    );
                    output.trSuccess("worldedit.region.set.cuboid");
                    break;
                case do_hash2("extend"):
                    playerData.region = worldedit::Region::createRegion(
                        worldedit::EXPAND,
                        tmpbox,
                        origin.getPlayer()->getDimensionId()
                    );
                    output.trSuccess("worldedit.region.set.extend");
                    break;
                case do_hash2("poly"):
                    playerData.region = worldedit::Region::createRegion(
                        worldedit::POLY,
                        tmpbox,
                        origin.getPlayer()->getDimensionId()
                    );
                    output.trSuccess("worldedit.region.set.poly");
                    break;
                case do_hash2("sphere"):
                    playerData.region = worldedit::Region::createRegion(
                        worldedit::SPHERE,
                        tmpbox,
                        origin.getPlayer()->getDimensionId()
                    );
                    output.trSuccess("worldedit.region.set.sphere");
                    break;
                case do_hash2("cylinder"):
                    playerData.region = worldedit::Region::createRegion(
                        worldedit::CYLINDER,
                        tmpbox,
                        origin.getPlayer()->getDimensionId()
                    );
                    output.trSuccess("worldedit.region.set.cylinder");
                    break;
                case do_hash2("convex"):
                    playerData.region = worldedit::Region::createRegion(
                        worldedit::CONVEX,
                        tmpbox,
                        origin.getPlayer()->getDimensionId()
                    );
                    output.trSuccess("worldedit.region.set.convex");
                    break;
                case do_hash2("loft"):
                    playerData.region = worldedit::Region::createRegion(
                        worldedit::LOFT,
                        tmpbox,
                        origin.getPlayer()->getDimensionId()
                    );
                    output.trSuccess("worldedit.region.set.loft");
                    break;
                default:
                    break;
                }
            }
            playerData.mainPosDim = -1;
            playerData.vicePosDim = -1;
        },
        CommandPermissionLevel::GameMasters
    );

    DynamicCommand::setup(
        "pos1",                                   // command name
        tr("worldedit.command.description.pos1"), // command description
        {},
        {ParamData("pos", ParamType::BlockPos, true, "pos")},
        {{"pos"}},
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
            auto          xuid       = player->getXuid();
            auto&         playerData = getPlayersData(xuid);
            BlockInstance blockInstance;
            if (results["pos"].isSet) {
                auto pos      = results["pos"].get<BlockPos>();
                blockInstance = Level::getBlockInstance(pos, player->getDimensionId());
            } else {
                blockInstance = Level::getBlockInstance(
                    (origin.getWorldPosition()).toBlockPos(),
                    player->getDimensionId()
                );
            }
            playerData.changeMainPos(blockInstance);
        },
        CommandPermissionLevel::GameMasters
    );

    DynamicCommand::setup(
        "pos2",                                   // command name
        tr("worldedit.command.description.pos2"), // command description
        {},
        {ParamData("pos", ParamType::BlockPos, true, "pos")},
        {{"pos"}},
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
            auto          xuid       = player->getXuid();
            auto&         playerData = getPlayersData(xuid);
            BlockInstance blockInstance;
            if (results["pos"].isSet) {
                auto pos      = results["pos"].get<BlockPos>();
                blockInstance = Level::getBlockInstance(pos, player->getDimensionId());
            } else {
                blockInstance = Level::getBlockInstance(
                    (origin.getWorldPosition()).toBlockPos(),
                    player->getDimensionId()
                );
            }
            playerData.changeVicePos(blockInstance);
        },
        CommandPermissionLevel::GameMasters
    );

    DynamicCommand::setup(
        "hpos1",                                   // command name
        tr("worldedit.command.description.hpos1"), // command description
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
            auto          xuid       = player->getXuid();
            auto&         playerData = getPlayersData(xuid);
            BlockInstance blockInstance;
            blockInstance =
                player->getBlockFromViewVector(true, false, 2048.0f, true, false);
            playerData.changeMainPos(blockInstance);
        },
        CommandPermissionLevel::GameMasters
    );

    DynamicCommand::setup(
        "hpos2",                                   // command name
        tr("worldedit.command.description.hpos2"), // command description
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
            auto          xuid       = player->getXuid();
            auto&         playerData = getPlayersData(xuid);
            BlockInstance blockInstance;
            blockInstance =
                player->getBlockFromViewVector(true, false, 2048.0f, true, false);
            playerData.changeVicePos(blockInstance);
        },
        CommandPermissionLevel::GameMasters
    );

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

    DynamicCommand::setup(
        "loftcircle",                                   // command name
        tr("worldedit.command.description.loftcircle"), // command description
        {},
        {ParamData("bool", ParamType::Bool, "bool")},
        {{"bool"}},
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
                auto& region = playerData.region;
                if (region->regionType != LOFT) {
                    output.trError("worldedit.loftcircle.invalid-type");
                    return;
                }
                bool arg = results["bool"].get<bool>();
                static_cast<LoftRegion*>(region.get())->setCircle(arg);
                output.trSuccess(
                    "worldedit.loftcircle.success",
                    (arg ? "true" : "false")
                );
            } else {
                output.trError("worldedit.error.incomplete-region");
            }
        },
        CommandPermissionLevel::GameMasters
    );
}
} // namespace we