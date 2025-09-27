#pragma once

#include "HistoryRecord.h"
#include "worldedit/Global.h"
#include <memory>
#include <mutex>
#include <vector>

namespace we {

/**
 * @brief 历史记录管理器
 *
 * 负责管理历史记录的存储、撤销(undo)、重做(redo)操作，
 * 支持配置历史记录长度和序列化的历史记录长度。
 */
class HistoryManager {
private:
    LocalContext&                               context;
    std::vector<std::shared_ptr<HistoryRecord>> mHistoryRecords;

    size_t mMaxHistoryLength;    ///< 最大历史记录长度
    size_t mMaxSerializedLength; ///< 最大序列化历史记录长度
    size_t mHeadIndex;           ///< 循环数组的头部索引（最老记录的位置）
    size_t mCurrentIndex;        ///< 当前历史记录索引（相对于头部的逻辑位置）
    size_t mSize;                ///< 当前历史记录数量

    mutable std::mutex mMutex; ///< 线程安全保护

public:
    /**
     * @brief 构造函数
     *
     * @param maxHistoryLength 最大历史记录长度（默认20）
     * @param maxSerializedLength 最大序列化历史记录长度（默认10）
     */
    explicit HistoryManager(
        LocalContext& context,
        size_t maxHistoryLength    = 128,
        size_t maxSerializedLength = 10
    );

    /**
     * @brief 析构函数
     */
    ~HistoryManager() = default;

    // 禁用拷贝构造和拷贝赋值
    HistoryManager(const HistoryManager&)            = delete;
    HistoryManager& operator=(const HistoryManager&) = delete;

    // 支持移动构造和移动赋值
    HistoryManager(HistoryManager&&) noexcept            = default;
    HistoryManager& operator=(HistoryManager&&) noexcept = default;

    /**
     * @brief 添加新的历史记录
     *
     * @param record 要添加的历史记录（转移所有权）
     */
    bool addRecord(std::shared_ptr<HistoryRecord> record);

    /**
     * @brief 撤销操作
     *
     * @return  撤销的历史记录，如果无法撤销则返回 nullptr
     */
   std::shared_ptr<HistoryRecord> undo();

    /**
     * @brief 重做操作
     *
     * @return  重做的历史记录，如果无法重做则返回 nullptr
     */
   std::shared_ptr<HistoryRecord> redo();

    /**
     * @brief 检查是否可以撤销
     *
     * @return bool 如果可以撤销返回 true
     */
    bool canUndo() const;

    /**
     * @brief 检查是否可以重做
     *
     * @return bool 如果可以重做返回 true
     */
    bool canRedo() const;

    /**
     * @brief 获取当前历史记录数量
     *
     * @return size_t 历史记录数量
     */
    size_t size() const;

    /**
     * @brief 获取最大历史记录长度
     *
     * @return size_t 最大历史记录长度
     */
    size_t getMaxHistoryLength() const;

    /**
     * @brief 设置最大历史记录长度
     *
     * @param maxLength 新的最大历史记录长度
     */
    void setMaxHistoryLength(size_t maxLength);

    /**
     * @brief 获取最大序列化历史记录长度
     *
     * @return size_t 最大序列化历史记录长度
     */
    size_t getMaxSerializedLength() const;

    /**
     * @brief 设置最大序列化历史记录长度
     *
     * @param maxLength 新的最大序列化历史记录长度
     */
    void setMaxSerializedLength(size_t maxLength);

    /**
     * @brief 清空所有历史记录
     */
    void clear();

    /**
     * @brief 获取总的估计内存使用量（字节）
     *
     * @return size_t 估计的内存使用量
     */
    size_t getEstimatedMemoryUsage() const;
};

} // namespace we