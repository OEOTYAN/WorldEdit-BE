//
// Created by OEOTYAN on 2022/05/20.
//
#include "allCommand.hpp"
#include "Global.h"
#include "utils/StringTool.h"
#include "mc/ItemStack.hpp"
#include "WorldEdit.h"
#include <mc/CommandBlockNameResult.hpp>
#include "store/BlockPattern.hpp"
#include "brush/Brushs.h"

namespace worldedit {
    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;

    // tool luseface

    void handToolCommandSetup() {
        DynamicCommand::setup(
            "tool",                                    // command name
            tr("worldedit.command.description.tool"),  // command description
            {
                // {"tree", {"tree"}},
                // {"deltree", {"deltree"}},
                {"farwand", {"farwand"}},
                {"airwand", {"airwand"}},
                // {"cycler", {"cycler"}},
                {"info", {"info"}},
                {"rep", {"rep"}},
                {"flood", {"flood"}},
                {"none", {"none"}},
            },
            {
                // ParamData("tree", ParamType::Enum, "tree"),
                // ParamData("deltree", ParamType::Enum, "deltree"),
                ParamData("farwand", ParamType::Enum, "farwand"),
                ParamData("airwand", ParamType::Enum, "airwand"),
                // ParamData("cycler", ParamType::Enum, "cycler"),
                ParamData("info", ParamType::Enum, "info"),
                ParamData("rep", ParamType::Enum, "rep"),
                ParamData("flood", ParamType::Enum, "flood"),
                ParamData("none", ParamType::Enum, "none"),
                ParamData("block", ParamType::Block, "block"),
                ParamData("blockPattern", ParamType::SoftEnum, "blockPattern"),
                ParamData("dis", ParamType::Int, true, "dis"),
                ParamData("needEdge", ParamType::Bool, true, "needEdge"),
            },
            {
                // {"tree"},
                // {"deltree"},
                {"farwand"},
                {"airwand"},
                // {"cycler"},
                {"info"},
                {"rep"},
                {"flood", "block", "dis", "needEdge"},
                {"flood", "blockPattern", "dis", "needEdge"},
                {"none"},
            },
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                auto& playerData = getPlayersData(xuid);
                auto* item = origin.getPlayer()->getHandSlot();
                std::string toolrName = item->getTypeName();
                std::string toolName = toolrName + std::to_string(item->getAuxValue());
                stringReplace(toolrName, "minecraft:", "");
                if (toolName == "") {
                    output.trError("worldedit.error.noitem");
                    return;
                }
                if (playerData.brushMap.find(toolName) != playerData.brushMap.end()) {
                    delete playerData.brushMap[toolName];
                    playerData.brushMap[toolName] = nullptr;
                }
                // if (results["tree"].isSet) {
                //     playerData.brushMap[toolName] = new TreeTool();
                //     output.trSuccess("worldedit.tool.set.tree", toolrName);
                // } else if (results["deltree"].isSet) {
                //     playerData.brushMap[toolName] = new DelTreeTool();
                //     output.trSuccess("worldedit.tool.set.deltree", toolrName);
                // } else
                if (results["farwand"].isSet) {
                    playerData.brushMap[toolName] = new FarWand();
                    output.trSuccess("worldedit.tool.set.farwand", toolrName);
                } else if (results["airwand"].isSet) {
                    playerData.brushMap[toolName] = new AirWand();
                    output.trSuccess("worldedit.tool.set.airwand", toolrName);
                }
                // else if (results["cycler"].isSet) {
                //     playerData.brushMap[toolName] = new CyclerTool();
                //     output.trSuccess("worldedit.tool.set.cycler", toolrName);
                // }
                else if (results["info"].isSet) {
                    playerData.brushMap[toolName] = new InfoTool();
                    output.trSuccess("worldedit.tool.set.info", toolrName);
                } else if (results["flood"].isSet) {
                    std::string bps = "minecraft:air";
                    if (results["blockPattern"].isSet) {
                        bps = results["blockPattern"].get<std::string>();
                    } else if (results["block"].isSet) {
                        bps = results["block"].get<CommandBlockName>().resolveBlock(0).getBlock()->getTypeName();
                    }

                    int radius = 5;
                    if (results["dis"].isSet) {
                        radius = results["dis"].get<int>();
                    }
                    bool needEdge = false;
                    if (results["needEdge"].isSet) {
                        needEdge = results["needEdge"].get<bool>();
                    }
                    playerData.brushMap[toolName] =
                        new FloodFillTool(new BlockPattern(bps, xuid, nullptr), radius, needEdge);
                    output.trSuccess("worldedit.tool.set.flood", toolrName);
                } else if (results["rep"].isSet) {
                    playerData.brushMap[toolName] = new RepTool();
                    output.trSuccess("worldedit.tool.set.rep", toolrName);
                } else if (results["none"].isSet) {
                    playerData.brushMap.erase(toolName);
                    output.trSuccess("worldedit.tool.clear", toolrName);
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "luseface",                                    // command name
            tr("worldedit.command.description.luseface"),  // command description
            {}, {ParamData("bool", ParamType::Bool, "bool")}, {{"bool"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto player = origin.getPlayer();
                auto* item = player->getHandSlot();
                std::string toolName = item->getTypeName() + std::to_string(item->getAuxValue());
                auto xuid = player->getXuid();
                auto& playerData = getPlayersData(xuid);
                if (playerData.brushMap.find(toolName) != playerData.brushMap.end()) {
                    auto* tool = playerData.brushMap[toolName];
                    auto useface = results["bool"].get<bool>();
                    tool->lneedFace = useface;
                    output.trSuccess("worldedit.tool.luseface.set", useface ? "true" : "false");
                } else {
                    output.trError("worldedit.error.notool");
                }
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit