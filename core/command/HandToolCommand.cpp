//
// Created by OEOTYAN on 2022/05/20.
//
#include "allCommand.hpp"
#include "Global.h"
#include "string/StringTool.h"
#include "MC/ItemStack.hpp"
#include "WorldEdit.h"
#include "store/BlockPattern.hpp"

namespace worldedit {
    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;

    // tool luseface

    void handToolCommandSetup() {
        DynamicCommand::setup(
            "tool",                // command name
            "set your hand tool",  // command description
            {
                {"tree", {"tree"}},
                {"deltree", {"deltree"}},
                {"farwand", {"farwand"}},
                {"airwand", {"airwand"}},
                {"cycler", {"cycler"}},
                {"info", {"info"}},
                {"rep", {"rep"}},
                {"flood", {"flood"}},
                {"none", {"none"}},
            },
            {
                ParamData("tree", ParamType::Enum, "tree"),
                ParamData("deltree", ParamType::Enum, "deltree"),
                ParamData("farwand", ParamType::Enum, "farwand"),
                ParamData("airwand", ParamType::Enum, "airwand"),
                ParamData("cycler", ParamType::Enum, "cycler"),
                ParamData("info", ParamType::Enum, "info"),
                ParamData("rep", ParamType::Enum, "rep"),
                ParamData("flood", ParamType::Enum, "flood"),
                ParamData("none", ParamType::Enum, "none"),
                ParamData("block", ParamType::Block, "block"),
                ParamData("blockPattern", ParamType::String, "blockPattern"),
                ParamData("dis", ParamType::Int, true, "dis"),
                ParamData("needEdge", ParamType::Bool, true, "needEdge"),
            },
            {
                {"tree"},
                {"deltree"},
                {"farwand"},
                {"airwand"},
                {"cycler"},
                {"info"},
                {"rep"},
                {"flood", "block", "dis", "needEdge"},
                {"flood", "blockPattern", "dis", "needEdge"},
                {"none"},
            },
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                auto* item = origin.getPlayer()->getHandSlot();
                std::string toolrName = item->getTypeName();
                std::string toolName = toolrName + std::to_string(item->getAuxValue());
                stringReplace(toolrName, "minecraft:", "");
                if (toolName == "") {
                    output.error("You need to select an item");
                    return;
                }
                if (results["tree"].isSet) {
                    mod.playerHandToolMap[xuid][toolName] = new TreeTool();
                    output.success(fmt::format("§aTree tool bound to {}", toolrName));
                } else if (results["deltree"].isSet) {
                    mod.playerHandToolMap[xuid][toolName] = new DelTreeTool();
                    output.success(fmt::format("§aDeltree tool bound to {}", toolrName));
                } else if (results["farwand"].isSet) {
                    mod.playerHandToolMap[xuid][toolName] = new FarWand();
                    output.success(fmt::format("§aFar wand tool bound to {}", toolrName));
                } else if (results["airwand"].isSet) {
                    mod.playerHandToolMap[xuid][toolName] = new AirWand();
                    output.success(fmt::format("§aAir wand tool bound to {}", toolrName));
                } else if (results["cycler"].isSet) {
                    mod.playerHandToolMap[xuid][toolName] = new CyclerTool();
                    output.success(fmt::format("§aCycler tool bound to {}", toolrName));
                } else if (results["info"].isSet) {
                    mod.playerHandToolMap[xuid][toolName] = new InfoTool();
                    output.success(fmt::format("§aBlock info tool bound to {}", toolrName));
                } else if (results["flood"].isSet) {
                    std::string bps = "minecraft:air";
                    if (results["blockPattern"].isSet) {
                        bps = results["blockPattern"].get<std::string>();
                    } else if (results["block"].isSet) {
                        bps = results["block"].get<Block const*>()->getTypeName();
                    }

                    int radius = 5;
                    if (results["dis"].isSet) {
                        radius = results["dis"].get<int>();
                    }
                    int needEdge = false;
                    if (results["needEdge"].isSet) {
                        needEdge = results["needEdge"].get<bool>();
                    }
                    mod.playerHandToolMap[xuid][toolName] =
                        new FloodFillTool(new BlockPattern(bps, xuid, nullptr), radius, needEdge);
                    output.success(fmt::format("§aFlood fill tool bound to {}", toolrName));
                } else if (results["rep"].isSet) {
                    mod.playerHandToolMap[xuid][toolName] = new RepTool();
                    output.success(fmt::format("§aRep fill tool bound to {}", toolrName));
                } else if (results["none"].isSet) {
                    delete mod.playerHandToolMap[xuid][toolName];
                    mod.playerHandToolMap[xuid].erase(toolName);
                    output.success(fmt::format("§aHand tool cleared"));
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "luseface",                // command name
            "set left click useface",  // command description
            {}, {ParamData("bool", ParamType::Bool, "bool")}, {{"bool"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod = worldedit::getMod();
                auto player = origin.getPlayer();
                auto* item = player->getHandSlot();
                std::string toolName = item->getTypeName() + std::to_string(item->getAuxValue());
                auto xuid = player->getXuid();

                if (mod.playerHandToolMap.find(xuid) != mod.playerHandToolMap.end() &&
                    mod.playerHandToolMap[xuid].find(toolName) != mod.playerHandToolMap[xuid].end()) {
                    auto* tool = mod.playerHandToolMap[xuid][toolName];
                    auto useface = results["bool"].get<bool>();
                    tool->lneedFace = useface;
                    output.success("§aTool size set to " + std::to_string(useface));
                } else {
                    output.error("You need to choose a tool first");
                }
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit