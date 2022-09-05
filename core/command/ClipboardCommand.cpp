//
// Created by OEOTYAN on 2022/05/20.
//
// #include "Global.h"
#include "allCommand.hpp"
#include <MC/CommandUtils.hpp>
#include "MC/StructureTemplate.hpp"
#include "region/ChangeRegion.hpp"
#include "WorldEdit.h"

namespace worldedit {
    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;

    // copy cut paste rotate flip clearclipboard

    void clipboardCommandSetup() {
        DynamicCommand::setup(
            "clearclipboard",        // command name
            "clear your clipboard",  // command description
            {}, {}, {{}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                mod.playerClipboardMap.erase(xuid);
                output.success(fmt::format("§aclipboard cleared"));
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "copy",                           // command name
            "copy region to your clipboard",  // command description
            {}, {}, {{}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];
                    auto boundingBox = region->getBoundBox();
                    auto* clipboard = &mod.playerClipboardMap[xuid];
                    *clipboard = Clipboard(boundingBox.max - boundingBox.min);
                    auto pPos = origin.getPlayer()->getPosition() - Vec3(0.0, 1.62, 0.0);
                    clipboard->playerPos = pPos.toBlockPos();
                    clipboard->playerRelPos = pPos.toBlockPos() - boundingBox.min;
                    auto dimID = mod.playerRegionMap[xuid]->getDimensionID();
                    auto blockSource = Level::getBlockSource(dimID);
                    mod.playerRegionMap[xuid]->forEachBlockInRegion([&](const BlockPos& pos) {
                        auto localPos = pos - boundingBox.min;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        clipboard->storeBlock(blockInstance, localPos);
                    });
                    if (true) {
                        auto st = StructureTemplate("worldedit_copy_cmd_tmp");
                        auto setting = StructureSettings();
                        setting.setIgnoreBlocks(true);
                        setting.setIgnoreEntities(false);
                        setting.setMirror(Mirror::None_15);
                        setting.setStructureSize(boundingBox.max - boundingBox.min + 1);
                        setting.setRotation(Rotation::None_14);
                        st.fillFromWorld(*blockSource, boundingBox.min + BlockPos(0, 1, 0), setting);
                        // auto structure = st.toTag()->toBinaryNBT();
                        // std::cout << structure << std::endl;
                        boundingBox += BlockPos(0, 1, 0);
                        clipboard->entityStr =
                            //  StructureTemplate::fromWorld("worldedit_copy_cmd_tmp", dimID,
                            //                                                     boundingBox.min, boundingBox.max,
                            //                                                     false, true)
                            st.toTag()->toBinaryNBT();
                    }
                    output.success(fmt::format("§aregion copied"));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "cut",                           // command name
            "cut region to your clipboard",  // command description
            {}, {}, {{}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.find(xuid) != mod.playerRegionMap.end() &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    auto* region = mod.playerRegionMap[xuid];
                    auto dimID = region->getDimensionID();
                    auto boundingBox = region->getBoundBox();
                    auto blockSource = Level::getBlockSource(dimID);

                  if (mod.maxHistoryLength > 0) {  auto history = mod.getPlayerNextHistory(xuid);
                    *history = Clipboard(boundingBox.max - boundingBox.min);
                    history->playerRelPos.x = dimID;
                    history->playerPos = boundingBox.min;

                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        auto localPos = pos - boundingBox.min;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history->storeBlock(blockInstance, localPos);
                    });}

                    auto pPos = origin.getPlayer()->getPosition() - Vec3(0.0, 1.62, 0.0);

                    auto* clipboard = &mod.playerClipboardMap[xuid];
                    *clipboard = Clipboard(boundingBox.max - boundingBox.min);
                    clipboard->playerPos = pPos.toBlockPos();
                    clipboard->playerRelPos = pPos.toBlockPos() - boundingBox.min;
                    region->forEachBlockInRegion([&](const BlockPos& pos) {
                        auto localPos = pos - boundingBox.min;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        clipboard->storeBlock(blockInstance, localPos);
                    });
                    region->forEachBlockInRegion([&](const BlockPos& pos) { setBlockSimple(blockSource, pos); });
                    output.success(fmt::format("§aregion cutted"));
                } else {
                    output.error("You don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "paste",                 // command name
            "paste your clipboard",  // command description
            {}, {ParamData("args", ParamType::SoftEnum, true, "-anose", "-anose")}, {{"args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                if (mod.playerClipboardMap.find(xuid) != mod.playerClipboardMap.end()) {
                    auto* clipboard = &mod.playerClipboardMap[xuid];
                    bool arg_a = false, arg_n = false, arg_o = false, arg_s = false, arg_e = false;
                    if (results["args"].isSet) {
                        auto str = results["args"].getRaw<std::string>();
                        if (str.find("-") == std::string::npos) {
                            output.error("wrong args");
                            return;
                        }
                        if (str.find("a") != std::string::npos) {
                            arg_a = true;
                        }
                        if (str.find("n") != std::string::npos) {
                            arg_n = true;
                        }
                        if (str.find("o") != std::string::npos) {
                            arg_o = true;
                        }
                        if (str.find("s") != std::string::npos) {
                            arg_s = true;
                        }
                        if (str.find("e") != std::string::npos) {
                            arg_e = true;
                        }
                    }
                    BlockPos pbPos;
                    if (arg_o) {
                        pbPos = clipboard->playerPos;
                    } else {
                        auto pPos = origin.getPlayer()->getPosition() - Vec3(0.0, 1.62, 0.0);
                        pbPos = pPos.toBlockPos();
                    }

                    BlockInstance blockInstance;
                    BoundingBox box = clipboard->getBoundingBox() + pbPos;

                    if (arg_n || arg_s) {
                        blockInstance = Level::getBlockInstance(box.min, origin.getPlayer()->getDimensionId());
                        changeMainPos(origin.getPlayer(), blockInstance, false);
                        blockInstance = Level::getBlockInstance(box.max, origin.getPlayer()->getDimensionId());
                        changeVicePos(origin.getPlayer(), blockInstance, false);
                    }

                    auto dimID = origin.getPlayer()->getDimensionId();
                    if (!arg_n) {
                        auto blockSource = Level::getBlockSource(dimID);
                     if (mod.maxHistoryLength > 0) {   auto history = mod.getPlayerNextHistory(xuid);
                        *history = Clipboard(box.max - box.min);
                        history->playerRelPos.x = dimID;
                        history->playerPos = box.min;

                        box.forEachBlockInBox([&](const BlockPos& pos) {
                            auto localPos = pos - box.min;
                            auto blockInstance = blockSource->getBlockInstance(pos);
                            history->storeBlock(blockInstance, localPos);
                        });}

                        if (arg_a) {
                            clipboard->forEachBlockInClipboard([&](const BlockPos& pos) {
                                if (clipboard->getSet(pos).getBlock() == BedrockBlocks::mAir &&
                                    clipboard->getSet(pos).getExBlock() == BedrockBlocks::mAir) {
                                    return;
                                }
                                auto worldPos = clipboard->getPos(pos) + pbPos;
                                clipboard->setBlocks(pos, worldPos, blockSource);
                            });
                        } else {
                            clipboard->forEachBlockInClipboard([&](const BlockPos& pos) {
                                auto worldPos = clipboard->getPos(pos) + pbPos;
                                clipboard->setBlocks(pos, worldPos, blockSource);
                            });
                        }
                        if (arg_e && !clipboard->entityStr.empty()) {
                            auto st = StructureTemplate("worldedit_copy_cmd_tmp");
                            st.getData()->load(*(CompoundTag::fromBinaryNBT(clipboard->entityStr).get()));
                            auto& palette = Global<Level>->getBlockPalette();
                            auto setting = StructureSettings();
                            setting.setMirror(clipboard->mirror);
                            setting.setStructureSize(st.getSize());
                            setting.setRotation(clipboard->rotation);
                            st.placeInWorld(*blockSource, palette,
                                            clipboard->getPos({0, 0, 0}) + pbPos + BlockPos(0, 1, 0), setting, nullptr,
                                            false);
                        }
                    }

                    output.success(fmt::format("§aclipboard pasted"));
                } else {
                    output.error("You don't have a clipboard yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "flip",            // command name
            "flip clipboard",  // command description
            {
                {"dir", {"me", "up", "down", "south", "north", "east", "west"}},
            },
            {ParamData("dir", ParamType::Enum, true, "dir")}, {{"dir"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerClipboardMap.find(xuid) != mod.playerClipboardMap.end()) {
                    worldedit::FACING facing;
                    if (results["dir"].isSet) {
                        auto str = results["dir"].getRaw<std::string>();
                        if (str == "me") {
                            facing = worldedit::dirToFacing(player->getViewVector(1.0f));
                        } else {
                            facing = worldedit::dirStringToFacing(str);
                        }
                    } else {
                        facing = worldedit::dirToFacing(player->getViewVector(1.0f));
                    }
                    mod.playerClipboardMap[xuid].flip(facing);
                    output.success(fmt::format("§aclipboard fliped"));
                } else {
                    output.error("You don't have a clipboard yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "rotate",            // command name
            "rotate clipboard",  // command description
            {},
            {ParamData("angleY", ParamType::Float, false, "angleY"),
             ParamData("angleX", ParamType::Float, true, "angleX"),
             ParamData("angleZ", ParamType::Float, true, "angleZ")},
            {{"angleY", "angleX", "angleZ"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                if (mod.playerClipboardMap.find(xuid) != mod.playerClipboardMap.end()) {
                    Vec3 angle(0, 0, 0);
                    if (results["angleY"].isSet) {
                        angle.y = results["angleY"].get<float>();
                    }
                    if (results["angleX"].isSet) {
                        angle.x = results["angleX"].get<float>();
                    }
                    if (results["angleZ"].isSet) {
                        angle.z = results["angleZ"].get<float>();
                    }
                    mod.playerClipboardMap[xuid].rotate(angle);
                    output.success(fmt::format("§aclipboard rotated"));
                } else {
                    output.error("You don't have a clipboard yet");
                }
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit