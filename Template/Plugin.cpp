#include "pch.h"
#include <LoggerAPI.h>
#include "Version.h"
#include "subscribe/subscribeall.hpp"
#include "command/allCommand.hpp"
#include "WorldEdit.h"

Logger loggerp(PLUGIN_NAME);

inline void CheckProtocolVersion() {
#ifdef TARGET_BDS_PROTOCOL_VERSION
    auto currentProtocol = LL::getServerProtocolVersion();
    if (TARGET_BDS_PROTOCOL_VERSION != currentProtocol) {
        loggerp.warn(
            "Protocol version not match, target version: {}, current version: "
            "{}.",
            TARGET_BDS_PROTOCOL_VERSION, currentProtocol);
        loggerp.warn(
            "This will most likely crash the server, please use the Plugin "
            "that matches the BDS version!");
    }
#endif  // TARGET_BDS_PROTOCOL_VERSION
}

void PluginInit() {
    CheckProtocolVersion();
    auto &mod= worldedit::getMod();
    worldedit::serverSubscribe();
    worldedit::playerSubscribe();
    worldedit::commandsSetup();
}
