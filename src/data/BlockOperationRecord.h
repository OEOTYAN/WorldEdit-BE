#pragma once

#include "HistoryRecord.h"
#include "HistoryRecordFactory.h"
#include "worldedit/Global.h"

namespace we {

/**
 * @brief 块操作历史记录
 *
 * 记录单个或多个块的变更操作，包括位置、原始块状态和新块状态。
 * 这是一个具体的历史记录实现示例。
 */
class BlockOperationRecord : public HistoryRecord {
public:
    /**
     * @brief 单个块变更信息
     */
    struct BlockChange {
        WithDim<BlockPos> position;    ///< 块的位置（包含维度信息）
        CompoundTag       oldBlockNbt; ///< 原始块的NBT数据
        CompoundTag       newBlockNbt; ///< 新块的NBT数据

        BlockChange() = default;
        BlockChange(WithDim<BlockPos> pos, CompoundTag oldNbt, CompoundTag newNbt)
        : position(pos),
          oldBlockNbt(std::move(oldNbt)),
          newBlockNbt(std::move(newNbt)) {}
    };

private:
    std::vector<BlockChange> mBlockChanges; ///< 所有块变更的列表
    std::string              mDescription;  ///< 操作描述

public:
    /**
     * @brief 默认构造函数
     */
    BlockOperationRecord() = default;

    /**
     * @brief 构造函数
     *
     * @param description 操作描述
     */
    explicit BlockOperationRecord(std::string description);

    /**
     * @brief 添加单个块变更
     *
     * @param position 块位置
     * @param oldBlockNbt 原始块NBT
     * @param newBlockNbt 新块NBT
     */
    void addBlockChange(
        WithDim<BlockPos> position,
        CompoundTag       oldBlockNbt,
        CompoundTag       newBlockNbt
    );

    /**
     * @brief 添加多个块变更
     *
     * @param changes 块变更列表
     */
    void addBlockChanges(std::vector<BlockChange> changes);

    /**
     * @brief 获取所有块变更
     *
     * @return const std::vector<BlockChange>& 块变更列表的常引用
     */
    const std::vector<BlockChange>& getBlockChanges() const;

    /**
     * @brief 获取操作描述
     *
     * @return const std::string& 操作描述的常引用
     */
    const std::string& getDescription() const;

    /**
     * @brief 设置操作描述
     *
     * @param description 新的操作描述
     */
    void setDescription(const std::string& description);

    /**
     * @brief 获取变更的块数量
     *
     * @return size_t 块数量
     */
    size_t getBlockCount() const;

    /**
     * @brief 清空所有块变更
     */
    void clear();

    // 实现基类的虚函数
    ll::Expected<>                 serialize(CompoundTag& tag) const noexcept override;
    ll::Expected<>                 deserialize(CompoundTag const& tag) noexcept override;
    std::string                    getType() const noexcept override;
    size_t                         getEstimatedSize() const noexcept override;
    std::unique_ptr<HistoryRecord> clone() const override;

    /**
     * @brief 获取类型名称常量
     *
     * @return const char* 类型名称
     */
    static constexpr const char* TYPE_NAME = "BlockOperation";
};

} // namespace we