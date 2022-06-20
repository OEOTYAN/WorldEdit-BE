//
// Created by OEOTYAN on 2022/06/10.
//
#pragma once
#ifndef WORLDEDIT_BRUSHCOMMAND_H
#define WORLDEDIT_BRUSHCOMMAND_H

#include <DynamicCommandAPI.h>
#include "brush/Brushs.h"
#include <MC/ItemStack.hpp>
#include "store/BlockPattern.hpp"

namespace worldedit {
    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;

    // brush

    void brushCommandSetup() {
        DynamicCommand::setup(
            "brush",  // command name
            "brush",  // command description
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
                ParamData("args", ParamType::String, true, "-aho"),
                ParamData("block", ParamType::Block, "block"),
                ParamData("blockPattern", ParamType::String, "blockPattern"),
                ParamData("radius", ParamType::Int, true, "radius"),
                ParamData("kernelsize", ParamType::Int, true, "kernelsize"),
                ParamData("height", ParamType::Int, true, "height"),
                ParamData("filename", ParamType::String, "filename"),
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
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                std::string brushName = player->getHandSlot()->getNbt()->toBinaryNBT();
                if (brushName == "") {
                    output.error("You need to select an item");
                    return;
                }
                auto xuid = player->getXuid();
                if (mod.playerBrushMap.find(xuid) != mod.playerBrushMap.end() &&
                    mod.playerBrushMap[xuid].find(brushName) != mod.playerBrushMap[xuid].end()) {
                    delete mod.playerBrushMap[xuid][brushName];
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
                        output.error("wrong args");
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
                    mod.playerBrushMap[xuid][brushName] =
                        new SphereBrush(radius, new BlockPattern(bps, xuid, nullptr), arg_h);
                    output.success("§aBrush set to sphere");
                    return;
                } else if (results["cube"].isSet) {
                    mod.playerBrushMap[xuid][brushName] =
                        new CubeBrush(radius, new BlockPattern(bps, xuid, nullptr), arg_h);
                    output.success("§aBrush set to cube");
                    return;
                } else if (results["cyl"].isSet) {
                    mod.playerBrushMap[xuid][brushName] =
                        new CylinderBrush(radius, new BlockPattern(bps, xuid, nullptr), height, arg_h);
                    output.success("§aBrush set to cylinder");
                    return;
                } else if (results["clipboard"].isSet) {
                    if (mod.playerClipboardMap.find(xuid) != mod.playerClipboardMap.end()) {
                        mod.playerBrushMap[xuid][brushName] =
                            new ClipboardBrush(0, mod.playerClipboardMap[xuid], arg_o, arg_a);
                    } else {
                        output.error("You don't have a clipboard yet");
                        return;
                    }
                    output.success("§aBrush set to clipboard");
                    return;
                } else if (results["smooth"].isSet) {
                    mod.playerBrushMap[xuid][brushName] = new SmoothBrush(radius, ksize);
                    output.success("§aBrush set to smooth");
                    return;
                } else if (results["gravity"].isSet) {
                    output.success("§aBrush set to gravity");
                    return;
                } else if (results["heightmap"].isSet) {
                    auto filename = results["filename"].get<std::string>();

                    if (filename.find(".png") == std::string::npos) {
                        filename += ".png";
                    }

                    filename = WE_DIR + "image/" + filename;

                    mod.playerBrushMap[xuid][brushName] =
                        new ImageHeightmapBrush(radius, height, loadpng(filename), arg_r);

                    output.success("§aBrush set to image heightmap");
                    return;
                } else if (results["none"].isSet) {
                    mod.playerBrushMap[xuid].erase(brushName);
                    output.success("§aBrush unbinded");
                    return;
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "bmask",                  // command name
            "set your brush's mask",  // command description
            {}, {ParamData("mask", ParamType::String, true, "mask")}, {{"mask"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                std::string brushName = player->getHandSlot()->getNbt()->toBinaryNBT();
                auto xuid = player->getXuid();
                if (mod.playerBrushMap.find(xuid) != mod.playerBrushMap.end() &&
                    mod.playerBrushMap[xuid].find(brushName) != mod.playerBrushMap[xuid].end()) {
                    auto* brush = mod.playerBrushMap[xuid][brushName];
                    if (results["mask"].isSet) {
                        auto tmp = results["mask"].get<std::string>();
                        brush->mask = tmp;
                        output.success("§aBrush mask set to §g" + tmp);
                    } else {
                        brush->mask = "";
                        output.success("§aBrush mask disabled");
                    }
                } else {
                    output.error("You need to choose a brush first");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "bsize",                  // command name
            "set your brush's size",  // command description
            {}, {ParamData("size", ParamType::Int, "size")}, {{"size"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                std::string brushName = player->getHandSlot()->getNbt()->toBinaryNBT();
                auto xuid = player->getXuid();
                if (mod.playerBrushMap.find(xuid) != mod.playerBrushMap.end() &&
                    mod.playerBrushMap[xuid].find(brushName) != mod.playerBrushMap[xuid].end()) {
                    auto* brush = mod.playerBrushMap[xuid][brushName];
                    auto size = results["size"].get<int>();
                    brush->size = size;
                    output.success("§aBrush size set to " + std::to_string(size));
                } else {
                    output.error("You need to choose a brush first");
                }
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit

#endif  // WORLDEDIT_BRUSHCOMMAND_H