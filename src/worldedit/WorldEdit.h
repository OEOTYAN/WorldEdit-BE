#pragma once

#include "Global.h"
#include "data/Config.h"
#include "data/PlayerStateManager.h"
#include "utils/ThreadPoolWrapper.h"

#include <ll/api/plugin/NativePlugin.h>

#define DBGLOG(...) ::we::WorldEdit::getInstance().getLogger().warn(__VA_ARGS__);

namespace we {

class WorldEdit {

public:
    static WorldEdit& getInstance();

    WorldEdit(ll::plugin::NativePlugin& self);

    [[nodiscard]] ll::plugin::NativePlugin& getSelf() const { return mSelf; }

    [[nodiscard]] bsci::GeometryGroup& getGeo() const { return *mGeometryGroup; }

    [[nodiscard]] ll::Logger& getLogger() const { return getSelf().getLogger(); }

    [[nodiscard]] Config& getConfig() { return *mConfig; }

    [[nodiscard]] PlayerStateManager& getPlayerStateManager() {
        return *mPlayerStateManager;
    }

    [[nodiscard]] ThreadPoolWrapper getPool() { return mThreadPool; }

    [[nodiscard]] TickPoolWrapper getTickPool() { return mTickSyncTaskPool; }

    [[nodiscard]] auto& getScheduler() { return mScheduler; }

    [[nodiscard]] auto& geServerScheduler() { return mScheduler; }

    [[nodiscard]] std::filesystem::path getConfigPath() const;

    bool loadConfig();

    bool saveConfig();

    bool load();

    bool enable();

    bool disable();

    bool unload();

private:
    ll::plugin::NativePlugin&            mSelf;
    std::unique_ptr<bsci::GeometryGroup> mGeometryGroup;
    std::optional<Config>                mConfig;
    std::shared_ptr<PlayerStateManager>  mPlayerStateManager;
    ll::thread::ThreadPool               mThreadPool;
    ll::schedule::Scheduler<std::chrono::system_clock, ThreadPoolWrapper> mScheduler;
    ll::thread::TickSyncTaskPool                                      mTickSyncTaskPool;
    ll::schedule::Scheduler<ll::chrono::ServerClock, TickPoolWrapper> mServerScheduler;
};

} // namespace we
