#include "HistoryManager.h"
#include "HistoryRecordFactory.h"
#include "utils/Serialize.h"
#include <algorithm>

namespace we {

HistoryManager::HistoryManager(size_t maxHistoryLength, size_t maxSerializedLength)
: mMaxHistoryLength(maxHistoryLength),
  mMaxSerializedLength(maxSerializedLength),
  mCurrentIndex(0),
  mSize(0) {
    mHistoryRecords.reserve(mMaxHistoryLength);
}

void HistoryManager::addRecord(std::unique_ptr<HistoryRecord> record) {
    if (!record) {
        return;
    }

    std::lock_guard<std::mutex> lock(mMutex);

    // 如果当前不在历史记录的末尾，清除后续的记录
    if (mCurrentIndex < mSize) {
        size_t actualIndex = getActualIndex(mCurrentIndex);

        // 清除从当前位置到末尾的所有记录
        for (size_t i = mCurrentIndex; i < mSize; ++i) {
            size_t idx = getActualIndex(i);
            if (idx < mHistoryRecords.size()) {
                mHistoryRecords[idx].reset();
            }
        }
        mSize = mCurrentIndex;
    }

    // 如果已达到最大容量，移除最老的记录
    if (mSize >= mMaxHistoryLength) {
        cleanupOldRecords();
    }

    // 添加新记录
    if (mHistoryRecords.size() <= mSize) {
        mHistoryRecords.resize(mSize + 1);
    }

    size_t actualIndex = getActualIndex(mSize);
    if (actualIndex >= mHistoryRecords.size()) {
        mHistoryRecords.resize(actualIndex + 1);
    }

    mHistoryRecords[actualIndex] = std::move(record);
    ++mSize;
    mCurrentIndex = mSize;
}

std::shared_ptr<HistoryRecord> HistoryManager::undo() {
    std::lock_guard<std::mutex> lock(mMutex);

    if (!canUndo()) {
        return nullptr;
    }

    --mCurrentIndex;
    size_t actualIndex = getActualIndex(mCurrentIndex);

    if (actualIndex < mHistoryRecords.size() && mHistoryRecords[actualIndex]) {
        return std::shared_ptr<HistoryRecord>(mHistoryRecords[actualIndex]->clone());
    }

    return nullptr;
}

std::shared_ptr<HistoryRecord> HistoryManager::redo() {
    std::lock_guard<std::mutex> lock(mMutex);

    if (!canRedo()) {
        return nullptr;
    }

    size_t actualIndex = getActualIndex(mCurrentIndex);
    ++mCurrentIndex;

    if (actualIndex < mHistoryRecords.size() && mHistoryRecords[actualIndex]) {
        return std::shared_ptr<HistoryRecord>(mHistoryRecords[actualIndex]->clone());
    }

    return nullptr;
}

bool HistoryManager::canUndo() const { return mCurrentIndex > 0 && mSize > 0; }

bool HistoryManager::canRedo() const { return mCurrentIndex < mSize; }

size_t HistoryManager::size() const {
    std::lock_guard<std::mutex> lock(mMutex);
    return mSize;
}

size_t HistoryManager::getMaxHistoryLength() const { return mMaxHistoryLength; }

void HistoryManager::setMaxHistoryLength(size_t maxLength) {
    std::lock_guard<std::mutex> lock(mMutex);

    if (maxLength == 0) {
        maxLength = 1; // 至少保留一个历史记录
    }

    mMaxHistoryLength = maxLength;

    // 如果新的限制小于当前大小，需要清理
    if (mSize > mMaxHistoryLength) {
        cleanupOldRecords();
    }

    // 调整容器大小
    if (mHistoryRecords.size() > mMaxHistoryLength) {
        mHistoryRecords.resize(mMaxHistoryLength);
    }
}

size_t HistoryManager::getMaxSerializedLength() const { return mMaxSerializedLength; }

void HistoryManager::setMaxSerializedLength(size_t maxLength) {
    std::lock_guard<std::mutex> lock(mMutex);
    mMaxSerializedLength = maxLength;
}

void HistoryManager::clear() {
    std::lock_guard<std::mutex> lock(mMutex);

    mHistoryRecords.clear();
    mCurrentIndex = 0;
    mSize         = 0;
}

size_t HistoryManager::getEstimatedMemoryUsage() const {
    std::lock_guard<std::mutex> lock(mMutex);

    size_t totalSize = sizeof(HistoryManager);
    totalSize += mHistoryRecords.capacity() * sizeof(std::unique_ptr<HistoryRecord>);

    for (const auto& record : mHistoryRecords) {
        if (record) {
            totalSize += record->getEstimatedSize();
        }
    }

    return totalSize;
}

void HistoryManager::cleanupOldRecords() {
    if (mSize <= mMaxHistoryLength) {
        return;
    }

    // 计算需要移除的记录数量
    size_t recordsToRemove = mSize - mMaxHistoryLength;

    // 移除最老的记录
    for (size_t i = 0; i < recordsToRemove; ++i) {
        size_t actualIndex = getActualIndex(i);
        if (actualIndex < mHistoryRecords.size()) {
            mHistoryRecords[actualIndex].reset();
        }
    }

    // 更新大小和当前索引
    mSize = mMaxHistoryLength;
    if (mCurrentIndex > recordsToRemove) {
        mCurrentIndex -= recordsToRemove;
    } else {
        mCurrentIndex = 0;
    }
}

size_t HistoryManager::getActualIndex(size_t logicalIndex) const {
    return logicalIndex % mMaxHistoryLength;
}

ll::Expected<> HistoryManager::serialize(CompoundTag& tag) const noexcept try {
    std::lock_guard<std::mutex> lock(mMutex);

    // 序列化基本配置
    tag["maxHistoryLength"]    = static_cast<int>(mMaxHistoryLength);
    tag["maxSerializedLength"] = static_cast<int>(mMaxSerializedLength);
    tag["currentIndex"]        = static_cast<int>(mCurrentIndex);

    // 计算要序列化的记录数量（不超过 maxSerializedLength）
    size_t recordsToSerialize = std::min(mSize, mMaxSerializedLength);
    tag["recordsCount"]       = static_cast<int>(recordsToSerialize);

    if (recordsToSerialize == 0) {
        return {};
    }

    // 序列化记录类型和数据
    std::vector<std::string> recordTypes;
    std::vector<std::string> recordData;
    recordTypes.reserve(recordsToSerialize);
    recordData.reserve(recordsToSerialize);

    // 序列化最近的记录（从最新开始）
    for (size_t i = 0; i < recordsToSerialize; ++i) {
        size_t logicalIndex = (mSize > i) ? (mSize - 1 - i) : 0;
        size_t actualIndex  = getActualIndex(logicalIndex);

        if (actualIndex < mHistoryRecords.size() && mHistoryRecords[actualIndex]) {
            recordTypes.push_back(mHistoryRecords[actualIndex]->getType());

            CompoundTag dataTag;
            auto        result = mHistoryRecords[actualIndex]->serialize(dataTag);
            if (!result) {
                return result;
            }
            recordData.push_back(dataTag.toBinaryNbt());
        }
    }

    // 序列化记录类型和数据
    auto typeResult = ll::reflection::serialize_to(tag["recordTypes"], recordTypes);
    if (!typeResult) {
        return typeResult;
    }

    return ll::reflection::serialize_to(tag["recordData"], recordData);
} catch (...) {
    return ll::makeExceptionError();
}

ll::Expected<> HistoryManager::deserialize(CompoundTag const& tag) noexcept try {
    std::lock_guard<std::mutex> lock(mMutex);

    // 清空现有数据
    clear();

    // 反序列化基本配置
    if (tag.contains("maxHistoryLength")) {
        auto result = ll::reflection::deserialize_to<int>(
            CompoundTagVariant{tag.at("maxHistoryLength")}
        );
        if (result) {
            mMaxHistoryLength = static_cast<size_t>(*result);
        }
    }
    if (tag.contains("maxSerializedLength")) {
        auto result = ll::reflection::deserialize_to<int>(
            CompoundTagVariant{tag.at("maxSerializedLength")}
        );
        if (result) {
            mMaxSerializedLength = static_cast<size_t>(*result);
        }
    }
    if (tag.contains("currentIndex")) {
        auto result = ll::reflection::deserialize_to<int>(
            CompoundTagVariant{tag.at("currentIndex")}
        );
        if (result) {
            mCurrentIndex = static_cast<size_t>(*result);
        }
    }

    // 反序列化历史记录
    std::vector<std::string> recordTypes;
    std::vector<std::string> recordData;

    if (tag.contains("recordTypes")) {
        auto typeResult = ll::reflection::deserialize_to<std::vector<std::string>>(
            CompoundTagVariant{tag.at("recordTypes")}
        );
        if (typeResult) {
            recordTypes = std::move(*typeResult);
        }
    }

    if (tag.contains("recordData")) {
        auto dataResult = ll::reflection::deserialize_to<std::vector<std::string>>(
            CompoundTagVariant{tag.at("recordData")}
        );
        if (dataResult) {
            recordData = std::move(*dataResult);
        }
    }

    // 重建历史记录（注意：由于序列化时是从最新到最老，这里需要反向处理）
    if (recordTypes.size() == recordData.size()) {
        mHistoryRecords.reserve(recordTypes.size());

        for (auto i = recordTypes.size(); i > 0; --i) {
            size_t idx = i - 1; // 反向索引

            // 使用工厂创建对应类型的历史记录
            auto record = HistoryRecordFactory::create(recordTypes[idx]);
            if (record) {
                auto dataTagResult = CompoundTag::fromBinaryNbt(recordData[idx]);
                if (!dataTagResult) {
                    continue;
                }
                auto result = record->deserialize(*dataTagResult);
                if (result) {
                    if (mHistoryRecords.size() <= mSize) {
                        mHistoryRecords.resize(mSize + 1);
                    }
                    mHistoryRecords[mSize] = std::move(record);
                    ++mSize;
                }
            }
        }
    }

    return {};
} catch (...) {
    return ll::makeExceptionError();
}

} // namespace we