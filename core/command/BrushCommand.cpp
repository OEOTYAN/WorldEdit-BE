//
// Created by OEOTYAN on 2022/06/10.
//
#include "allCommand.hpp"
#include <DynamicCommandAPI.h>
#include "brush/Brushs.h"
#include <MC/ItemStack.hpp>
#include "store/BlockPattern.hpp"
#include "filesys/download.h"
#include "WorldEdit.h"
#include "string/StringTool.h"

namespace worldedit {
    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;

    // brush bmask bsize ruseface

    void brushCommandSetup() {
        DynamicCommand::setup(
            "brush",                                    // command name
            tr("worldedit.command.description.brush"),  // command description
            {
                // enums{enumName, {values...}}
                {"sphere", {"sphere"}},
                {"cyl", {"cyl"}},
                {"cube", {"cube"}},
                {"clipboard", {"clipboard"}},
                {"smooth", {"smooth"}},
                {"gravity", {"gravity"}},
                {"heightmap", {"heightmap"}},
                {"none", {"none"}},
                {"file", {"file"}},
                {"link", {"link"}},
            },
            {
                // parameters(type, name, [optional], [enumOptions(also
                // enumName)], [identifier]) identifier: used to identify unique
                // parameter data, if idnetifier is not set,
                //   it is set to be the same as enumOptions or name (identifier
                //   = enumOptions.empty() ? name:enumOptions)
                ParamData("sphere", ParamType::Enum, "sphere"),
                ParamData("cyl", ParamType::Enum, "cyl"),
                ParamData("cube", ParamType::Enum, "cube"),
                ParamData("clipboard", ParamType::Enum, "clipboard"),
                ParamData("smooth", ParamType::Enum, "smooth"),
                ParamData("gravity", ParamType::Enum, "gravity"),
                ParamData("heightmap", ParamType::Enum, "heightmap"),
                ParamData("none", ParamType::Enum, "none"),
                ParamData("args", ParamType::SoftEnum, true, "-aho", "-aho"),
                ParamData("block", ParamType::Block, "block"),
                ParamData("blockPattern", ParamType::String, "blockPattern"),
                ParamData("radius", ParamType::Int, true, "radius"),
                ParamData("kernelsize", ParamType::Int, true, "kernelsize"),
                ParamData("height", ParamType::Int, true, "height"),
                ParamData("filename", ParamType::SoftEnum, "filename"),
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
                {"sphere", "blockPattern", "radius", "args"},
                {"cyl", "block", "radius", "height", "args"},
                {"cyl", "blockPattern", "radius", "height", "args"},
                {"smooth", "radius", "kernelsize"},
                {"gravity", "args"},
                {"heightmap", "file", "filename", "radius", "height", "args"},
                {"heightmap", "link", "url", "radius", "height", "args"},
                {"none"},
            },
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto player = origin.getPlayer();
                auto* item = player->getHandSlot();
                std::string brushName = item->getTypeName();
                std::string brushrName = brushName;
                if (brushName == "") {
                    output.trError("worldedit.error.noitem");
                    return;
                }
                stringReplace(brushrName, "minecraft:", "");
                brushName += std::to_string(item->getAuxValue());
                auto xuid = player->getXuid();
                auto& playerData = getPlayersData(xuid);
                if (playerData.brushMap.find(brushName) != playerData.brushMap.end()) {
                    delete playerData.brushMap[brushName];
                    playerData.brushMap[brushName] = nullptr;
                }

                unsigned short radius = 2;
                int ksize = 5;
                int height = 1;
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

                std::string bps = "minecraft:air";
                if (results["blockPattern"].isSet) {
                    bps = results["blockPattern"].get<std::string>();
                } else if (results["block"].isSet) {
                    bps = results["block"].get<Block const*>()->getTypeName();
                }

                if (results["sphere"].isSet) {
                    playerData.brushMap[brushName] =
                        new SphereBrush(radius, new BlockPattern(bps, xuid, nullptr), arg_h);
                    output.trSuccess("worldedit.brush.set.sphere", brushrName);
                    return;
                } else if (results["cube"].isSet) {
                    playerData.brushMap[brushName] = new CubeBrush(radius, new BlockPattern(bps, xuid, nullptr), arg_h);
                    output.trSuccess("worldedit.brush.set.cube", brushrName);
                    return;
                } else if (results["cyl"].isSet) {
                    playerData.brushMap[brushName] =
                        new CylinderBrush(radius, new BlockPattern(bps, xuid, nullptr), height, arg_h);
                    output.trSuccess("worldedit.brush.set.cylinder", brushrName);
                    return;
                } else if (results["clipboard"].isSet) {
                    if (playerData.clipboard.used) {
                        playerData.brushMap[brushName] = new ClipboardBrush(0, playerData.clipboard, arg_o, arg_a);
                    } else {
                        output.trError("worldedit.error.empty-clipboard");
                        return;
                    }
                    output.trSuccess("worldedit.brush.set.clipboard", brushrName);
                    return;
                } else if (results["smooth"].isSet) {
                    playerData.brushMap[brushName] = new SmoothBrush(radius, ksize);
                    output.trSuccess("worldedit.brush.set.smooth",brushrName);
                    return;
                } else if (results["gravity"].isSet) {
                    output.trSuccess("worldedit.brush.set.gravity", brushrName);
                    return;
                } else if (results["heightmap"].isSet) {
                    std::string filename;
                    if (results["filename"].isSet) {
                        filename = results["filename"].get<std::string>();

                        if (filename.find(".png") == std::string::npos) {
                            filename += ".png";
                        }

                        filename = WE_DIR + "image/" + filename;
                    } else /* if (results["link"].isSet)*/ {
                        if (downloadImage(results["url"].get<std::string>())) {
                            filename = WE_DIR + "imgtemp/0.png";
                        } else {
                            output.trError("worldedit.error.download-image");
                            return;
                        }
                    }

                    playerData.brushMap[brushName] = new ImageHeightmapBrush(radius, height, loadpng(filename), arg_r);

                    output.trSuccess("worldedit.brush.set.heightmap", brushrName);
                    return;
                } else if (results["none"].isSet) {
                    playerData.brushMap.erase(brushName);
                    output.trSuccess("worldedit.brush.clear", brushrName);
                    return;
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "bmask",                                    // command name
            tr("worldedit.command.description.bmask"),  // command description
            {}, {ParamData("mask", ParamType::String, true, "mask")}, {{"mask"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto player = origin.getPlayer();
                auto* item = player->getHandSlot();
                std::string brushName = item->getTypeName() + std::to_string(item->getAuxValue());
                auto xuid = player->getXuid();
                auto& playerData = getPlayersData(xuid);
                if (playerData.brushMap.find(brushName) != playerData.brushMap.end()) {
                    auto* brush = playerData.brushMap[brushName];
                    if (results["mask"].isSet) {
                        auto tmp = results["mask"].get<std::string>();
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
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "bsize",                                    // command name
            tr("worldedit.command.description.bsize"),  // command description
            {}, {ParamData("size", ParamType::Int, "size")}, {{"size"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto player = origin.getPlayer();
                auto* item = player->getHandSlot();
                std::string brushName = item->getTypeName() + std::to_string(item->getAuxValue());
                auto xuid = player->getXuid();
                auto& playerData = getPlayersData(xuid);
                if (playerData.brushMap.find(brushName) != playerData.brushMap.end()) {
                    auto* brush = playerData.brushMap[brushName];
                    auto size = results["size"].get<int>();
                    brush->size = size;
                    output.trSuccess("worldedit.brush.size.set", size);
                } else {
                    output.trError("worldedit.error.nobrush");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "ruseface",                                    // command name
            tr("worldedit.command.description.ruseface"),  // command description
            {}, {ParamData("bool", ParamType::Bool, "bool")}, {{"bool"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto player = origin.getPlayer();
                auto* item = player->getHandSlot();
                std::string brushName = item->getTypeName() + std::to_string(item->getAuxValue());
                auto xuid = player->getXuid();
                auto& playerData = getPlayersData(xuid);
                if (playerData.brushMap.find(brushName) != playerData.brushMap.end()) {
                    auto* brush = playerData.brushMap[brushName];
                    auto useface = results["bool"].get<bool>();
                    brush->needFace = useface;
                    output.trSuccess("worldedit.brush.ruseface.set", useface ? "true" : "false");
                } else {
                    output.trError("worldedit.error.notb");
                }
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit