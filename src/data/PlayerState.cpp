#include "PlayerState.h"
#include "utils/Serialize.h"
#include "worldedit/WorldEdit.h"

namespace we {

PlayerState::PlayerState(mce::UUID const& uuid, bool temp)
: temp(temp),
  uuid(uuid),
  config(WorldEdit::getInstance().getConfig().player_default_config) {}

void PlayerState::setMainPosInternal() {
    if (mainPos) {
        auto& we     = WorldEdit::getInstance();
        mainPos->geo = we.getGeo().box(
            mainPos->data.dim,
            AABB{mainPos->data.pos}.shrink(-0.07),
            we.getConfig().colors.main_hand_color
        );
    }
}
void PlayerState::setVicePosInternal() {
    if (vicePos) {
        auto& we     = WorldEdit::getInstance();
        vicePos->geo = we.getGeo().box(
            vicePos->data.dim,
            AABB{vicePos->data.pos}.shrink(-0.06),
            we.getConfig().colors.off_hand_color
        );
    }
}
Region& PlayerState::getOrCreateRegion(WithDim<BlockPos> const& v) {
    if (!region || region->getDim() != v.dim) {
        region =
            Region::create(regionType.value_or(config.default_region_type), v.dim, v.pos);
        regionType = region->getType();
        mDirty     = true;
    }
    return *region;
}
bool PlayerState::setMainPos(WithDim<BlockPos> const& v) {
    if (auto& r = getOrCreateRegion(v); r.setMainPos(v.pos)) {
        mainPos.emplace(v);
        setMainPosInternal();
        if (r.needResetVice()) {
            vicePos.reset();
        }
        mDirty = true;
        return true;
    }
    return false;
}
bool PlayerState::setVicePos(WithDim<BlockPos> const& v) {
    if (getOrCreateRegion(v).setVicePos(v.pos)) {
        vicePos.emplace(v);
        setVicePosInternal();
        mDirty = true;
        return true;
    }
    return false;
}

void PlayerState::serialize(CompoundTag& nbt) const {
    doSerialize(config, nbt["config"]);
    if (mainPos) doSerialize(mainPos->data, nbt["mainPos"]);
    if (vicePos) doSerialize(vicePos->data, nbt["vicePos"]);
    if (regionType) doSerialize(*regionType, nbt["regionType"]);
    if (region) region->serialize(nbt["region"].emplace<CompoundTag>());
}
void PlayerState::deserialize(CompoundTag const& nbt) {
    ll::reflection::deserialize(config, nbt["config"]);
    if (nbt.contains("mainPos")) {
        mainPos.emplace();
        ll::reflection::deserialize(mainPos->data, nbt["mainPos"]);
        setMainPosInternal();
    }
    if (nbt.contains("vicePos")) {
        vicePos.emplace();
        ll::reflection::deserialize(vicePos->data, nbt["vicePos"]);
        setVicePosInternal();
    }
    if (nbt.contains("regionType")) {
        ll::reflection::deserialize(*regionType, nbt["regionType"]);
    }
    if (nbt.contains("region")) {
        region = Region::create(nbt["region"].get<CompoundTag>());
    }
}
} // namespace we