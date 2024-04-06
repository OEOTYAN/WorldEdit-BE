#include "WorldEdit.h"
#include "allCommand.hpp"
#include "brush/Brushs.h"
#include "filesys/download.h"
#include "store/Patterns.h"
#include "utils/StringTool.h"
#include <DynamicCommandAPI.h>
#include <mc/CommandBlockNameResult.hpp>
#include <mc/ItemStack.hpp>

namespace we {
using ParamType = DynamicCommand::ParameterType;
using ParamData = DynamicCommand::ParameterData;

// brush bmask bsize ruseface

void brushCommandSetup() {
    DynamicCommand::setup(
        "brush",                                   // command name
        tr("worldedit.command.description.brush"), // command description
        {
  // enums{enumName, {values...}}
            {"sphere",    {"sphere"}   },
            {"mix",       {"mix"}      },
            {"cyl",       {"cyl"}      },
            {"cube",      {"cube"}     },
            {"color",     {"color"}    },
            {"clipboard", {"clipboard"}},
            {"smooth",    {"smooth"}   },
            {"heightmap", {"heightmap"}},
            {"none",      {"none"}     },
            {"flat",      {"flat"}     },
            {"file",      {"file"}     },
            {"link",      {"link"}     },
    },
        {
            // parameters(type, name, [optional], [enumOptions(also
            // enumName)], [identifier]) identifier: used to identify unique
            // parameter data, if idnetifier is not set,
            //   it is set to be the same as enumOptions or name (identifier
            //   = enumOptions.empty() ? name:enumOptions)
            ParamData("sphere", ParamType::Enum, "sphere"),
            ParamData("cyl", ParamType::Enum, "cyl"),
            ParamData("mix", ParamType::Enum, "mix"),
            ParamData("cube", ParamType::Enum, "cube"),
            ParamData("color", ParamType::Enum, "color"),
            ParamData("clipboard", ParamType::Enum, "clipboard"),
            ParamData("smooth", ParamType::Enum, "smooth"),
            ParamData("flat", ParamType::Enum, "flat"),
            ParamData("heightmap", ParamType::Enum, "heightmap"),
            ParamData("none", ParamType::Enum, "none"),
            ParamData("args", ParamType::SoftEnum, true, "-ahor", "-ahor"),
            ParamData("block", ParamType::Block, "block"),
            ParamData("blockPattern", ParamType::SoftEnum, "blockPattern"),
            ParamData("radius", ParamType::Int, true, "radius"),
            ParamData("Rf", ParamType::Float, "Rf"),
            ParamData("Gf", ParamType::Float, "Gf"),
            ParamData("Bf", ParamType::Float, "Bf"),
            ParamData("mixBoxLerp", ParamType::Bool, true, "mixBoxLerp"),
            ParamData("density", ParamType::Float, true, "density"),
            ParamData("opacity", ParamType::Float, true, "opacity"),
            ParamData("kernelsize", ParamType::Int, true, "kernelsize"),
            ParamData("height", ParamType::Int, true, "height"),
            ParamData("imagefilename", ParamType::SoftEnum, "imagefilename"),
            ParamData("url", ParamType::String, "url"),
            ParamData("file", ParamType::Enum, "file"),
            ParamData("link", ParamType::Enum, "link"),
        },
        {
            // overloads{ (type == Enum ? enumOptions : name) ...}
            {"clipboard", "args"},
            {"cube", "block", "radius", "args"},
            {"cube", "blockPattern", "radius", "args"},
            {"sphere", "block", "radius", "args"},
            {"flat", "radius", "density"},
            {"sphere", "blockPattern", "radius", "args"},
            {"mix", "radius", "density", "opacity", "mixBoxLerp"},
            {"color", "Rf", "Gf", "Bf", "radius", "density", "opacity", "mixBoxLerp"},
            {"color", "block", "radius", "density", "opacity", "mixBoxLerp"},
            {"color", "blockPattern", "radius", "density", "opacity", "mixBoxLerp"},
            {"cyl", "block", "radius", "height", "args"},
            {"cyl", "blockPattern", "radius", "height", "args"},
            {"smooth", "radius", "kernelsize", "density"},
            {"heightmap", "file", "imagefilename", "radius", "height", "args"},
            {"heightmap", "link", "url", "radius", "height", "args"},
            {"none"},
        },
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
            auto*       item       = player->getHandSlot();
            std::string brushName  = item->getTypeName();
            std::string brushrName = brushName;
            if (brushName == "") {
                output.trError("worldedit.error.noitem");
                return;
            }
            stringReplace(brushrName, "minecraft:", "");
            brushName                 += std::to_string(item->getAuxValue());
            auto           xuid        = player->getXuid();
            auto&          playerData  = getPlayersData(xuid);
            unsigned short radius      = 2;
            int            ksize       = 5;
            int            height      = 1;
            if (results["radius"].isSet) {
                radius = (unsigned short)(results["radius"].get<int>());
            }
            if (results["height"].isSet) {
                height = results["height"].get<int>();
            }
            if (results["kernelsize"].isSet) {
                ksize = results["kernelsize"].get<int>();
            }

            bool arg_a = false, arg_h = false, arg_o = true, arg_r = false;
            if (results["args"].isSet) {
                auto str = results["args"].getRaw<std::string>();
                if (str.find("-") == std::string::npos) {
                    output.trError("worldedit.command.error.args", str);
                    return;
                }
                if (str.find("a") != std::string::npos) {
                    arg_a = true;
                }
                if (str.find("o") != std::string::npos) {
                    arg_o = false;
                }
                if (str.find("h") != std::string::npos) {
                    arg_h = true;
                }
                if (str.find("r") != std::string::npos) {
                    arg_r = true;
                }
            }

            float density    = 0.5f;
            float opacity    = 1.0f;
            float mixBoxLerp = true;

            if (results["density"].isSet) {
                density = results["density"].get<float>();
            }

            if (results["opacity"].isSet) {
                opacity = results["opacity"].get<float>();
            }

            if (results["mixBoxLerp"].isSet) {
                mixBoxLerp = results["mixBoxLerp"].get<bool>();
            }

            std::string bps = "minecraft:air";
            if (results["blockPattern"].isSet) {
                bps = results["blockPattern"].get<std::string>();
            } else if (results["block"].isSet) {
                bps = results["block"]
                          .get<CommandBlockName>()
                          .resolveBlock(0)
                          .getBlock()
                          ->getTypeName();
            }

            if (results["sphere"].isSet) {
                playerData.brushMap[brushName] = std::make_unique<SphereBrush>(
                    SphereBrush(radius, Pattern::createPattern(bps, xuid), arg_h)
                );
                output.trSuccess("worldedit.brush.set.sphere", brushrName);
                return;
            } else if (results["color"].isSet) {
                if (results["Rf"].isSet) {
                    playerData.brushMap[brushName] =
                        std::make_unique<ColorBrush>(ColorBrush(
                            radius,
                            density,
                            opacity,
                            mce::Color(
                                results["Rf"].get<float>(),
                                results["Gf"].get<float>(),
                                results["Bf"].get<float>()
                            ),
                            nullptr,
                            mixBoxLerp
                        ));
                } else if (isColorHex(bps)) {
                    playerData.brushMap[brushName] =
                        std::make_unique<ColorBrush>(ColorBrush(
                            radius,
                            density,
                            opacity,
                            mce::Color(bps),
                            nullptr,
                            mixBoxLerp
                        ));
                } else {
                    playerData.brushMap[brushName] =
                        std::make_unique<ColorBrush>(ColorBrush(
                            radius,
                            density,
                            opacity,
                            mce::Color(),
                            Pattern::createPattern(bps, xuid),
                            mixBoxLerp
                        ));
                }
                output.trSuccess("worldedit.brush.set.color", brushrName);
                return;
            } else if (results["mix"].isSet) {
                playerData.brushMap[brushName] = std::make_unique<MixBrush>(
                    MixBrush(radius, density, opacity, mixBoxLerp)
                );
                output.trSuccess("worldedit.brush.set.mix", brushrName);
                return;
            } else if (results["flat"].isSet) {
                playerData.brushMap[brushName] =
                    std::make_unique<FlatBrush>(FlatBrush(radius, density));
                output.trSuccess("worldedit.brush.set.flat", brushrName);
                return;
            } else if (results["cube"].isSet) {
                playerData.brushMap[brushName] = std::make_unique<CubeBrush>(
                    CubeBrush(radius, Pattern::createPattern(bps, xuid), arg_h)
                );
                output.trSuccess("worldedit.brush.set.cube", brushrName);
                return;
            } else if (results["cyl"].isSet) {
                playerData.brushMap[brushName] =
                    std::make_unique<CylinderBrush>(CylinderBrush(
                        radius,
                        Pattern::createPattern(bps, xuid),
                        height,
                        arg_h
                    ));
                output.trSuccess("worldedit.brush.set.cylinder", brushrName);
                return;
            } else if (results["clipboard"].isSet) {
                if (playerData.clipboard.used) {
                    playerData.brushMap[brushName] = std::make_unique<ClipboardBrush>(
                        ClipboardBrush(0, playerData.clipboard, arg_o, arg_a)
                    );
                } else {
                    output.trError("worldedit.error.empty-clipboard");
                    return;
                }
                output.trSuccess("worldedit.brush.set.clipboard", brushrName);
                return;
            } else if (results["smooth"].isSet) {
                playerData.brushMap[brushName] =
                    std::make_unique<SmoothBrush>(SmoothBrush(radius, ksize, density));
                output.trSuccess("worldedit.brush.set.smooth", brushrName);
                return;
            } else if (results["heightmap"].isSet) {
                std::string filename;
                if (results["imagefilename"].isSet) {
                    filename = results["imagefilename"].get<std::string>();

                    filename = WE_DIR + "image/" + filename;
                } else /* if (results["link"].isSet)*/ {
                    if (downloadImage(results["url"].get<std::string>())) {
                        filename = WE_DIR + "imgtemp/0image";
                    } else {
                        output.trError("worldedit.error.download-image");
                        return;
                    }
                }

                playerData.brushMap[brushName] = std::make_unique<ImageHeightmapBrush>(
                    ImageHeightmapBrush(radius, height, loadImage(filename), arg_r)
                );

                output.trSuccess("worldedit.brush.set.heightmap", brushrName);
                return;
            } else if (results["none"].isSet) {
                if (playerData.brushMap.contains(brushName)) {
                    playerData.brushMap.erase(brushName);
                    output.trSuccess("worldedit.brush.clear", brushrName);
                } else {
                    output.trError("worldedit.error.nobrush");
                }
                return;
            }
        },
        CommandPermissionLevel::GameMasters
    );

    DynamicCommand::setup(
        "bmask",                                   // command name
        tr("worldedit.command.description.bmask"), // command description
        {},
        {ParamData("mask", ParamType::String, true, "mask")},
        {{"mask"}},
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
            auto*       item = player->getHandSlot();
            std::string brushName =
                item->getTypeName() + std::to_string(item->getAuxValue());
            auto  xuid       = player->getXuid();
            auto& playerData = getPlayersData(xuid);
            if (playerData.brushMap.contains(brushName)) {
                auto& brush = playerData.brushMap[brushName];
                if (results["mask"].isSet) {
                    auto tmp    = results["mask"].get<std::string>();
                    brush->mask = tmp;
                    output.trSuccess("worldedit.brush.mask.set", tmp);
                } else {
                    brush->mask = "";
                    output.trSuccess("worldedit.brush.mask.clear");
                }
            } else {
                output.trError("worldedit.error.nobrush");
            }
        },
        CommandPermissionLevel::GameMasters
    );

    DynamicCommand::setup(
        "bsize",                                   // command name
        tr("worldedit.command.description.bsize"), // command description
        {},
        {ParamData("size", ParamType::Int, "size")},
        {{"size"}},
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
            auto*       item = player->getHandSlot();
            std::string brushName =
                item->getTypeName() + std::to_string(item->getAuxValue());
            auto  xuid       = player->getXuid();
            auto& playerData = getPlayersData(xuid);
            if (playerData.brushMap.contains(brushName)) {
                auto& brush = playerData.brushMap[brushName];
                auto  size  = results["size"].get<int>();
                brush->size = size;
                output.trSuccess("worldedit.brush.size.set", size);
            } else {
                output.trError("worldedit.error.nobrush");
            }
        },
        CommandPermissionLevel::GameMasters
    );

    DynamicCommand::setup(
        "ruseface",                                   // command name
        tr("worldedit.command.description.ruseface"), // command description
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
            auto*       item = player->getHandSlot();
            std::string brushName =
                item->getTypeName() + std::to_string(item->getAuxValue());
            auto  xuid       = player->getXuid();
            auto& playerData = getPlayersData(xuid);
            if (playerData.brushMap.contains(brushName)) {
                auto& brush     = playerData.brushMap[brushName];
                auto  useface   = results["bool"].get<bool>();
                brush->needFace = useface;
                output.trSuccess(
                    "worldedit.brush.ruseface.set",
                    useface ? "true" : "false"
                );
            } else {
                output.trError("worldedit.error.notb");
            }
        },
        CommandPermissionLevel::GameMasters
    );

    DynamicCommand::setup(
        "clearallbrushs",                                   // command name
        tr("worldedit.command.description.clearallbrushs"), // command description
        {},
        {},
        {{}},
        // dynamic command callback
        [](DynamicCommand const&                                    command,
           CommandOrigin const&                                     origin,
           CommandOutput&                                           output,
           std::unordered_map<std::string, DynamicCommand::Result>& results) {
            auto player = origin.getPlayer()->getXuid();
            if (getPlayersDataMap().contains(player)) {
                getPlayersDataMap()[player]->brushMap.clear();
            }
            output.trSuccess("worldedit.clearallbrushs.success");
        },
        CommandPermissionLevel::GameMasters
    );
}
} // namespace we