//
// Created by OEOTYAN on 2022/05/20.
//
#pragma once
#ifndef WORLDEDIT_HANDTOOLCOMMAND_H
#define WORLDEDIT_HANDTOOLCOMMAND_H

#include "pch.h"
#include <EventAPI.h>
#include <LoggerAPI.h>
#include <MC/Level.hpp>
#include <MC/BlockInstance.hpp>
#include <MC/Block.hpp>
#include <MC/BlockActor.hpp>
#include <MC/BlockSource.hpp>
#include <MC/CompoundTag.hpp>
#include <MC/Actor.hpp>
#include <MC/Player.hpp>
#include <MC/ServerPlayer.hpp>
#include <MC/Dimension.hpp>
#include <MC/ItemStack.hpp>
#include "Version.h"
#include "string/StringTool.h"
#include <LLAPI.h>
#include <ServerAPI.h>
#include <EventAPI.h>
#include <ScheduleAPI.h>
#include <DynamicCommandAPI.h>
#include "WorldEdit.h"

namespace worldedit {
    using ParamType = DynamicCommand::ParameterType;
    using ParamData = DynamicCommand::ParameterData;

    // none farwand

    void HandToolCommandSetup() {
        DynamicCommand::setup(
            "none",                  // command name
            "clear your hand tool",  // command description
            {}, {}, {{}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                if (mod.playerHandToolMap.find(xuid) !=
                    mod.playerHandToolMap.end()) {
                    std::string toolName =
                        origin.getPlayer()->getHandSlot()->getTypeName();
                    mod.playerHandToolMap[xuid].erase(toolName);
                }
                output.success(fmt::format("§ahand tool cleared"));
            },
            CommandPermissionLevel::GameMasters);
        DynamicCommand::setup(
            "farwand",            // command name
            "set your far wand",  // command description
            {}, {}, {{}},
            // dynamic command callback
            [](DynamicCommand const& command, CommandOrigin const& origin,
               CommandOutput& output,
               std::unordered_map<std::string, DynamicCommand::Result>&
                   results) {
                auto& mod = worldedit::getMod();
                auto xuid = origin.getPlayer()->getXuid();
                std::string toolName =
                    origin.getPlayer()->getHandSlot()->getTypeName();
                mod.playerHandToolMap[xuid][toolName] = "farwand";
                stringReplace(toolName, "minecraft:", "");
                output.success(
                    fmt::format("§aFar wand tool bound to {}", toolName));
            },
            CommandPermissionLevel::GameMasters);
    }
}  // namespace worldedit

#endif  // WORLDEDIT_HANDTOOLCOMMAND_H