// #include "Globals.h"
#include "WorldEdit.h"
#include "allCommand.hpp"
#include "data/PlayerData.h"
#include "eval/Eval.h"
#include "mc/StructureTemplate.hpp"
#include "region/Regions.h"
#include "store/BlockNBTSet.hpp"
#include "store/Clipboard.hpp"
#include <mc/BlockSource.hpp>
#include <mc/CommandUtils.hpp>

namespace we {
using ParamType = DynamicCommand::ParameterType;
using ParamData = DynamicCommand::ParameterData;

// copy cut paste rotate flip clearclipboard

void clipboardCommandSetup() {
    DynamicCommand::setup(
        "clearclipboard",                                   // command name
        tr("worldedit.command.description.clearclipboard"), // command description
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
            auto  xuid           = player->getXuid();
            auto& playerData     = getPlayersData(xuid);
            playerData.clipboard = std::move(Clipboard());
            output.trSuccess("worldedit.clipboard.clear");
        },
        CommandPermissionLevel::GameMasters
    );

    DynamicCommand::setup(
        "copy",                                   // command name
        tr("worldedit.command.description.copy"), // command description
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
            auto  xuid       = player->getXuid();
            auto& playerData = getPlayersData(xuid);
            if (playerData.region != nullptr && playerData.region->hasSelected()) {
                auto& region      = playerData.region;
                auto  boundingBox = region->getBoundBox();

                playerData.clipboard =
                    std::move(std::move(Clipboard(boundingBox.max - boundingBox.min)));
                auto pPos                         = origin.getWorldPosition();
                playerData.clipboard.playerPos    = pPos.toBlockPos();
                playerData.clipboard.playerRelPos = pPos.toBlockPos() - boundingBox.min;
                auto dimID                        = region->getDimensionID();
                auto blockSource                  = &player->getDimensionBlockSource();
                region->forEachBlockInRegion([&](BlockPos const& pos) {
                    auto localPos      = pos - boundingBox.min;
                    auto blockInstance = blockSource->getBlockInstance(pos);
                    playerData.clipboard.storeBlock(blockInstance, localPos);
                });
                if (true) {
                    auto st = StructureTemplate(
                        "worldedit_copy_cmd_tmp",
                        dAccess<
                            Bedrock::NonOwnerPointer<class IUnknownBlockTypeRegistry>,
                            192>(Global<StructureManager>)
                    );

                    auto setting = StructureSettings();
                    setting.setIgnoreBlocks(true);
                    setting.setIgnoreEntities(false);
                    setting.setMirror(Mirror::None);
                    setting.setStructureSize(boundingBox.max - boundingBox.min + 1);
                    setting.setRotation(Rotation::None);
                    st.fillFromWorld(
                        *blockSource,
                        boundingBox.min + BlockPos(0, 1, 0),
                        setting
                    );
                    // auto structure = st.toTag()->toBinaryNBT();
                    // std::cout << structure << std::endl;
                    boundingBox += BlockPos(0, 1, 0);
                    playerData.clipboard.entities =
                        //  StructureTemplate::fromWorld("worldedit_copy_cmd_tmp",
                        //  dimID,
                        //  boundingBox.min, boundingBox.max,
                        //  false, true)
                        st.toTag();
                }
                output.trSuccess("worldedit.clipboard.copy");
            } else {
                output.trError("worldedit.error.incomplete-region");
            }
        },
        CommandPermissionLevel::GameMasters
    );

    DynamicCommand::setup(
        "cut",                                   // command name
        tr("worldedit.command.description.cut"), // command description
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
            auto  xuid       = player->getXuid();
            auto& playerData = getPlayersData(xuid);
            if (playerData.region != nullptr && playerData.region->hasSelected()) {
                auto& region      = playerData.region;
                auto  dimID       = region->getDimensionID();
                auto  boundingBox = region->getBoundBox();
                auto  blockSource = &player->getDimensionBlockSource();

                if (playerData.maxHistoryLength > 0) {
                    auto& history = playerData.getNextHistory();
                    history = std::move(Clipboard(boundingBox.max - boundingBox.min));
                    history.playerRelPos.x = dimID;
                    history.playerPos      = boundingBox.min;

                    region->forEachBlockInRegion([&](BlockPos const& pos) {
                        auto localPos      = pos - boundingBox.min;
                        auto blockInstance = blockSource->getBlockInstance(pos);
                        history.storeBlock(blockInstance, localPos);
                    });
                }

                auto pPos = origin.getWorldPosition();

                playerData.clipboard =
                    std::move(std::move(Clipboard(boundingBox.max - boundingBox.min)));
                playerData.clipboard.playerPos    = pPos.toBlockPos();
                playerData.clipboard.playerRelPos = pPos.toBlockPos() - boundingBox.min;
                region->forEachBlockInRegion([&](BlockPos const& pos) {
                    auto localPos      = pos - boundingBox.min;
                    auto blockInstance = blockSource->getBlockInstance(pos);
                    playerData.clipboard.storeBlock(blockInstance, localPos);
                });

                EvalFunctions f;
                f.setbs(blockSource);
                f.setbox(boundingBox);
                phmap::flat_hash_map<std::string, double> variables;
                playerData.setVarByPlayer(variables);
                auto playerPos = origin.getWorldPosition();
                Vec3 center    = boundingBox.getCenter().toVec3();

                region->forEachBlockInRegion([&](BlockPos const& pos) {
                    setFunction(variables, f, boundingBox, playerPos, pos, center);
                    playerData.setBlockSimple(blockSource, f, variables, pos);
                });
                output.trSuccess("worldedit.clipboard.cut");
            } else {
                output.trError("worldedit.error.incomplete-region");
            }
        },
        CommandPermissionLevel::GameMasters
    );

    DynamicCommand::setup(
        "paste",                                   // command name
        tr("worldedit.command.description.paste"), // command description
        {},
        {ParamData("args", ParamType::SoftEnum, true, "-anose", "-anose")},
        {{"args"}},
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
            if (playerData.clipboard.used) {
                long long i = 0;

                bool arg_a = false, arg_n = false, arg_o = false, arg_s = false,
                     arg_e = false;
                if (results["args"].isSet) {
                    auto str = results["args"].getRaw<std::string>();
                    if (str.find("-") == std::string::npos) {
                        output.trError("worldedit.command.error.args", str);
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
                    pbPos = playerData.clipboard.playerPos;
                } else {
                    auto pPos = origin.getWorldPosition();
                    pbPos     = pPos.toBlockPos();
                }

                BoundingBox box = playerData.clipboard.getBoundingBox() + pbPos;

                auto playerRot = origin.getRotation().value_or(Vec2::UNIT_X);
                ;
                EvalFunctions f;
                f.setbs(&player->getDimensionBlockSource());
                f.setbox(box);
                phmap::flat_hash_map<std::string, double> variables;
                playerData.setVarByPlayer(variables);

                BlockInstance blockInstance;

                auto dimID = origin.getPlayer()->getDimensionId();
                if (arg_s) {
                    if (playerData.region != nullptr
                        && playerData.region->hasSelected()) {
                        playerData.region = nullptr;
                    }
                    blockInstance = Level::getBlockInstance(box.min, dimID);
                    playerData.changeMainPos(blockInstance, false);
                    blockInstance = Level::getBlockInstance(box.max, dimID);
                    playerData.changeVicePos(blockInstance, false);
                }

                auto blockSource = &player->getDimensionBlockSource();
                if (!arg_n) {
                    if (playerData.maxHistoryLength > 0) {
                        auto& history          = playerData.getNextHistory();
                        history                = std::move(Clipboard(box.max - box.min));
                        history.playerRelPos.x = dimID;
                        history.playerPos      = box.min;

                        box.forEachBlockInBox([&](BlockPos const& pos) {
                            auto localPos      = pos - box.min;
                            auto blockInstance = blockSource->getBlockInstance(pos);
                            history.storeBlock(blockInstance, localPos);
                        });
                    }

                    Vec3 center = (pbPos + box.getCenter()).toVec3();

                    auto playerPos = origin.getWorldPosition();
                    if (arg_a) {
                        playerData.clipboard.forEachBlockInClipboard([&](BlockPos const&
                                                                             pos) {
                            if (playerData.clipboard.getSet(pos).getBlock()
                                    == BedrockBlocks::mAir
                                && playerData.clipboard.getSet(pos).getExBlock()
                                       == BedrockBlocks::mAir) {
                                return;
                            }
                            auto worldPos = playerData.clipboard.getPos(pos) + pbPos;
                            setFunction(variables, f, box, playerPos, worldPos, center);
                            i += playerData.clipboard.setBlocks(
                                pos,
                                worldPos,
                                blockSource,
                                playerData,
                                f,
                                variables
                            );
                        });
                    } else {
                        playerData.clipboard.forEachBlockInClipboard([&](BlockPos const&
                                                                             pos) {
                            auto worldPos = playerData.clipboard.getPos(pos) + pbPos;
                            setFunction(variables, f, box, playerPos, worldPos, center);

                            i += playerData.clipboard.setBlocks(
                                pos,
                                worldPos,
                                blockSource,
                                playerData,
                                f,
                                variables
                            );
                        });
                    }
                }
                if (arg_e && playerData.clipboard.entities != nullptr) {
                    auto st = StructureTemplate(
                        "worldedit_paste_cmd_tmp",
                        dAccess<
                            Bedrock::NonOwnerPointer<class IUnknownBlockTypeRegistry>,
                            192>(Global<StructureManager>)
                    );
                    st.getData()->load(*playerData.clipboard.entities);
                    auto& palette = Global<Level>->getBlockPalette();
                    auto  setting = StructureSettings();
                    setting.setMirror(playerData.clipboard.mirror);
                    setting.setStructureSize(st.getSize());
                    setting.setRotation(playerData.clipboard.rotation);
                    st.placeInWorld(
                        *blockSource,
                        palette,
                        playerData.clipboard.getPos({0, 0, 0}) + pbPos
                            + BlockPos(0, 1, 0),
                        setting,
                        nullptr,
                        false
                    );
                }

                output.trSuccess("worldedit.clipboard.paste", i);
            } else {
                output.trError("worldedit.error.empty-clipboard");
            }
        },
        CommandPermissionLevel::GameMasters
    );

    DynamicCommand::setup(
        "flip",                                   // command name
        tr("worldedit.command.description.flip"), // command description
        {
            {"dir", {"me", "up", "down", "south", "north", "east", "west"}},
    },
        {ParamData("dir", ParamType::Enum, true, "dir")},
        {{"dir"}},
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
            if (playerData.clipboard.used) {
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
                playerData.clipboard.flip(facing);
                output.trSuccess("worldedit.clipboard.flip");
            } else {
                output.trError("worldedit.error.empty-clipboard");
            }
        },
        CommandPermissionLevel::GameMasters
    );

    DynamicCommand::setup(
        "rotate",                                   // command name
        tr("worldedit.command.description.rotate"), // command description
        {
    },
        {ParamData("angleY", ParamType::Float, false, "angleY"),
         ParamData("angleX", ParamType::Float, true, "angleX"),
         ParamData("angleZ", ParamType::Float, true, "angleZ")},
        {{"angleY", "angleX", "angleZ"}},
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
            if (playerData.clipboard.used) {
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
                playerData.clipboard.rotate(angle);
                output.trSuccess("worldedit.clipboard.rotate");
            } else {
                output.trError("worldedit.error.empty-clipboard");
            }
        },
        CommandPermissionLevel::GameMasters
    );
}
} // namespace we