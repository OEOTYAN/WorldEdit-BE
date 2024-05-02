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

ll::Expected<> PlayerState::serialize(CompoundTag& nbt) const {
    return ll::reflection::serialize_to(nbt["config"], config)
        .and_then([&, this]() {
            if (mainPos)
                return ll::reflection::serialize_to(nbt["mainPos"], mainPos->data);
            return ll::Expected<>{};
        })
        .and_then([&, this]() {
            if (vicePos)
                return ll::reflection::serialize_to(nbt["vicePos"], vicePos->data);
            return ll::Expected<>{};
        })
        .and_then([&, this]() {
            if (regionType)
                return ll::reflection::serialize_to(nbt["regionType"], *regionType);
            return ll::Expected<>{};
        })
        .and_then([&, this]() {
            if (region) return region->serialize(nbt["region"].emplace<CompoundTag>());
            return ll::Expected<>{};
        });
}
ll::Expected<> PlayerState::deserialize(CompoundTag const& nbt) {
    return ll::reflection::deserialize(config, nbt["config"])
        .and_then([&, this]() {
            ll::Expected<> res;
            if (nbt.contains("mainPos")) {
                mainPos.emplace();
                if (res = ll::reflection::deserialize(mainPos->data, nbt["mainPos"]))
                    setMainPosInternal();
            }
            return res;
        })
        .and_then([&, this]() {
            ll::Expected<> res;
            if (nbt.contains("vicePos")) {
                vicePos.emplace();
                if (res = ll::reflection::deserialize(vicePos->data, nbt["vicePos"]))
                    setMainPosInternal();
            }
            return res;
        })
        .and_then([&, this]() {
            if (nbt.contains("regionType"))
                return ll::reflection::deserialize(*regionType, nbt["regionType"]);
            return ll::Expected<>{};
        })
        .and_then([&, this]() -> ll::Expected<> {
            if (nbt.contains("region")) {
                if (auto res = Region::create(nbt["region"].get<CompoundTag>()); res) {
                    region = std::move(*res);
                } else {
                    return ll::forwardError(res.error());
                }
            }
            return {};
        });
}
} // namespace we