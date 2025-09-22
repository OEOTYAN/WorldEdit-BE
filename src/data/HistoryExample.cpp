#include "HistoryExample.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

namespace we::examples {

void HistoryExample::basicUsageExample() {
    std::cout << "=== 基本使用示例 ===" << std::endl;

    // 创建历史管理器，最大历史记录长度为5，最大序列化长度为3
    HistoryManager manager(5, 3);

    printManagerStatus(manager, "初始状态");

    // 添加一些历史记录
    for (int i = 1; i <= 3; ++i) {
        auto record = createExampleBlockRecord("操作 " + std::to_string(i), i);
        manager.addRecord(std::move(record));
        printManagerStatus(manager, "添加操作 " + std::to_string(i) + " 后");
    }

    // 执行撤销操作
    std::cout << "\n--- 撤销操作 ---" << std::endl;
    while (manager.canUndo()) {
        auto undoRecord = manager.undo();
        if (undoRecord) {
            if (auto blockRecord =
                    dynamic_cast<BlockOperationRecord*>(undoRecord.get())) {
                std::cout << "撤销: " << blockRecord->getDescription()
                          << " (块数量: " << blockRecord->getBlockCount() << ")"
                          << std::endl;
            }
        }
        printManagerStatus(manager, "撤销后");
    }

    // 执行重做操作
    std::cout << "\n--- 重做操作 ---" << std::endl;
    while (manager.canRedo()) {
        auto redoRecord = manager.redo();
        if (redoRecord) {
            if (auto blockRecord =
                    dynamic_cast<BlockOperationRecord*>(redoRecord.get())) {
                std::cout << "重做: " << blockRecord->getDescription()
                          << " (块数量: " << blockRecord->getBlockCount() << ")"
                          << std::endl;
            }
        }
        printManagerStatus(manager, "重做后");
    }
}

void HistoryExample::serializationExample() {
    std::cout << "\n=== 序列化示例 ===" << std::endl;

    // 创建并填充历史管理器
    HistoryManager originalManager(10, 5);

    for (int i = 1; i <= 7; ++i) {
        auto record =
            createExampleBlockRecord("序列化测试操作 " + std::to_string(i), i * 2);
        originalManager.addRecord(std::move(record));
    }

    printManagerStatus(originalManager, "序列化前的原始管理器");

    // 序列化
    CompoundTag serializedData;
    auto        serializeResult = originalManager.serialize(serializedData);

    if (serializeResult) {
        std::cout << "序列化成功" << std::endl;

        // 反序列化到新的管理器
        HistoryManager deserializedManager;
        auto deserializeResult = deserializedManager.deserialize(serializedData);

        if (deserializeResult) {
            std::cout << "反序列化成功" << std::endl;
            printManagerStatus(deserializedManager, "反序列化后的管理器");
        } else {
            std::cout << "反序列化失败" << std::endl;
        }
    } else {
        std::cout << "序列化失败" << std::endl;
    }
}

void HistoryExample::configurationExample() {
    std::cout << "\n=== 配置示例 ===" << std::endl;

    HistoryManager manager;

    std::cout << "默认配置:" << std::endl;
    std::cout << "  最大历史记录长度: " << manager.getMaxHistoryLength() << std::endl;
    std::cout << "  最大序列化长度: " << manager.getMaxSerializedLength() << std::endl;

    // 添加记录直到超过默认限制
    for (int i = 1; i <= 25; ++i) {
        auto record = createExampleBlockRecord("配置测试操作 " + std::to_string(i));
        manager.addRecord(std::move(record));
    }

    printManagerStatus(manager, "添加25个记录后");

    // 修改配置
    manager.setMaxHistoryLength(15);
    manager.setMaxSerializedLength(8);

    std::cout << "修改配置后:" << std::endl;
    std::cout << "  最大历史记录长度: " << manager.getMaxHistoryLength() << std::endl;
    std::cout << "  最大序列化长度: " << manager.getMaxSerializedLength() << std::endl;

    printManagerStatus(manager, "修改配置后");

    // 检查内存使用情况
    std::cout << "估计内存使用量: " << manager.getEstimatedMemoryUsage() << " 字节"
              << std::endl;
}

void HistoryExample::customRecordTypeExample() {
    std::cout << "\n=== 自定义历史记录类型示例 ===" << std::endl;

    // 这里展示如何使用已有的 BlockOperationRecord
    // 在实际项目中，你可以创建更多自定义类型

    auto blockRecord = std::make_unique<BlockOperationRecord>("自定义块操作");

    // 添加一些块变更
    WithDim<BlockPos> pos1{BlockPos(10, 64, 20), DimensionType{0}}; // Overworld
    WithDim<BlockPos> pos2{BlockPos(11, 64, 20), DimensionType{0}};

    CompoundTag oldBlock, newBlock;
    oldBlock["name"] = "minecraft:air";
    newBlock["name"] = "minecraft:stone";

    blockRecord->addBlockChange(pos1, oldBlock, newBlock);
    blockRecord->addBlockChange(pos2, oldBlock, newBlock);

    std::cout << "创建的自定义记录:" << std::endl;
    std::cout << "  类型: " << blockRecord->getType() << std::endl;
    std::cout << "  描述: " << blockRecord->getDescription() << std::endl;
    std::cout << "  块数量: " << blockRecord->getBlockCount() << std::endl;
    std::cout << "  估计大小: " << blockRecord->getEstimatedSize() << " 字节"
              << std::endl;

    // 测试序列化
    CompoundTag serializedRecord;
    auto        serializeResult = blockRecord->serialize(serializedRecord);

    if (serializeResult) {
        std::cout << "记录序列化成功" << std::endl;

        // 测试克隆
        auto clonedRecord = blockRecord->clone();
        std::cout << "记录克隆成功，类型: " << clonedRecord->getType() << std::endl;
    }
}

void HistoryExample::threadSafetyExample() {
    std::cout << "\n=== 线程安全示例 ===" << std::endl;

    HistoryManager manager(50, 20);

    // 创建多个线程同时操作历史管理器
    std::vector<std::thread> threads;

    // 添加记录的线程
    for (int threadId = 0; threadId < 3; ++threadId) {
        threads.emplace_back([&manager, threadId]() {
            for (int i = 0; i < 10; ++i) {
                auto record = createExampleBlockRecord(
                    "线程" + std::to_string(threadId) + "操作" + std::to_string(i)
                );
                manager.addRecord(std::move(record));
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }

    // 撤销/重做操作的线程
    threads.emplace_back([&manager]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        for (int i = 0; i < 5; ++i) {
            if (manager.canUndo()) {
                manager.undo();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            if (manager.canRedo()) {
                manager.redo();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    printManagerStatus(manager, "线程安全测试完成后");
}

std::unique_ptr<BlockOperationRecord> HistoryExample::createExampleBlockRecord(
    const std::string& description,
    size_t             blockCount
) {
    auto record = std::make_unique<BlockOperationRecord>(description);

    // 创建一些示例块变更
    for (size_t i = 0; i < blockCount; ++i) {
        WithDim<BlockPos> pos{
            BlockPos(static_cast<int>(i), 64, 0),
            DimensionType{0}
        }; // Overworld

        CompoundTag oldBlock, newBlock;
        oldBlock["name"] = "minecraft:air";
        oldBlock["id"]   = static_cast<int>(i % 10);

        newBlock["name"] = "minecraft:stone";
        newBlock["id"]   = static_cast<int>((i + 1) % 10);

        record->addBlockChange(pos, std::move(oldBlock), std::move(newBlock));
    }

    return record;
}

void HistoryExample::printManagerStatus(
    const HistoryManager& manager,
    const std::string&    title
) {
    std::cout << "[" << title << "]" << std::endl;
    std::cout << "  大小: " << manager.size()
              << ", 可撤销: " << (manager.canUndo() ? "是" : "否")
              << ", 可重做: " << (manager.canRedo() ? "是" : "否") << std::endl;
}

} // namespace we::examples