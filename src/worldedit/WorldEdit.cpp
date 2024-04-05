#include "WorldEdit.h"

#include <memory>

#include <ll/api/Config.h>
#include <ll/api/plugin/NativePlugin.h>
#include <ll/api/plugin/RegisterHelper.h>
#include <ll/api/utils/ErrorUtils.h>


namespace we {

static std::unique_ptr<WorldEdit> instance;

WorldEdit& WorldEdit::getInstance() { return *instance; }

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
    if (!loadConfig()) {
        return false;
    }
    return true;
}

bool WorldEdit::enable() {
    if (!mConfig) {
        loadConfig();
    }
    static auto geo = bsci::GeometryGroup::createDefault();

    geo->line(
        0,
        BlockPos{0, 70, 0}.center(),
        BlockPos{10, 73, 6}.center(),
        mce::Color::BLUE
    );

    geo->box(
        0,
        BoundingBox{
            {0,  70, 0},
            {10, 73, 6}
    },
        mce::Color::PINK
    );

    geo->circle(0, BlockPos{0, 73, 0}.center(), Vec3{1, 1, 1}.normalize(), 8);

    geo->sphere(
        0,
        BlockPos{0, 180, 0}.center(),
        100,
        mce::Color::CYAN
    );
    return true;
}

bool WorldEdit::disable() {
    saveConfig();
    mConfig.reset();
    return true;
}

bool WorldEdit::unload() { return true; }

} // namespace we

LL_REGISTER_PLUGIN(we::WorldEdit, we::instance);
