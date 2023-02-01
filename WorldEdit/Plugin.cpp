#include "Global.h"
#include <LoggerAPI.h>
#include <I18nAPI.h>
#include "Version.h"
#include "subscribe/subscribeall.hpp"
#include "WorldEdit.h"
#include <filesystem>

void preparePath(std::string_view path) {
    namespace fs = std::filesystem;
    if (!fs::exists(path)) {
        fs::create_directory(path);
    }
}

void PluginInit() {
    preparePath("./plugins/WorldEdit");
    preparePath("./plugins/WorldEdit/lang");
    preparePath("./plugins/WorldEdit/image");
    preparePath("./plugins/WorldEdit/imgtemp");
    preparePath("./plugins/WorldEdit/mappings");
    preparePath("./plugins/WorldEdit/textures");
    preparePath("./plugins/WorldEdit/structures");

    Translation::load("./plugins/WorldEdit/lang/");

    worldedit::getPlayersDataMap();

    worldedit::serverSubscribe();
    worldedit::playerSubscribe();

    worldedit::logger().info("");
    worldedit::logger().info(tr("worldedit.version"), PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR,
                             PLUGIN_VERSION_REVISION,
                             PLUGIN_VERSION_STATUS == 0 ? "dev" : (PLUGIN_VERSION_STATUS == 1 ? "beta" : "release"));
    worldedit::logger().info(tr("worldedit.wiki"));
    worldedit::logger().info("");
}
