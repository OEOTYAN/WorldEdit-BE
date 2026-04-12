#include "WorldEdit.h"

#include <memory>

#include "command/Command.h"
#include "utils/Serialize.h"

#include <ll/api/Config.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/command/ServerCommandRegisterEvent.h>
#include <ll/api/event/server/ServerStartedEvent.h>
#include <ll/api/event/server/ServerStoppingEvent.h>
#include <ll/api/mod/NativeMod.h>
#include <ll/api/mod/RegisterHelper.h>
#include <ll/api/service/GamingStatus.h>
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

bool WorldEdit::saveConfig() {
    if (!mConfig) {
        return true;
    }
    return ll::config::saveConfig(*mConfig, getConfigPath());
}

bool WorldEdit::load() {
    mGeometryGroup = bsci::GeometryGroup::createDefault();
    if (!loadConfig()) {
        return false;
    }
    if (auto res = ::ll::i18n::getInstance().load(getSelf().getLangDir()); !res) {
        getLogger().error("i18n load failed");
        res.error().log(getLogger());
    }
    mListeners.push_back(
        ll::event::EventBus::getInstance()
            .emplaceListener<ll::event::ServerCommandRegisterEvent>([this](auto&&) {
                setupCommands();
            })
    );
    mListeners.push_back(
        ll::event::EventBus::getInstance().emplaceListener<ll::event::ServerStartedEvent>(
            [this](auto&&) {
                mLocalContextManager = std::make_shared<LocalContextManager>(*this);
            }
        )
    );
    mListeners.push_back(
        ll::event::EventBus::getInstance()
            .emplaceListener<ll::event::ServerStoppingEvent>([this](auto&&) {
                mLocalContextManager.reset();
            })
    );

    return true;
}

bool WorldEdit::enable() {
    if (ll::getGamingStatus() == ll::GamingStatus::Running) {
        mLocalContextManager = std::make_shared<LocalContextManager>(*this);
    }
    return true;
}

bool WorldEdit::disable() {
    mLocalContextManager.reset();
    return true;
}

bool WorldEdit::unload() {
    saveConfig();
    return true;
}

} // namespace we

LL_REGISTER_MOD(we::WorldEdit, we::WorldEdit::getInstance());
