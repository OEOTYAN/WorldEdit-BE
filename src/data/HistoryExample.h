#pragma once

/**
 * @file HistoryExample.h
 * @brief 历史记录管理系统使用示例
 *
 * 本文件展示如何使用历史记录管理系统，包括：
 * 1. 创建和配置 HistoryManager
 * 2. 创建和添加历史记录
 * 3. 执行撤销和重做操作
 * 4. 序列化和反序列化历史记录
 */

#include "BlockOperationRecord.h"
#include "HistoryManager.h"
#include "worldedit/Global.h"

namespace we::examples {

/**
 * @brief 历史记录管理系统使用示例类
 */
class HistoryExample {
public:
    /**
     * @brief 基本使用示例
     *
     * 演示创建历史管理器、添加记录、撤销和重做操作
     */
    static void basicUsageExample();

    /**
     * @brief 序列化示例
     *
     * 演示如何序列化和反序列化历史记录管理器
     */
    static void serializationExample();

    /**
     * @brief 配置示例
     *
     * 演示如何配置历史记录长度限制
     */
    static void configurationExample();

    /**
     * @brief 自定义历史记录类型示例
     *
     * 演示如何创建自定义的历史记录类型
     */
    static void customRecordTypeExample();

    /**
     * @brief 线程安全示例
     *
     * 演示历史记录管理器的线程安全特性
     */
    static void threadSafetyExample();

private:
    /**
     * @brief 创建示例块变更记录
     *
     * @param description 操作描述
     * @param blockCount 块数量
     * @return std::unique_ptr<BlockOperationRecord> 创建的记录
     */
    static std::unique_ptr<BlockOperationRecord>
    createExampleBlockRecord(const std::string& description, size_t blockCount = 1);

    /**
     * @brief 输出历史管理器状态
     *
     * @param manager 历史管理器
     * @param title 标题
     */
    static void
    printManagerStatus(const HistoryManager& manager, const std::string& title);
};

} // namespace we::examples