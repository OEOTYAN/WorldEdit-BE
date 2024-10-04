#pragma once

#include "Global.h"
#include "Macros.h"
#include "data/Config.h"
#include "data/PlayerContextManager.h"

#include <ll/api/mod/NativeMod.h>

namespace we {

class WorldEdit {

public:
    static WorldEdit& getInstance();

    WorldEdit(ll::mod::NativeMod& self);

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    [[nodiscard]] bsci::GeometryGroup& getGeo() const { return *mGeometryGroup; }

    [[nodiscard]] ll::io::Logger& getLogger() const { return getSelf().getLogger(); }

    [[nodiscard]] Config& getConfig() { return *mConfig; }

    [[nodiscard]] PlayerContextManager& getPlayerContextManager() {
        return *mPlayerContextManager;
    }

    [[nodiscard]] std::filesystem::path getConfigPath() const;

    bool loadConfig();

    bool saveConfig();

    bool load();

    bool enable();

    bool disable();

    bool unload();

private:
    ll::mod::NativeMod&                   mSelf;
    std::unique_ptr<bsci::GeometryGroup>  mGeometryGroup;
    std::optional<Config>                 mConfig;
    std::shared_ptr<PlayerContextManager> mPlayerContextManager;
};

inline ll::io::Logger& logger() { return WorldEdit::getInstance().getLogger(); }

} // namespace we
