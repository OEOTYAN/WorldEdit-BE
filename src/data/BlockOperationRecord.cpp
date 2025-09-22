#include "BlockOperationRecord.h"
#include "HistoryRecordFactory.h"
#include "utils/Serialize.h"
#include <mc/nbt/ListTag.h>

namespace {
// 注册BlockOperationRecord类型
static we::HistoryRecordRegistrar<we::BlockOperationRecord>
    blockRecordRegistrar(we::BlockOperationRecord::TYPE_NAME);
} // namespace

namespace we {

BlockOperationRecord::BlockOperationRecord(std::string description)
: mDescription(std::move(description)) {}

void BlockOperationRecord::addBlockChange(
    WithDim<BlockPos> position,
    CompoundTag       oldBlockNbt,
    CompoundTag       newBlockNbt
) {
    mBlockChanges.emplace_back(position, std::move(oldBlockNbt), std::move(newBlockNbt));
}

void BlockOperationRecord::addBlockChanges(std::vector<BlockChange> changes) {
    mBlockChanges.reserve(mBlockChanges.size() + changes.size());
    for (auto& change : changes) {
        mBlockChanges.emplace_back(std::move(change));
    }
}

const std::vector<BlockOperationRecord::BlockChange>&
BlockOperationRecord::getBlockChanges() const {
    return mBlockChanges;
}

const std::string& BlockOperationRecord::getDescription() const { return mDescription; }

void BlockOperationRecord::setDescription(const std::string& description) {
    mDescription = description;
}

size_t BlockOperationRecord::getBlockCount() const { return mBlockChanges.size(); }

void BlockOperationRecord::clear() {
    mBlockChanges.clear();
    mDescription.clear();
}

ll::Expected<> BlockOperationRecord::serialize(CompoundTag& tag) const noexcept try {
    // 序列化描述
    tag["description"] = mDescription;

    // 序列化块变更数量
    tag["blockCount"] = static_cast<int>(mBlockChanges.size());

    // 序列化位置列表
    std::vector<WithDim<BlockPos>> positions;
    positions.reserve(mBlockChanges.size());
    for (const auto& change : mBlockChanges) {
        positions.push_back(change.position);
    }
    auto posResult = ll::reflection::serialize_to(tag["positions"], positions);
    if (!posResult) {
        return posResult;
    }

    // 序列化旧块NBT的二进制数据
    std::vector<std::string> oldBlockData;
    oldBlockData.reserve(mBlockChanges.size());
    for (const auto& change : mBlockChanges) {
        oldBlockData.push_back(change.oldBlockNbt.toBinaryNbt());
    }
    auto oldResult = ll::reflection::serialize_to(tag["oldBlockData"], oldBlockData);
    if (!oldResult) {
        return oldResult;
    }

    // 序列化新块NBT的二进制数据
    std::vector<std::string> newBlockData;
    newBlockData.reserve(mBlockChanges.size());
    for (const auto& change : mBlockChanges) {
        newBlockData.push_back(change.newBlockNbt.toBinaryNbt());
    }
    return ll::reflection::serialize_to(tag["newBlockData"], newBlockData);
} catch (...) {
    return ll::makeExceptionError();
}

ll::Expected<> BlockOperationRecord::deserialize(CompoundTag const& tag) noexcept try {
    // 清空现有数据
    clear();

    // 反序列化描述
    if (tag.contains("description")) {
        auto result = ll::reflection::deserialize_to<std::string>(
            CompoundTagVariant{tag.at("description")}
        );
        if (result) {
            mDescription = *result;
        }
    }

    // 反序列化位置列表
    std::vector<WithDim<BlockPos>> positions;
    if (tag.contains("positions")) {
        auto posResult = ll::reflection::deserialize_to<std::vector<WithDim<BlockPos>>>(
            CompoundTagVariant{tag.at("positions")}
        );
        if (posResult) {
            positions = std::move(*posResult);
        }
    }

    // 反序列化旧块数据
    std::vector<std::string> oldBlockData;
    if (tag.contains("oldBlockData")) {
        auto oldResult = ll::reflection::deserialize_to<std::vector<std::string>>(
            CompoundTagVariant{tag.at("oldBlockData")}
        );
        if (oldResult) {
            oldBlockData = std::move(*oldResult);
        }
    }

    // 反序列化新块数据
    std::vector<std::string> newBlockData;
    if (tag.contains("newBlockData")) {
        auto newResult = ll::reflection::deserialize_to<std::vector<std::string>>(
            CompoundTagVariant{tag.at("newBlockData")}
        );
        if (newResult) {
            newBlockData = std::move(*newResult);
        }
    }

    // 重建BlockChange列表
    if (positions.size() == oldBlockData.size()
        && positions.size() == newBlockData.size()) {
        mBlockChanges.reserve(positions.size());
        for (size_t i = 0; i < positions.size(); ++i) {
            BlockChange change;
            change.position = positions[i];

            auto oldResult = CompoundTag::fromBinaryNbt(oldBlockData[i]);
            auto newResult = CompoundTag::fromBinaryNbt(newBlockData[i]);

            if (oldResult && newResult) {
                change.oldBlockNbt = *oldResult;
                change.newBlockNbt = *newResult;
                mBlockChanges.emplace_back(std::move(change));
            }
        }
    }

    return {};
} catch (...) {
    return ll::makeExceptionError();
}

std::string BlockOperationRecord::getType() const noexcept { return TYPE_NAME; }

size_t BlockOperationRecord::getEstimatedSize() const noexcept {
    size_t size  = sizeof(BlockOperationRecord);
    size        += mDescription.capacity();
    size        += mBlockChanges.capacity() * sizeof(BlockChange);

    // 估算NBT数据大小
    for (const auto& change : mBlockChanges) {
        size += change.oldBlockNbt.toBinaryNbt().size();
        size += change.newBlockNbt.toBinaryNbt().size();
    }

    return size;
}

std::unique_ptr<HistoryRecord> BlockOperationRecord::clone() const {
    auto cloned           = std::make_unique<BlockOperationRecord>(mDescription);
    cloned->mBlockChanges = mBlockChanges;
    return cloned;
}

} // namespace we