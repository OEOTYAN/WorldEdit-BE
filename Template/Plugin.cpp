#include "Global.h"
#include <LoggerAPI.h>
#include "Version.h"
#include "subscribe/subscribeall.hpp"
#include "command/allCommand.hpp"
#include "WorldEdit.h"

Logger loggerp(PLUGIN_NAME);

void PluginInit() {
    auto& mod = worldedit::getMod();
    worldedit::serverSubscribe();
    worldedit::playerSubscribe();
    worldedit::commandsSetup();
}
