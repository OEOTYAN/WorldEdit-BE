#include "Globals.h"
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
    if (!ll::getPlugin("ParticleAPI")) {
        std::string path = "plugins/LiteLoader/ParticleAPI.dll";
        if (std::filesystem::exists(path)) {
            auto* handle = LoadLibrary(str2wstr(path).c_str());
            if (!handle) {
                throw std::runtime_error("Cannot get ParticleAPI.dll");
            }
            auto fn = GetProcAddress(handle, "onPostInit");
            ((void (*)())fn)();
            ParticleCUI::init(handle);
        } else {
            throw std::runtime_error("ParticleAPI.dll not exist!");
        }
    }

    preparePath("./plugins/WorldEdit");
    preparePath("./plugins/WorldEdit/lang");
    preparePath("./plugins/WorldEdit/image");
    preparePath("./plugins/WorldEdit/models");
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
