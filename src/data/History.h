#pragma once

/**
 * @file History.h
 * @brief 历史记录管理系统总头文件
 *
 * 这个头文件包含了整个历史记录管理系统的所有核心组件：
 *
 * 1. HistoryRecord - 抽象的历史记录基类
 * 2. HistoryManager - 历史记录管理器
 * 3. HistoryRecordFactory - 历史记录工厂类
 * 4. BlockOperationRecord - 块操作历史记录实现
 * 5. HistoryExample - 使用示例
 *
 * 使用方法：
 * ```cpp
 * #include "data/History.h"
 *
 * // 创建历史管理器
 * we::HistoryManager manager(20, 10);
 *
 * // 创建历史记录
 * auto record = std::make_unique<we::BlockOperationRecord>("我的操作");
 * record->addBlockChange(pos, oldNbt, newNbt);
 *
 * // 添加到管理器
 * manager.addRecord(std::move(record));
 *
 * // 撤销操作
 * if (manager.canUndo()) {
 *     auto undoRecord = manager.undo();
 *     // 处理撤销...
 * }
 *
 * // 序列化
 * CompoundTag nbt;
 * manager.serialize(nbt);
 * ```
 */

// 核心组件
#include "HistoryManager.h"
#include "HistoryRecord.h"
#include "HistoryRecordFactory.h"

// 具体实现
#include "BlockOperationRecord.h"

// 示例和文档
#include "HistoryExample.h"

namespace we {

/**
 * @brief 历史记录管理系统命名空间
 *
 * 包含了完整的历史记录管理功能，支持：
 * - 撤销/重做操作
 * - 序列化和反序列化
 * - 可配置的历史记录长度
 * - 线程安全操作
 * - 可扩展的记录类型系统
 */
namespace history {

/**
 * @brief 创建一个标准配置的历史管理器
 *
 * @param maxHistoryLength 最大历史记录长度（默认20）
 * @param maxSerializedLength 最大序列化长度（默认10）
 * @return std::unique_ptr<HistoryManager> 创建的历史管理器
 */
inline std::unique_ptr<HistoryManager>
createStandardManager(size_t maxHistoryLength = 20, size_t maxSerializedLength = 10) {
    return std::make_unique<HistoryManager>(maxHistoryLength, maxSerializedLength);
}

/**
 * @brief 创建块操作记录的便捷函数
 *
 * @param description 操作描述
 * @return std::unique_ptr<BlockOperationRecord> 创建的块操作记录
 */
inline std::unique_ptr<BlockOperationRecord>
createBlockOperationRecord(const std::string& description = "") {
    return std::make_unique<BlockOperationRecord>(description);
}

/**
 * @brief 获取支持的历史记录类型列表
 *
 * @return std::vector<std::string> 支持的类型名称列表
 */
inline std::vector<std::string> getSupportedRecordTypes() {
    return HistoryRecordFactory::getSupportedTypes();
}

/**
 * @brief 检查是否支持指定的历史记录类型
 *
 * @param type 类型名称
 * @return bool 如果支持返回 true
 */
inline bool isRecordTypeSupported(const std::string& type) {
    return HistoryRecordFactory::isSupported(type);
}

} // namespace history
} // namespace we