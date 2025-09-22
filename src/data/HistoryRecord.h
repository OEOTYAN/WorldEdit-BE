#pragma once

#include "worldedit/Global.h"

namespace we {

/**
 * @brief 抽象的历史记录数据基类
 *
 * 此类定义了历史记录数据的基本接口，包括序列化和反序列化方法。
 * 所有具体的历史记录类型都应继承此类并实现相应的虚函数。
 */
class HistoryRecord {
public:
    virtual ~HistoryRecord() = default;

    /**
     * @brief 序列化历史记录数据到 CompoundTag
     *
     * @param tag 用于存储序列化数据的 CompoundTag 引用
     * @return ll::Expected<> 成功时返回空的 Expected，失败时返回错误信息
     */
    virtual ll::Expected<> serialize(CompoundTag& tag) const noexcept = 0;

    /**
     * @brief 从 CompoundTag 反序列化历史记录数据
     *
     * @param tag 包含序列化数据的 CompoundTag 常引用
     * @return ll::Expected<> 成功时返回空的 Expected，失败时返回错误信息
     */
    virtual ll::Expected<> deserialize(CompoundTag const& tag) noexcept = 0;

    /**
     * @brief 获取历史记录的类型标识
     *
     * @return std::string 历史记录的类型名称，用于反序列化时的类型识别
     */
    virtual std::string getType() const noexcept = 0;

    /**
     * @brief 获取历史记录的估计大小（字节）
     *
     * @return size_t 历史记录占用的内存大小估计值
     */
    virtual size_t getEstimatedSize() const noexcept = 0;

    /**
     * @brief 克隆当前历史记录
     *
     * @return std::unique_ptr<HistoryRecord> 当前历史记录的副本
     */
    virtual std::unique_ptr<HistoryRecord> clone() const = 0;
};

} // namespace we