# 历史记录管理系统

本项目为 WorldEdit-BE 添加了一个完整的历史记录管理系统，支持撤销(undo)和重做(redo)操作，以及序列化和反序列化功能。

## 功能特性

### 核心功能
- ✅ **撤销/重做操作**：支持多步撤销和重做
- ✅ **序列化支持**：可以序列化和反序列化历史记录
- ✅ **可配置限制**：支持设置历史记录长度和序列化长度限制
- ✅ **线程安全**：所有操作都是线程安全的
- ✅ **内存管理**：自动清理超出限制的历史记录
- ✅ **可扩展设计**：支持自定义历史记录类型

### 技术特性
- 使用 `ll::Expected<>` 进行错误处理
- 使用 `CompoundTag` 进行序列化
- 支持 RAII 和现代 C++ 特性
- 完整的类型安全设计

## 文件结构

```
src/data/
├── HistoryRecord.h              # 抽象历史记录基类
├── HistoryManager.h/.cpp        # 历史记录管理器
├── HistoryRecordFactory.h/.cpp  # 历史记录工厂类
├── BlockOperationRecord.h/.cpp  # 块操作历史记录实现
├── HistoryExample.h/.cpp        # 使用示例和测试
└── History.h                    # 总头文件
```

## 快速开始

### 1. 包含头文件

```cpp
#include "data/History.h"
```

### 2. 创建历史管理器

```cpp
// 创建历史管理器（最大20个历史记录，序列化时保存最近10个）
we::HistoryManager manager(20, 10);

// 或者使用便捷函数
auto manager = we::history::createStandardManager();
```

### 3. 创建和添加历史记录

```cpp
// 创建块操作记录
auto record = std::make_unique<we::BlockOperationRecord>("放置方块操作");

// 添加块变更信息
WithDim<BlockPos> position{BlockPos(10, 64, 20), DimensionType::Overworld};
CompoundTag oldBlock, newBlock;
oldBlock["name"] = "minecraft:air";
newBlock["name"] = "minecraft:stone";

record->addBlockChange(position, std::move(oldBlock), std::move(newBlock));

// 添加到历史管理器
manager.addRecord(std::move(record));
```

### 4. 执行撤销和重做

```cpp
// 撤销操作
if (manager.canUndo()) {
    auto undoRecord = manager.undo();
    if (undoRecord) {
        // 处理撤销逻辑
        if (auto blockRecord = dynamic_cast<we::BlockOperationRecord*>(undoRecord.get())) {
            // 恢复块状态
            for (const auto& change : blockRecord->getBlockChanges()) {
                // 将 change.position 处的方块恢复为 change.oldBlockNbt 的状态
            }
        }
    }
}

// 重做操作
if (manager.canRedo()) {
    auto redoRecord = manager.redo();
    if (redoRecord) {
        // 处理重做逻辑
        if (auto blockRecord = dynamic_cast<we::BlockOperationRecord*>(redoRecord.get())) {
            // 应用新的块状态
            for (const auto& change : blockRecord->getBlockChanges()) {
                // 将 change.position 处的方块设置为 change.newBlockNbt 的状态
            }
        }
    }
}
```

### 5. 序列化和反序列化

```cpp
// 序列化
CompoundTag nbt;
auto result = manager.serialize(nbt);
if (result) {
    // 保存 nbt 到文件或数据库
    auto binaryData = nbt.toBinaryNbt();
}

// 反序列化
we::HistoryManager newManager;
auto loadResult = newManager.deserialize(nbt);
if (loadResult) {
    // 成功加载历史记录
}
```

## 高级用法

### 自定义历史记录类型

1. **继承 HistoryRecord 基类**：

```cpp
class MyCustomRecord : public we::HistoryRecord {
private:
    std::string mData;
    
public:
    MyCustomRecord(std::string data) : mData(std::move(data)) {}
    
    ll::Expected<> serialize(CompoundTag& tag) const noexcept override {
        return ll::reflection::serialize_to(tag["data"], mData);
    }
    
    ll::Expected<> deserialize(CompoundTag const& tag) noexcept override {
        return ll::reflection::deserialize(mData, tag.at("data"));
    }
    
    std::string getType() const noexcept override {
        return "MyCustomRecord";
    }
    
    size_t getEstimatedSize() const noexcept override {
        return sizeof(MyCustomRecord) + mData.capacity();
    }
    
    std::unique_ptr<HistoryRecord> clone() const override {
        return std::make_unique<MyCustomRecord>(mData);
    }
    
    static constexpr const char* TYPE_NAME = "MyCustomRecord";
};
```

2. **注册自定义类型**：

```cpp
// 在头文件末尾添加
REGISTER_HISTORY_RECORD(MyCustomRecord, MyCustomRecord::TYPE_NAME)
```

### 配置管理器

```cpp
// 设置最大历史记录长度
manager.setMaxHistoryLength(50);

// 设置最大序列化长度
manager.setMaxSerializedLength(20);

// 检查内存使用情况
size_t memoryUsage = manager.getEstimatedMemoryUsage();
std::cout << "内存使用量: " << memoryUsage << " 字节" << std::endl;

// 清空历史记录
manager.clear();
```

### 批量操作

```cpp
auto record = std::make_unique<we::BlockOperationRecord>("批量放置");

std::vector<we::BlockOperationRecord::BlockChange> changes;
for (int i = 0; i < 100; ++i) {
    WithDim<BlockPos> pos{BlockPos(i, 64, 0), DimensionType::Overworld};
    CompoundTag oldBlock, newBlock;
    // ... 设置块数据
    changes.emplace_back(pos, std::move(oldBlock), std::move(newBlock));
}

record->addBlockChanges(std::move(changes));
manager.addRecord(std::move(record));
```

## 集成指南

### 1. 添加到现有代码

在需要历史记录功能的类中添加 HistoryManager：

```cpp
class PlayerData {
private:
    we::HistoryManager mHistoryManager;
    
public:
    PlayerData() : mHistoryManager(20, 10) {}
    
    void executeOperation(/* 参数 */) {
        // 创建历史记录
        auto record = std::make_unique<we::BlockOperationRecord>("用户操作");
        
        // 执行操作前记录状态
        // ... 收集变更信息
        
        // 执行实际操作
        // ... 修改世界状态
        
        // 添加到历史记录
        mHistoryManager.addRecord(std::move(record));
    }
    
    void undo() {
        if (mHistoryManager.canUndo()) {
            auto record = mHistoryManager.undo();
            // 应用撤销逻辑
        }
    }
    
    void redo() {
        if (mHistoryManager.canRedo()) {
            auto record = mHistoryManager.redo();
            // 应用重做逻辑
        }
    }
};
```

### 2. 序列化集成

在现有的序列化逻辑中添加历史记录序列化：

```cpp
ll::Expected<> PlayerData::serialize(CompoundTag& nbt) const noexcept {
    // 现有序列化代码...
    
    // 添加历史记录序列化
    return mHistoryManager.serialize(nbt["history"].emplace<CompoundTag>());
}

ll::Expected<> PlayerData::deserialize(CompoundTag const& nbt) noexcept {
    // 现有反序列化代码...
    
    // 添加历史记录反序列化
    if (nbt.contains("history")) {
        return mHistoryManager.deserialize(nbt.at("history").get<CompoundTag>());
    }
    return {};
}
```

## 性能考虑

### 内存管理
- 历史记录会自动清理超出限制的旧记录
- 序列化时只保存最近的指定数量记录
- 使用 `getEstimatedMemoryUsage()` 监控内存使用

### 线程安全
- 所有公共方法都是线程安全的
- 内部使用 `std::mutex` 保护共享状态
- 避免在持有锁的情况下执行耗时操作

### 序列化性能
- 只序列化必要的历史记录数量
- 使用二进制NBT格式提高序列化速度
- 支持渐进式序列化（分批处理大量记录）

## 错误处理

系统使用 `ll::Expected<>` 进行错误处理：

```cpp
auto result = manager.serialize(nbt);
if (!result) {
    // 处理序列化错误
    std::cout << "序列化失败: " << result.error().message() << std::endl;
}
```

## 测试和示例

查看 `HistoryExample.cpp` 中的完整示例：

```cpp
// 运行基本使用示例
we::examples::HistoryExample::basicUsageExample();

// 运行序列化示例
we::examples::HistoryExample::serializationExample();

// 运行配置示例
we::examples::HistoryExample::configurationExample();

// 运行线程安全示例
we::examples::HistoryExample::threadSafetyExample();
```

## 注意事项

1. **内存使用**：大量的历史记录会占用内存，建议合理设置历史记录长度限制
2. **序列化大小**：序列化时只保存最近的记录，避免文件过大
3. **线程安全**：虽然类本身是线程安全的，但应用撤销/重做逻辑时需要注意世界状态的一致性
4. **类型注册**：自定义历史记录类型需要正确注册才能被反序列化

## 扩展建议

1. **添加更多记录类型**：如区域操作记录、实体操作记录等
2. **压缩支持**：对大型历史记录进行压缩以节省内存
3. **异步序列化**：在后台线程中执行序列化操作
4. **历史记录查询**：添加按时间、类型等条件查询历史记录的功能