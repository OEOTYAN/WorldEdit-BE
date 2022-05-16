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
#include "command/allCommand.hpp"
#include "WorldEdit.h"

Logger logger(PLUGIN_NAME);

inline void CheckProtocolVersion() {
#ifdef TARGET_BDS_PROTOCOL_VERSION
    auto currentProtocol = LL::getServerProtocolVersion();
    if (TARGET_BDS_PROTOCOL_VERSION != currentProtocol) {
        logger.warn(
            "Protocol version not match, target version: {}, current version: "
            "{}.",
            TARGET_BDS_PROTOCOL_VERSION, currentProtocol);
        logger.warn(
            "This will most likely crash the server, please use the Plugin "
            "that matches the BDS version!");
    }
#endif  // TARGET_BDS_PROTOCOL_VERSION
}

void PluginInit() {
    CheckProtocolVersion();

    worldedit::serverSubscribe();
    worldedit::playerSubscribe();
    worldedit::commandsSetup();
}
