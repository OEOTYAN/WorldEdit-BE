#pragma once

#include "HistoryRecord.h"
#include "worldedit/Global.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace we {

/**
 * @brief 历史记录工厂类
 *
 * 负责根据类型字符串创建相应的历史记录实例。
 * 使用注册机制支持不同类型的历史记录。
 */
class HistoryRecordFactory {
private:
    using CreatorFunction = std::function<std::unique_ptr<HistoryRecord>()>;
    static std::unordered_map<std::string, CreatorFunction> sCreators;

public:
    /**
     * @brief 注册历史记录创建器
     *
     * @param type 历史记录类型名称
     * @param creator 创建器函数
     */
    static void registerCreator(const std::string& type, CreatorFunction creator);

    /**
     * @brief 根据类型创建历史记录实例
     *
     * @param type 历史记录类型名称
     * @return std::unique_ptr<HistoryRecord> 创建的历史记录实例，失败时返回 nullptr
     */
    static std::unique_ptr<HistoryRecord> create(const std::string& type);

    /**
     * @brief 检查是否支持指定类型
     *
     * @param type 历史记录类型名称
     * @return bool 如果支持返回 true
     */
    static bool isSupported(const std::string& type);

    /**
     * @brief 获取所有支持的类型列表
     *
     * @return std::vector<std::string> 支持的类型名称列表
     */
    static std::vector<std::string> getSupportedTypes();
};

/**
 * @brief 自动注册模板类
 *
 * 用于在编译时自动注册历史记录类型
 *
 * @tparam T 历史记录类型，必须继承自 HistoryRecord
 */
template <typename T>
class HistoryRecordRegistrar {
public:
    explicit HistoryRecordRegistrar(const std::string& type) {
        static_assert(
            std::is_base_of_v<HistoryRecord, T>,
            "T must inherit from HistoryRecord"
        );

        HistoryRecordFactory::registerCreator(
            type,
            []() -> std::unique_ptr<HistoryRecord> { return std::make_unique<T>(); }
        );
    }
};

/**
 * @brief 用于自动注册的宏
 *
 * 在源文件中使用此宏来自动注册历史记录类型
 */
#define REGISTER_HISTORY_RECORD(Type, TypeName)                                          \
    namespace {                                                                          \
    static we::HistoryRecordRegistrar<Type> g_registrar_##Type(TypeName);                \
    }

} // namespace we