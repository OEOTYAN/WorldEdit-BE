#include "LocalContext.h"
#include "utils/Serialize.h"
#include "worldedit/WorldEdit.h"

namespace we {

LocalContext::LocalContext(mce::UUID const& uuid, bool temp)
: temp(temp),
  uuid(uuid),
  config(WorldEdit::getInstance().getConfig().player_default_config) {}

void LocalContext::setMainPosInternal() {
    if (mainPos) {
        auto& we     = WorldEdit::getInstance();
        mainPos->geo = we.getGeo().box(
            mainPos->data.dim,
            AABB{mainPos->data.pos}.shrink(-0.07f),
            we.getConfig().colors.main_hand_color
        );
    }
}
void LocalContext::setOffPosInternal() {
    if (offPos) {
        auto& we    = WorldEdit::getInstance();
        offPos->geo = we.getGeo().box(
            offPos->data.dim,
            AABB{offPos->data.pos}.shrink(-0.06f),
            we.getConfig().colors.off_hand_color
        );
    }
}
Region& LocalContext::getOrCreateRegion(WithDim<BlockPos> const& v) {
    if (!region || region->getDim() != v.dim) {
        region =
            Region::create(regionType.value_or(config.default_region_type), v.dim, v.pos);
        regionType = region->getType();
    }
    return *region;
}
bool LocalContext::setMainPos(WithDim<BlockPos> const& v) {
    if (auto& r = getOrCreateRegion(v); r.setMainPos(v.pos)) {
        mainPos.emplace(v);
        setMainPosInternal();
        if (r.needResetVice()) {
            offPos.reset();
        }
        return true;
    }
    return false;
}
bool LocalContext::setOffPos(WithDim<BlockPos> const& v) {
    if (getOrCreateRegion(v).setOffPos(v.pos)) {
        offPos.emplace(v);
        setOffPosInternal();
        return true;
    }
    return false;
}

ll::Expected<> LocalContext::serialize(CompoundTag& nbt) const noexcept try {
    return ll::reflection::serialize_to(nbt["config"], config)
        .and_then([&, this]() {
            if (mainPos)
                return ll::reflection::serialize_to(nbt["mainPos"], mainPos->data);
            return ll::Expected<>{};
        })
        .and_then([&, this]() {
            if (offPos) return ll::reflection::serialize_to(nbt["offPos"], offPos->data);
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
} catch (...) {
    return ll::makeExceptionError();
}
ll::Expected<> LocalContext::deserialize(CompoundTag const& nbt) noexcept try {
    return ll::reflection::deserialize(config, nbt["config"])
        .and_then([&, this]() {
            ll::Expected<> res;
            if (nbt.contains("mainPos")) {
                if (res = ll::reflection::deserialize(
                        mainPos.emplace().data,
                        nbt["mainPos"]
                    ))
                    setMainPosInternal();
            }
            return res;
        })
        .and_then([&, this]() {
            ll::Expected<> res;
            if (nbt.contains("offPos")) {
                if (res =
                        ll::reflection::deserialize(offPos.emplace().data, nbt["offPos"]))
                    setMainPosInternal();
            }
            return res;
        })
        .and_then([&, this]() {
            if (nbt.contains("regionType"))
                return ll::reflection::deserialize(
                    regionType.emplace(),
                    nbt["regionType"]
                );
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
} catch (...) {
    return ll::makeExceptionError();
}
} // namespace we
