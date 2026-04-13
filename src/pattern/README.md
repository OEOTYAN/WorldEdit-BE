# Pattern 模块说明

本文档描述当前重构版 `src/pattern` 模块已经实现的语法、运行时行为、示例，以及与旧版 `old/core/store/*Pattern*` 相比尚未补齐的能力。

## 1. 当前范围

当前 Pattern 模块只实现了 **Block List Pattern**，并已经接入 `set` 命令的 pattern 重载。

当前涉及的主要文件：

- `PatternAst.h`：AST 定义
- `PatternParser.h/.cpp`：字符串解析
- `Pattern.h/.cpp`：运行时接口与 AST 分发
- `BlockListPattern.h/.cpp`：加权方块列表的编译与求值

当前命令入口：

- `set <pattern>`

也就是说，目前 README 中提到的 pattern 语法，都是给 `set` 使用的。

---

## 2. 语法总览

### 2.1 顶层结构

一个 pattern 由一个或多个“条目”组成，条目之间用英文逗号 `,` 分隔：

```text
pattern := entry ("," entry)*
```

每个条目的语法如下：

```text
entry := [weight "%"] block [":" data]
```

含义如下：

- `weight`：该条目的权重，可省略；省略时默认是 `1`
- `block`：方块说明，可以是静态方块名，也可以是表达式
- `data`：方块 data，可省略；可为数字或表达式

---

## 3. 词法与分隔规则

### 3.1 顶层逗号

顶层逗号用于分隔条目，但 **括号表达式内部** 和 **exprtk 单引号字符串内部** 的逗号不会被当作分隔符。

例如：

```text
1%minecraft:stone, 1%(rtid(0))
```

这是两个条目。

而：

```text
(foo(1,2))%minecraft:stone
```

这里的 `,` 在表达式括号内部，不会拆条目。

### 3.2 空白

条目前后的空格、逗号两侧的空格、`%` 后的空格、块说明后的空格，都会被裁剪。

例如下面几种写法在解析结果上等价：

```text
minecraft:stone
1%minecraft:stone
1 % minecraft:stone
  1   %   minecraft:stone  
```

### 3.3 括号的意义

当前 parser 里，`()` 只用于表示“显式表达式边界”。

尤其对 `block` 字段来说：

- `minecraft:stone` 会被当成静态块名
- `(rtid(0))` 会被当成动态块表达式

如果想让 block 字段走表达式路径，必须把整个表达式包在一层括号里。

### 3.4 exprtk 字符串

因为外层 pattern 现在使用 `()` 标记表达式，所以 exprtk 内部仍然可以继续使用单引号字符串。

例如：

```text
(rtid(1))
```

或更一般地，在 exprtk 内部使用字符串函数时，单引号不会和 pattern 层语法冲突。

---

## 4. 各字段详细说明

## 4.1 weight

`weight` 位于 `%` 左边。

- 可省略
- 可以是字面量数字
- 也可以是 exprtk 表达式

如果省略，则默认权重为 `1.0`。

示例：

```text
minecraft:stone
2%minecraft:stone
(x+1)%minecraft:stone
```

当前实现中：

- 每次对某个坐标求值时，都会先计算所有条目的权重
- 权重小于 `0` 时会被钳制为 `0`
- 所有权重之和小于等于 `0` 时，最终返回空气

### 4.2 block

`block` 位于 `%` 右边、可选 `:data` 左边。

它有两种形式：

#### 形式 A：静态块名

例如：

```text
minecraft:stone
minecraft:wool
stone
```

当前 parser 对块名字面量的识别规则是：

- 允许字符：字母、数字、`_`、`.`、`-`
- 允许一个命名空间分隔冒号，例如 `minecraft:stone`

注意：当前 parser 不会主动给短名补 `minecraft:` 前缀，而是把名字原样交给注册表解析。

#### 形式 B：动态块表达式

必须写成一整个括号表达式：

```text
(rtid(0))
```

当前运行时行为：

1. 先执行 exprtk 表达式
2. 如果表达式结果里包含字符串结果，则把该字符串当作块名解析
3. 否则把标量结果四舍五入后当作 runtime id 解析

也就是说，动态块表达式支持两条路径：

- 字符串路径：表达式返回块名
- 数值路径：表达式返回 runtime id

### 4.3 data

`data` 位于最后一个 `:` 右边。

可以是：

- 数字字面量
- exprtk 表达式

示例：

```text
minecraft:stone:0
minecraft:wool:(x)
minecraft:planks:(y+z)
```

当前运行时行为：

- `data` 表达式结果会先四舍五入
- 如果结果小于 `0`，则会被钳制为 `0`
- 随后按 `tryGetFromRegistry(name, data)` 解析

---

## 5. 当前支持的表达式能力

## 5.1 变量

当前 pattern 表达式中，已注册的坐标变量只有：

- `x`
- `y`
- `z`

它们表示当前正在求值的世界坐标。

## 5.2 exprtk 内建能力

除了上面的坐标变量，还可以使用 exprtk 默认提供的：

- 数学常量
- 常见数学函数
- exprtk 的标准表达式语法

具体以当前项目引入的 exprtk 版本为准。

---

## 6. 运行时求值语义

## 6.1 编译时机

当前实现中：

- pattern 字符串先被解析成 AST
- AST 再被编译成运行时结构
- exprtk 表达式在编译阶段只编译一次
- 实际遍历区域时只做重复求值，不重复编译

## 6.2 选块流程

对于一个坐标 `pos`，`BlockListPattern::pickBlock(pos)` 的行为是：

1. 计算每个条目的权重
2. 将负权重钳制为 `0`
3. 用 ll 的随机工具按加权概率选中一个条目
4. 解析该条目的 block
5. 如果有 data，则继续按 data 解析目标方块状态

## 6.3 解析失败时的处理

当前实现里，如果某一步解析失败：

- 静态块名找不到：最终回退为空气
- 动态字符串块名找不到：最终回退为空气
- 动态 runtime id 找不到：最终回退为空气
- 所有权重都不大于 `0`：最终回退为空气

也就是说，目前 pattern 运行时是“尽量给出一个块；失败时回退空气”，而不是在遍历过程中中断。

---

## 7. 示例

下面示例都基于当前 parser 和运行时实现。

## 7.1 单个静态方块

```text
minecraft:stone
```

含义：整片区域全部设置为 `minecraft:stone`。

---

## 7.2 两种方块等概率混合

```text
1%minecraft:stone,1%minecraft:dirt
```

也可以简写为：

```text
minecraft:stone,minecraft:dirt
```

因为省略权重时默认是 `1`。

---

## 7.3 不等权重混合

```text
70%minecraft:stone,30%minecraft:dirt
```

含义：每个位置独立地按 70:30 随机选择石头或泥土。

---

## 7.4 带 data 的静态方块

```text
minecraft:wool:14
```

含义：使用 `minecraft:wool` 且 data 为 `14`。

---

## 7.5 data 使用表达式

```text
minecraft:wool:(x+y)
```

含义：data 由当前位置坐标表达式决定，结果会四舍五入后再使用。

---

## 7.6 权重使用表达式

```text
(x+1)%minecraft:stone,1%minecraft:dirt
```

含义：石头的权重随 `x` 增长而变化，泥土权重固定为 `1`。

注意：如果某个位置上 `(x+1)` 小于 `0`，则该条权重会被钳制为 `0`。

---

## 7.7 block 使用字符串表达式

```text
(rtid(0))
```

含义：调用 `rtid(0)` 得到一个字符串块名，再按块名解析该方块。

这是当前“块名由表达式决定”的最小可用形式。

---

## 7.8 动态块名 + 动态 data

```text
(rtid(0)):(z)
```

含义：块名来自表达式，data 也来自表达式。

---

## 7.9 混合静态块与动态块

```text
3%minecraft:stone,1%(rtid(0))
```

含义：大多数情况下放石头，少数情况下由动态表达式返回块名。

---

## 7.10 命令示例

```text
/set 70%minecraft:stone,30%minecraft:dirt
```

```text
/set minecraft:wool:(x+y)
```

```text
/set 3%minecraft:stone,1%(rtid(0))
```

---

## 8. 当前实现细节与注意事项

## 8.1 block 表达式必须显式加括号

下面这种写法：

```text
rtid(0)
```

在 `block` 位置当前 **不会** 按表达式解析，因为 parser 会先按块名字面量规则读 token。

如果要让 block 走表达式路径，必须写成：

```text
(rtid(0))
```

## 8.2 weight / data 的表达式更宽松

`weight` 和 `data` 位置本身就是“表达式友好”的：

- 纯数字会按字面量处理
- 其他内容会按运行时表达式处理

所以：

```text
(x+1)%minecraft:stone
minecraft:wool:(x+y)
```

都合法。

## 8.3 当前错误提示还比较粗

命令层现在对 parser / pattern 创建失败时，只返回：

- `Invalid pattern`
- `Failed to create pattern`

还没有把更细的具体错误原因透传给用户。

---

## 9. 与旧版相比，目前还缺什么

旧版入口见：

- `old/core/store/Pattern.cpp`
- `old/core/store/BlockListPattern.cpp`
- `old/core/store/ClipboardPattern.*`
- `old/core/store/GradientPattern.*`

旧版 Pattern 工厂支持：

- 普通 BlockListPattern
- `#hand`
- `#clipboard`
- `#lighten`
- `#darken`

而当前重构版 **只实现了普通 Block List Pattern**。

下面按功能列出当前相对旧版尚未补齐的部分。

## 9.1 快捷模式还没补

当前还没有：

- `#hand`
- `#clipboard`
- `#lighten`
- `#darken`

也就是说，旧版里“从手持方块取样”“从剪贴板取样”“按梯度变亮/变暗”的 pattern 入口，目前都没有迁移到新模块。

## 9.2 SNBT / 复杂方块构造还没补

旧版 `BlockListPattern.cpp` 支持从 SNBT 构造 block，甚至进一步处理：

- 主 block
- extra block
- block entity

当前新模块还没有这些能力。

目前只能走：

- 块名 + 可选 data
- 动态块名表达式
- 动态 data 表达式

不能直接在 pattern 里写 SNBT 方块描述。

## 9.3 旧版 eval 函数集合还没迁移

旧版 `old/core/eval/Eval.cpp` 里提供了很多和世界上下文相关的函数/变量能力，例如：

- `id`
- `runtimeid`
- `biome`
- `hasplayer`
- 以及更多旧版上下文函数

当前新模块里，表达式层只明确注册了：

- `x`
- `y`
- `z`
- exprtk 内建能力

所以和旧版相比，**表达式上下文能力还非常不完整**。

## 9.4 旧版特殊语法兼容性还没补

旧版 parser/扫描器里存在一批历史语法与特殊分支，例如：

- 单引号包住函数体的旧式表达式写法
- `rt'...'` 这种特殊形式
- 旧版的 `num` / `function` / `rtfunction` 扫描路径
- 一部分历史兼容处理
- SNBT 的特殊识别分支

当前新 parser 已经换成更明确的 AST 方案，因此这些旧格式大多没有保留。

换句话说：**新模块目前更规整，但兼容旧输入的范围还不够广。**

## 9.5 自动补全和更友好的诊断还没补

相对旧版，现在还缺：

- 更详细的 parse 错误信息透传
- 更明确的运行时解析失败诊断
- 可能的 pattern 语法补全/提示
- 更丰富的单元测试或示例验证

## 9.6 更完整的 Pattern 类型体系还没补

当前 AST 只有：

- `BlockListPatternAst`

还没有为下面这些类型建立正式 AST / 运行时实现：

- Clipboard Pattern
- Gradient Pattern
- Hand Pattern
- 未来可能的更复杂 pattern 组合

---

## 10. 当前已完成的重构收益

虽然还没有追平旧版全部功能，但当前版本已经具备几个明显优点：

- 有清晰的 AST，而不是旧版那种手写扫描 + 字符串拼接状态机
- 表达式编译只做一次，运行时重复求值不会反复编译
- 不依赖异常控制流
- block / weight / data 的职责边界更清晰
- 更容易继续扩展新的函数注册与 pattern 类型

因此，当前版本适合作为后续继续补全旧版功能的基础。

---

## 11. 建议的后续补全顺序

如果要继续向旧版靠齐，建议优先级如下：

1. 先把表达式上下文能力补齐
   - 例如 `id`、`runtimeid`、`biome` 等旧版常用函数
2. 再补 `#hand`
   - 这是最简单、最实用的快捷入口之一
3. 再补 `#clipboard`
   - 需要和剪贴板系统对接
4. 再补 `#lighten` / `#darken`
   - 需要重新设计梯度模型
5. 最后再处理 SNBT / extra block / block entity
   - 这部分复杂度最高，也最容易牵连更多系统

---

## 12. 一句话总结

当前重构版 Pattern 已经能稳定支持：

- 加权方块列表
- 坐标表达式权重
- 动态 data
- 动态 block 表达式

但与旧版相比，仍然缺少快捷模式、SNBT 构造、旧版世界上下文函数集，以及 `#clipboard` / `#lighten` / `#darken` 等完整 pattern 类型。
