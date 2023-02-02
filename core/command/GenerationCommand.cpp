//
// Created by OEOTYAN on 2022/06/10.
//
#include "allCommand.hpp"
#include "builder/SimpleBuilder.h"
#include "store/Patterns.h"
#include "WorldEdit.h"
#include <mc/CommandBlockNameResult.hpp>
#include "region/Regions.h"

namespace worldedit {
    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;

    // cyl hcyl sphere hsphere

    void generationCommandSetup() {
        DynamicCommand::setup(
            "cyl",                                    // command name
            tr("worldedit.command.description.cyl"),  // command description
            {},
            {ParamData("block", ParamType::Block, "block"),
             ParamData("blockPattern", ParamType::SoftEnum, "blockPattern"),
             ParamData("radius", ParamType::Int, "radius"), ParamData("height", ParamType::Int, true, "height"),
             ParamData("args", ParamType::SoftEnum, true, "-h", "-h")},
            {{"block", "radius", "height", "args"}, {"blockPattern", "radius", "height", "args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                auto dimID = player->getDimensionId();
                auto pos = (player->getPosition() - Vec3(0.0, 1.62, 0.0)).toBlockPos();

                bool arg_h = false;
                if (results["args"].isSet) {
                    auto str = results["args"].getRaw<std::string>();
                    if (str.find("-") == std::string::npos) {
                        output.trError("worldedit.command.error.args", str);
                        return;
                    }
                    if (str.find("h") != std::string::npos) {
                        arg_h = true;
                    }
                }
                unsigned short radius = 0;
                if (results["radius"].isSet) {
                    radius = (unsigned short)(results["radius"].get<int>());
                }
                int height = 1;
                if (results["height"].isSet) {
                    height = results["height"].get<int>();
                }

                std::string bps = "minecraft:air";
                if (results["blockPattern"].isSet) {
                    bps = results["blockPattern"].get<std::string>();
                } else if (results["block"].isSet) {
                    bps = results["block"].get<CommandBlockName>().resolveBlock(0).getBlock()->getTypeName();
                }
                auto pattern = Pattern::createPattern(bps, xuid);
                long long i = SimpleBuilder::buildCylinder(pos, dimID, xuid, pattern.get(), radius, height, arg_h);
                i = std::max(0ll, i);
                output.trSuccess("worldedit.set.success", i);
            },
            CommandPermissionLevel::GameMasters);
        DynamicCommand::setup(
            "hcyl",                                    // command name
            tr("worldedit.command.description.hcyl"),  // command description
            {},
            {ParamData("block", ParamType::Block, "block"),
             ParamData("blockPattern", ParamType::SoftEnum, "blockPattern"),
             ParamData("radius", ParamType::Int, "radius"), ParamData("height", ParamType::Int, true, "height")},
            {{"block", "radius", "height"}, {"blockPattern", "radius", "height"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                auto dimID = player->getDimensionId();
                auto pos = (player->getPosition() - Vec3(0.0, 1.62, 0.0)).toBlockPos();

                bool arg_h = true;
                unsigned short radius = 0;
                if (results["radius"].isSet) {
                    radius = (unsigned short)(results["radius"].get<int>());
                }
                int height = 1;
                if (results["height"].isSet) {
                    height = results["height"].get<int>();
                }

                std::string bps = "minecraft:air";
                if (results["blockPattern"].isSet) {
                    bps = results["blockPattern"].get<std::string>();
                } else if (results["block"].isSet) {
                    bps = results["block"].get<CommandBlockName>().resolveBlock(0).getBlock()->getTypeName();
                }
                auto pattern = Pattern::createPattern(bps, xuid);
                long long i = SimpleBuilder::buildCylinder(pos, dimID, xuid, pattern.get(), radius, height, arg_h);
                i = std::max(0ll, i);
                output.trSuccess("worldedit.set.success", i);
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "sphere",                                    // command name
            tr("worldedit.command.description.sphere"),  // command description
            {},
            {ParamData("block", ParamType::Block, "block"),
             ParamData("blockPattern", ParamType::SoftEnum, "blockPattern"),
             ParamData("radius", ParamType::Int, "radius"), ParamData("args", ParamType::SoftEnum, true, "-h", "-h")},
            {{"block", "radius", "args"}, {"blockPattern", "radius", "args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                auto dimID = player->getDimensionId();
                auto pos = (player->getPosition() - Vec3(0.0, 1.62, 0.0)).toBlockPos();

                bool arg_h = false;
                if (results["args"].isSet) {
                    auto str = results["args"].getRaw<std::string>();
                    if (str.find("-") == std::string::npos) {
                        output.trError("worldedit.command.error.args", str);
                        return;
                    }
                    if (str.find("h") != std::string::npos) {
                        arg_h = true;
                    }
                }
                unsigned short radius = 0;
                if (results["radius"].isSet) {
                    radius = (unsigned short)(results["radius"].get<int>());
                }

                std::string bps = "minecraft:air";
                if (results["blockPattern"].isSet) {
                    bps = results["blockPattern"].get<std::string>();
                } else if (results["block"].isSet) {
                    bps = results["block"].get<CommandBlockName>().resolveBlock(0).getBlock()->getTypeName();
                }
                auto pattern = Pattern::createPattern(bps, xuid);
                long long i = SimpleBuilder::buildSphere(pos, dimID, xuid, pattern.get(), radius, arg_h);
                i = std::max(0ll, i);
                output.trSuccess("worldedit.set.success", i);
            },
            CommandPermissionLevel::GameMasters);
        DynamicCommand::setup(
            "hsphere",                                    // command name
            tr("worldedit.command.description.hsphere"),  // command description
            {},
            {ParamData("block", ParamType::Block, "block"),
             ParamData("blockPattern", ParamType::SoftEnum, "blockPattern"),
             ParamData("radius", ParamType::Int, "radius")},
            {{"block", "radius"}, {"blockPattern", "radius"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto player = origin.getPlayer();
                auto xuid = player->getXuid();
                auto dimID = player->getDimensionId();
                auto pos = (player->getPosition() - Vec3(0.0, 1.62, 0.0)).toBlockPos();

                bool arg_h = true;
                unsigned short radius = 0;
                if (results["radius"].isSet) {
                    radius = (unsigned short)(results["radius"].get<int>());
                }

                std::string bps = "minecraft:air";
                if (results["blockPattern"].isSet) {
                    bps = results["blockPattern"].get<std::string>();
                } else if (results["block"].isSet) {
                    bps = results["block"].get<CommandBlockName>().resolveBlock(0).getBlock()->getTypeName();
                }
                auto pattern = Pattern::createPattern(bps, xuid);
                long long i = SimpleBuilder::buildSphere(pos, dimID, xuid, pattern.get(), radius, arg_h);
                i = std::max(0ll, i);
                output.trSuccess("worldedit.set.success", i);
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit