#include "WorldEdit.h"

#include <memory>

#include "command/Command.h"
#include "utils/Serialize.h"

#include <ll/api/Config.h>
#include <ll/api/mod/NativeMod.h>
#include <ll/api/mod/RegisterHelper.h>
#include <ll/api/utils/ErrorUtils.h>
#include <mc/network/packet/TextPacket.h>

namespace we {

WorldEdit& WorldEdit::getInstance() {
    static WorldEdit instance;
    return instance;
}

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
    if (auto res = ::ll::i18n::getInstance().load(getSelf().getLangDir()); !res) {
        getLogger().error("i18n load failed");
        res.error().log(getLogger());
    }
    return true;
}

bool WorldEdit::enable() {
    if (!mConfig) {
        loadConfig();
    }
    mLocalContextManager = std::make_shared<LocalContextManager>(*this);
    setupCommands();
    return true;
}

bool WorldEdit::disable() {
    saveConfig();
    mConfig.reset();
    mLocalContextManager.reset();
    return true;
}

bool WorldEdit::unload() { return true; }

} // namespace we

LL_REGISTER_MOD(we::WorldEdit, we::WorldEdit::getInstance());
