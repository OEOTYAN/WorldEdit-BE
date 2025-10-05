#include "LocalContext.h"
#include "utils/Serialize.h"
#include "worldedit/WorldEdit.h"

namespace we {

LocalContext::LocalContext(mce::UUID const& uuid, bool temp)
: temp(temp),
  uuid(uuid),
  config(WorldEdit::getInstance().getConfig().player_default_config),
  history(*this) {
    setupBuilder(temp ? BuilderType::Inplace : BuilderType::None);
}
LocalContext::~LocalContext() {
    if (builder) builder->remove();
}

SimulatedPlayer* LocalContext::createSimulatedPlayer(std::string const& name) const {
    return SimulatedPlayer::create(name, lastPos, lastDim);
}

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

void LocalContext::setupBuilder(BuilderType type) {
    auto& gConfig = WorldEdit::getInstance().getConfig().player_state;
    if (!gConfig.allow_inplace_builder && type != BuilderType::Bot) {
        type = BuilderType::None;
    }
    if (!gConfig.allow_bot_builder && type == BuilderType::Bot) {
        type = BuilderType::None;
    }
    if (!builder || builder->getType() != type) {
        if (builder) builder->remove();
        builder = Builder::create(type, *this);
    }
}

void LocalContext::updateBuilderByPerm(bool isOp) {
    if (builder->getType() == BuilderType::None) {
        setupBuilder(isOp ? BuilderType::Inplace : BuilderType::Bot);
    } else if (!isOp) {
        setupBuilder(BuilderType::Bot);
    }
}

bool LocalContext::masked(BlockSource&, BlockPos const&) const { return false; }

bool LocalContext::setBlock(
    BlockSource&                source,
    BlockPos const&             pos,
    Block const&                block,
    std::shared_ptr<BlockActor> blockActor
) const {
    return builder->setBlock(source, pos, block, blockActor);
}
bool LocalContext::setExtraBlock(
    BlockSource&    source,
    BlockPos const& pos,
    Block const&    block
) const {
    return builder->setExtraBlock(source, pos, block);
}
bool LocalContext::setBiome(
    BlockSource&    source,
    BlockPos const& pos,
    Biome const&    biome
) const {
    return builder->setBiome(source, pos, biome);
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
        })
        .and_then([&, this]() {
            if (builder)
                return ll::reflection::serialize_to(
                    nbt["builderType"],
                    builder->getType()
                );
            return ll::Expected<>{};
        });
} catch (...) {
    return ll::makeExceptionError();
}
ll::Expected<> LocalContext::deserialize(CompoundTag const& nbt) noexcept try {
    return ll::reflection::deserialize(config, nbt["config"])
        .and_then([&, this]() {
            history.setMaxHistoryLength(config.history_length);
            return ll::Expected<>{};
        })
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
        })
        .and_then([&, this]() {
            ll::Expected<> result;
            BuilderType    builderType = BuilderType::None;
            if (nbt.contains("builderType"))
                result = ll::reflection::deserialize(builderType, nbt["builderType"]);
            setupBuilder(builderType);
            return result;
        });
} catch (...) {
    return ll::makeExceptionError();
}
} // namespace we
