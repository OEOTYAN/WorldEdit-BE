//
// Created by OEOTYAN on 2022/05/16.
//
#pragma once
#ifndef WORLDEDIT_ALLCOMMAND_H
#define WORLDEDIT_ALLCOMMAND_H
#include "pch.h"
#include <EventAPI.h>
#include <LoggerAPI.h>
#include <MC/Level.hpp>
#include <MC/BlockInstance.hpp>
#include <MC/Block.hpp>
#include <MC/BlockSource.hpp>
#include <MC/Actor.hpp>
#include <MC/Player.hpp>
#include <MC/ServerPlayer.hpp>
#include <MC/ItemStack.hpp>
#include "Version.h"
#include "eval/Eval.h"
#include "string/StringTool.h"
#include <LLAPI.h>
#include <ServerAPI.h>
#include <EventAPI.h>
#include <ScheduleAPI.h>
#include <DynamicCommandAPI.h>
#include "particle/Graphics.h"
#include "subscribe/subscribeall.hpp"
#include "WorldEdit.h"

namespace worldedit {
    // Direct setup of dynamic command with necessary information
    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;
    void commandsSetup() {
        DynamicCommand::setup(
            "sel",                  // command name
            "change/clear region",  // command description
            {
                // enums{enumName, {values...}}
                {"region",
                 {"cuboid", "expand", "poly", "sphere", "convex", "clear"}},
            },
            {
                // parameters(type, name, [optional], [enumOptions(also
                // enumName)], [identifier]) identifier: used to identify unique
                // parameter data, if idnetifier is not set,
                //   it is set to be the same as enumOptions or name (identifier
                //   = enumOptions.empty() ? name:enumOptions)
                ParamData("region", ParamType::Enum, true, "region"),
            },
            {
                // overloads{ (type == Enum ? enumOptions : name) ...}
                {"region"},  // testenum <add|remove>
            },
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& action = results["region"].get<std::string&>();
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                if (action == "clear") {
                    if (mod.playerRegionMap.count(xuid) > 0) {
                        mod.playerRegionMap[xuid]->selecting = false;
                    }
                    output.success("§aRegion has been cleared");
                } else {
                    BoundingBox tmpbox;
                    if (mod.playerRegionMap.count(xuid) > 0) {
                        tmpbox = mod.playerRegionMap[xuid]->getBoundBox();
                        delete mod.playerRegionMap[xuid];
                    }
                    switch (do_hash(action.c_str())) {
                        case do_hash("cuboid"):
                            mod.playerRegionMap[xuid] =
                                worldedit::Region::createRegion(
                                    worldedit::CUBOID, tmpbox,
                                    origin.getPlayer()->getDimensionId());
                            output.success(
                                "§aRegion has been switched to cuboid");
                            break;
                        case do_hash("expand"):
                            mod.playerRegionMap[xuid] =
                                worldedit::Region::createRegion(
                                    worldedit::EXPAND, tmpbox,
                                    origin.getPlayer()->getDimensionId());
                            output.success(
                                "§aRegion has been switched to expand");
                            break;
                        case do_hash("poly"):
                            mod.playerRegionMap[xuid] =
                                worldedit::Region::createRegion(
                                    worldedit::POLY, tmpbox,
                                    origin.getPlayer()->getDimensionId());
                            output.success(
                                "§aRegion has been switched to poly");
                            break;
                        case do_hash("sphere"):
                            mod.playerRegionMap[xuid] =
                                worldedit::Region::createRegion(
                                    worldedit::SPHERE, tmpbox,
                                    origin.getPlayer()->getDimensionId());
                            output.success(
                                "§aRegion has been switched to sphere");
                            break;
                        case do_hash("convex"):
                            mod.playerRegionMap[xuid] =
                                worldedit::Region::createRegion(
                                    worldedit::CONVEX, tmpbox,
                                    origin.getPlayer()->getDimensionId());
                            output.success(
                                "§aRegion has been switched to convex");
                            break;
                        default:
                            output.success("§cMissing parameter");
                            break;
                    }
                }
                if (mod.playerMainPosMap.count(xuid) > 0) {
                    mod.playerMainPosMap.erase(xuid);
                }
                if (mod.playerVicePosMap.count(xuid) > 0) {
                    mod.playerVicePosMap.erase(xuid);
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "set",        // command name
            "set block",  // command description
            {
                // enums{enumName, {values...}}
                // {"region",
                //  {"cuboid", "expand", "poly", "sphere", "convex", "clear"}},
            },
            {// parameters(type, name, [optional], [enumOptions(also
             // enumName)], [identifier]) identifier: used to identify
             // unique parameter data, if idnetifier is not set,
             //   it is set to be the same as enumOptions or name
             //   (identifier = enumOptions.empty() ? name:enumOptions)
             ParamData("block", ParamType::Block, "block"),
             ParamData("data", ParamType::Int, true, "data")},
            {
                // overloads{ (type == Enum ? enumOptions : name) ...}
                {"block", "data"},  // testenum <add|remove>
            },
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto blockname =
                    results["block"].get<Block const*>()->getTypeName();
                int data = 0;
                if (results["data"].isSet) {
                    data = results["data"].getRaw<int>();
                }
                auto block = Block::create(blockname, (unsigned short)data);
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.count(xuid) > 0 &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    long long i = 0;
                    auto dimID = mod.playerRegionMap[xuid]->getDimensionID();
                    mod.playerRegionMap[xuid]->forEachBlockInRegion(
                        [&](const BlockPos& pos) {
                            Level::setBlock(pos, dimID, block);
                            i++;
                        });
                    if (i > 1) {
                        output.success(fmt::format("§a{} blocks placed", i));
                    } else {
                        output.success(fmt::format("§a{} block placed", i));
                    }
                } else {
                    output.success("§cYou don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);

        DynamicCommand::setup(
            "gen",                     // command name
            "generate with function",  // command description
            {
                // enums{enumName, {values...}}
                // {"region",
                //  {"cuboid", "expand", "poly", "sphere", "convex", "clear"}},
            },
            {// parameters(type, name, [optional], [enumOptions(also
             // enumName)], [identifier]) identifier: used to identify
             // unique parameter data, if idnetifier is not set,
             //   it is set to be the same as enumOptions or name
             //   (identifier = enumOptions.empty() ? name:enumOptions)
             ParamData("function", ParamType::String, "function"),
             ParamData("block", ParamType::Block, "block"),
             ParamData("data", ParamType::Int, true, "data")},
            {
                // overloads{ (type == Enum ? enumOptions : name) ...}
                {"function", "block", "data"},  // testenum <add|remove>
            },
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto function = results["function"].get<std::string>();
                auto blockname =
                    results["block"].get<Block const*>()->getTypeName();
                int data = 0;
                if (results["data"].isSet) {
                    data = results["data"].getRaw<int>();
                }
                auto block = Block::create(blockname, (unsigned short)data);
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                if (mod.playerRegionMap.count(xuid) > 0 &&
                    mod.playerRegionMap[xuid]->hasSelected()) {
                    std::string s(function);
                    std::string origin(s);
                    s = toLowerString(s);
                    stringReplace(s, "--", "+");
                    stringReplace(s, "and", "&&");
                    stringReplace(s, "xor", "^");
                    stringReplace(s, "or", "||");
                    stringReplace(s, "--", "+");
                    stringReplace(s, "mod", "%");
                    stringReplace(s, "==", "=");
                    stringReplace(s, "π", "pi");
                    std::map<std::string, double> variables;
                    EvalFunctions f;
                    variables["pi"] = 3.14159265358979323846;
                    variables["e"] = 2.7182818284590452354;
                    auto str = s.c_str();
                    long long i = 0;
                    auto dimID = mod.playerRegionMap[xuid]->getDimensionID();
                    auto box = mod.playerRegionMap[xuid]->getBoundBox();
                    double lengthx = (box.bpos2.x - box.bpos1.x) * 0.5;
                    double lengthy = (box.bpos2.y - box.bpos1.y) * 0.5;
                    double lengthz = (box.bpos2.z - box.bpos1.z) * 0.5;
                    double centerx = (box.bpos2.x + box.bpos1.x) * 0.5;
                    double centery = (box.bpos2.y + box.bpos1.y) * 0.5;
                    double centerz = (box.bpos2.z + box.bpos1.z) * 0.5;
                    mod.playerRegionMap[xuid]->forEachBlockInRegion(
                        [&](const BlockPos& pos) {
                            variables["x"] = (pos.x - centerx) / lengthx;
                            variables["y"] = (pos.y - centery) / lengthy;
                            variables["z"] = (pos.z - centerz) / lengthz;
                            if (cpp_eval::eval<double>(str, variables, f) >
                                0.5) {
                                Level::setBlock(pos, dimID, block);
                                i++;
                            }
                        });
                    if (i > 1) {
                        output.success(
                            fmt::format("§g{}\n§a{} blocks placed", origin, i));
                    } else {
                        output.success(
                            fmt::format("§g{}\n§a{} block placed", origin, i));
                    }
                } else {
                    output.success("§cYou don't have a region yet");
                }
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit

#endif  // WORLDEDIT_ALLCOMMAND_H