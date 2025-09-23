#include "HistoryManager.h"
#include "utils/Serialize.h"
#include <algorithm>

namespace we {

HistoryManager::HistoryManager(size_t maxHistoryLength, size_t maxSerializedLength)
: mMaxHistoryLength(maxHistoryLength),
  mMaxSerializedLength(maxSerializedLength),
  mCurrentIndex(0),
  mSize(0) {
    mHistoryRecords.resize(mMaxHistoryLength);
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

optional_ref<HistoryRecord const> HistoryManager::undo() {
    std::lock_guard<std::mutex> lock(mMutex);

    if (!canUndo()) {
        return nullptr;
    }

    --mCurrentIndex;
    size_t actualIndex = getActualIndex(mCurrentIndex);

    if (actualIndex < mHistoryRecords.size() && mHistoryRecords[actualIndex]) {
        return *mHistoryRecords[actualIndex];
    }

    return nullptr;
}

optional_ref<HistoryRecord const> HistoryManager::redo() {
    std::lock_guard<std::mutex> lock(mMutex);

    if (!canRedo()) {
        return nullptr;
    }

    size_t actualIndex = getActualIndex(mCurrentIndex);
    ++mCurrentIndex;

    if (actualIndex < mHistoryRecords.size() && mHistoryRecords[actualIndex]) {
        return *mHistoryRecords[actualIndex];
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
} // namespace we