#include "HistoryManager.h"
#include "data/LocalContext.h"
#include "utils/Serialize.h"

namespace we {

HistoryManager::HistoryManager(
    LocalContext& context,
    size_t        maxHistoryLength,
    size_t        maxSerializedLength
)
: context(context),
  mMaxHistoryLength(maxHistoryLength),
  mMaxSerializedLength(maxSerializedLength),
  mHeadIndex(0),
  mCurrentIndex(0),
  mSize(0) {
    mHistoryRecords.resize(mMaxHistoryLength);
}

bool HistoryManager::addRecord(std::shared_ptr<HistoryRecord> record) {
    if (!record || record->size() == 0) {
        return false;
    }

    std::lock_guard<std::mutex> lock(mMutex);

    // 如果当前不在历史记录的末尾，清除后续的记录
    if (mCurrentIndex < mSize) {
        // 清除从当前位置到末尾的所有记录
        for (size_t i = mCurrentIndex; i < mSize; ++i) {
            size_t actualIndex = (mHeadIndex + i) % mMaxHistoryLength;
            mHistoryRecords[actualIndex].reset();
        }
        mSize = mCurrentIndex;
    }

    // 计算新记录的位置
    size_t insertIndex;
    if (mSize < mMaxHistoryLength) {
        // 还有空间，直接在末尾添加
        insertIndex = (mHeadIndex + mSize) % mMaxHistoryLength;
        ++mSize;
    } else {
        // 数组已满，覆盖最老的记录，移动头部
        insertIndex = mHeadIndex;
        mHeadIndex  = (mHeadIndex + 1) % mMaxHistoryLength;
        // mSize 保持不变
    }

    // 添加新记录
    mHistoryRecords[insertIndex] = std::move(record);
    mCurrentIndex                = mSize;
    return true;
}

std::shared_ptr<HistoryRecord> HistoryManager::undo() {
    std::lock_guard<std::mutex> lock(mMutex);

    if (!canUndo()) {
        return nullptr;
    }

    --mCurrentIndex;
    size_t actualIndex = (mHeadIndex + mCurrentIndex) % mMaxHistoryLength;

    if (mHistoryRecords[actualIndex]) {
        return mHistoryRecords[actualIndex];
    }

    return nullptr;
}

std::shared_ptr<HistoryRecord> HistoryManager::redo() {
    std::lock_guard<std::mutex> lock(mMutex);

    if (!canRedo()) {
        return nullptr;
    }

    size_t actualIndex = (mHeadIndex + mCurrentIndex) % mMaxHistoryLength;
    ++mCurrentIndex;

    if (mHistoryRecords[actualIndex]) {
        return mHistoryRecords[actualIndex];
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

    if (maxLength == mMaxHistoryLength) {
        return; // 没有变化
    }

    context.config.history_length = maxLength;

    if (maxLength < mMaxHistoryLength) {
        // 缩小容量，需要重新组织数据
        std::vector<std::shared_ptr<HistoryRecord>> newRecords(maxLength);
        size_t                                      newSize = std::min(mSize, maxLength);

        // 保留最新的记录
        for (size_t i = 0; i < newSize; ++i) {
            size_t oldIndex = (mHeadIndex + (mSize - newSize) + i) % mMaxHistoryLength;
            newRecords[i]   = std::move(mHistoryRecords[oldIndex]);
        }

        mHistoryRecords   = std::move(newRecords);
        mMaxHistoryLength = maxLength;
        mHeadIndex        = 0;
        mSize             = newSize;
        mCurrentIndex     = std::min(mCurrentIndex, mSize);
    } else {
        // 扩大容量
        std::vector<std::shared_ptr<HistoryRecord>> newRecords(maxLength);

        // 复制现有记录到新数组的开头
        for (size_t i = 0; i < mSize; ++i) {
            size_t oldIndex = (mHeadIndex + i) % mMaxHistoryLength;
            newRecords[i]   = std::move(mHistoryRecords[oldIndex]);
        }

        mHistoryRecords   = std::move(newRecords);
        mMaxHistoryLength = maxLength;
        mHeadIndex        = 0;
        // mSize 和 mCurrentIndex 保持不变
    }
}

size_t HistoryManager::getMaxSerializedLength() const { return mMaxSerializedLength; }

void HistoryManager::setMaxSerializedLength(size_t maxLength) {
    std::lock_guard<std::mutex> lock(mMutex);
    mMaxSerializedLength = maxLength;
}

void HistoryManager::clear() {
    std::lock_guard<std::mutex> lock(mMutex);

    // 清空所有记录
    for (auto& record : mHistoryRecords) {
        record.reset();
    }

    mCurrentIndex = 0;
    mSize         = 0;
    mHeadIndex    = 0;
}

size_t HistoryManager::getEstimatedMemoryUsage() const {
    std::lock_guard<std::mutex> lock(mMutex);

    size_t totalSize = sizeof(HistoryManager);
    totalSize += mHistoryRecords.capacity() * sizeof(std::shared_ptr<HistoryRecord>);

    for (const auto& record : mHistoryRecords) {
        if (record) {
            totalSize += record->getEstimatedSize();
        }
    }

    return totalSize;
}

} // namespace we