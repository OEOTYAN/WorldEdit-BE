#include "WorldEdit.h"

#include <memory>

#include "utils/Serialize.h"

#include <ll/api/Config.h>
#include <ll/api/plugin/NativePlugin.h>
#include <ll/api/plugin/RegisterHelper.h>
#include <ll/api/utils/ErrorUtils.h>
#include <mc/network/packet/TextPacket.h>

namespace we {

static std::unique_ptr<WorldEdit> instance;

WorldEdit& WorldEdit::getInstance() { return *instance; }

WorldEdit::WorldEdit(ll::plugin::NativePlugin& self)
: mSelf(self),
  mThreadPool(std::clamp(std::thread::hardware_concurrency(), 1u, 16u)),
  mScheduler(mThreadPool),
  mServerScheduler(mTickSyncTaskPool) {}

std::filesystem::path WorldEdit::getConfigPath() const {
    return getSelf().getConfigDir() / u8"config.json";
}

bool WorldEdit::loadConfig() {
    bool res{};
    mConfig.emplace();
    try {
        res = ll::config::loadConfig(*mConfig, getConfigPath());
    } catch (...) {
        ll::error_utils::printCurrentException(getLogger());
        res = false;
    }
    if (!res) {
        res = ll::config::saveConfig(*mConfig, getConfigPath());
    }
    return res;
}

bool WorldEdit::saveConfig() { return ll::config::saveConfig(*mConfig, getConfigPath()); }

bool WorldEdit::load() {
    mGeometryGroup = bsci::GeometryGroup::createDefault();
    if (!loadConfig()) {
        return false;
    }
    getLogger().playerLevel = getConfig().log.player_log_level;
    getLogger().setPlayerOutputFunc([this](std::string_view msg) {
        getTickPool().addTask([pkt = TextPacket::createRawMessage(msg)] {
            const_cast<TextPacket&>(pkt).sendToClients();
        });
    });
    return true;
}

bool WorldEdit::enable() {
    if (!mConfig) {
        loadConfig();
    }
    mPlayerStateManager = std::make_shared<PlayerStateManager>();
    return true;
}

bool WorldEdit::disable() {
    saveConfig();
    mConfig.reset();
    mPlayerStateManager.reset();
    return true;
}

bool WorldEdit::unload() { return true; }

} // namespace we

LL_REGISTER_PLUGIN(we::WorldEdit, we::instance);
