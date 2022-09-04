//
// Created by OEOTYAN on 2022/05/20.
//
#include "allCommand.hpp"
#include "Global.h"
#include "string/StringTool.h"
#include "MC/ItemStack.hpp"
#include "WorldEdit.h"

namespace worldedit {
    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;

    // tool

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
                ParamData("flood", ParamType::Enum, "flood"),
                ParamData("none", ParamType::Enum, "none"),
                ParamData("block", ParamType::Block, "block"),
                ParamData("blockPattern", ParamType::String, "blockPattern"),
                ParamData("distance", ParamType::Float, "distance"),
            },
            {
                {"tree"},
                {"deltree"},
                {"farwand"},
                {"airwand"},
                {"cycler"},
                {"info"},
                {"flood", "block", "distance"},
                {"flood", "blockPattern", "distance"},
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
                    mod.playerHandToolMap[xuid][toolName] = new FloodFillTool();
                    output.success(fmt::format("§aFlood fill tool bound to {}", toolrName));
                } else if (results["none"].isSet) {
                    delete mod.playerHandToolMap[xuid][toolName];
                    mod.playerHandToolMap[xuid].erase(toolName);
                    output.success(fmt::format("§aHand tool cleared"));
                }
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit