//
// Created by OEOTYAN on 2022/06/10.
//
#pragma once
#ifndef WORLDEDIT_GENERATIONCOMMAND_H
#define WORLDEDIT_GENERATIONCOMMAND_H

#include "builder/SimpleBuilder.h"
#include "store/BlockPattern.hpp"

namespace worldedit {
    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;

    // cyl hcyl sphere hsphere

    void generationCommandSetup() {
        DynamicCommand::setup(
            "cyl",           // command name
            "set cylinder",  // command description
            {},
            {ParamData("block", ParamType::Block, "block"),
             ParamData("blockPattern", ParamType::String, "blockPattern"),
             ParamData("radius", ParamType::Int, "radius"), ParamData("height", ParamType::Int, true, "height"),
             ParamData("args", ParamType::String, true, "-h")},
            {{"block", "radius", "height", "args"}, {"blockPattern", "radius", "height", "args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod    = worldedit::getMod();
                auto  player = origin.getPlayer();
                auto  xuid   = player->getXuid();
                auto  dimID  = player->getDimensionId();
                auto  pos    = (player->getPosition() - Vec3(0, 1, 0)).toBlockPos();
                pos.y -= 1;

                bool arg_h = false;
                if (results["args"].isSet) {
                    auto str = results["args"].getRaw<std::string>();
                    if (str.find("-") == std::string::npos) {
                        output.error("wrong args");
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
                    bps = results["block"].get<Block const*>()->getTypeName();
                }
                CuboidRegion region(BoundingBox(pos, pos), dimID);
                BlockPattern blockPattern(bps, xuid, &region);
                long long    i = SimpleBuilder::buildCylinder(pos, dimID, xuid, &blockPattern, radius, height, arg_h);
                i              = std::max(0ll, i);
                output.success(fmt::format("§a{} block(s) placed", i));
            },
            CommandPermissionLevel::GameMasters);
        DynamicCommand::setup(
            "hcyl",                   // command name
            "set hollowed cylinder",  // command description
            {},
            {ParamData("block", ParamType::Block, "block"),
             ParamData("blockPattern", ParamType::String, "blockPattern"),
             ParamData("radius", ParamType::Int, "radius"), ParamData("height", ParamType::Int, true, "height")},
            {{"block", "radius", "height"}, {"blockPattern", "radius", "height"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod    = worldedit::getMod();
                auto  player = origin.getPlayer();
                auto  xuid   = player->getXuid();
                auto  dimID  = player->getDimensionId();
                auto pos = (player->getPosition() - Vec3(0, 1, 0)).toBlockPos();
                pos.y -= 1;

                bool           arg_h  = true;
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
                    bps = results["block"].get<Block const*>()->getTypeName();
                }
                CuboidRegion region(BoundingBox(pos, pos), dimID);
                BlockPattern blockPattern(bps, xuid, &region);
                long long    i = SimpleBuilder::buildCylinder(pos, dimID, xuid, &blockPattern, radius, height, arg_h);
                i              = std::max(0ll, i);
                output.success(fmt::format("§a{} block(s) placed", i));
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "sphere",      // command name
            "set sphere",  // command description
            {},
            {ParamData("block", ParamType::Block, "block"),
             ParamData("blockPattern", ParamType::String, "blockPattern"),
             ParamData("radius", ParamType::Int, "radius"), ParamData("args", ParamType::String, true, "-h")},
            {{"block", "radius", "args"}, {"blockPattern", "radius", "args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod    = worldedit::getMod();
                auto  player = origin.getPlayer();
                auto  xuid   = player->getXuid();
                auto  dimID  = player->getDimensionId();
                auto pos = (player->getPosition() - Vec3(0, 1, 0)).toBlockPos();
                pos.y -= 1;

                bool arg_h = false;
                if (results["args"].isSet) {
                    auto str = results["args"].getRaw<std::string>();
                    if (str.find("-") == std::string::npos) {
                        output.error("wrong args");
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
                    bps = results["block"].get<Block const*>()->getTypeName();
                }
                CuboidRegion region(BoundingBox(pos, pos), dimID);
                BlockPattern blockPattern(bps, xuid, &region);
                long long    i = SimpleBuilder::buildSphere(pos, dimID, xuid, &blockPattern, radius, arg_h);
                i              = std::max(0ll, i);
                output.success(fmt::format("§a{} block(s) placed", i));
            },
            CommandPermissionLevel::GameMasters);
        DynamicCommand::setup(
            "hsphere",              // command name
            "set hollowed sphere",  // command description
            {},
            {ParamData("block", ParamType::Block, "block"),
             ParamData("blockPattern", ParamType::String, "blockPattern"),
             ParamData("radius", ParamType::Int, "radius"), ParamData("args", ParamType::String, true, "-h")},
            {{"block", "radius", "args"}, {"blockPattern", "radius", "args"}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>& results) {
                auto& mod    = worldedit::getMod();
                auto  player = origin.getPlayer();
                auto  xuid   = player->getXuid();
                auto  dimID  = player->getDimensionId();
                auto pos = (player->getPosition() - Vec3(0, 1, 0)).toBlockPos();
                pos.y -= 1;

                bool           arg_h  = true;
                unsigned short radius = 0;
                if (results["radius"].isSet) {
                    radius = (unsigned short)(results["radius"].get<int>());
                }

                std::string bps = "minecraft:air";
                if (results["blockPattern"].isSet) {
                    bps = results["blockPattern"].get<std::string>();
                } else if (results["block"].isSet) {
                    bps = results["block"].get<Block const*>()->getTypeName();
                }
                CuboidRegion region(BoundingBox(pos, pos), dimID);
                BlockPattern blockPattern(bps, xuid, &region);
                long long    i = SimpleBuilder::buildSphere(pos, dimID, xuid, &blockPattern, radius, arg_h);
                i              = std::max(0ll, i);
                output.success(fmt::format("§a{} block(s) placed", i));
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit

#endif  // WORLDEDIT_GENERATIONCOMMAND_H