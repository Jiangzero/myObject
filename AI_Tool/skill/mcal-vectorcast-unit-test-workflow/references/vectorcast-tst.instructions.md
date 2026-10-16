---
applyTo: "**/*.tst"
description: "VectorCAST .tst 单元测试脚本编写规范知识库。当编写、修改或审查 VectorCAST .tst 测试脚本时自动应用：包含 .tst 脚本语法格式规则、指针/数组/malloc 写法、STUB 与 EXPECTED 用户代码块写法、TT_RET 错误码常量表、寄存器掩码与 DMA 事件常量、MC/DC 测试设计与边界值策略。"
---

# VectorCAST `.tst` 测试脚本编写规范

本知识库用于指导生成符合 VectorCAST 工具规范的 `.tst` 单元测试脚本（嵌入式 C，AUTOSAR 风格驱动）。

## 1. 脚本基本结构

每个 `.tst` 文件以脚本特性头开始，随后是一系列测试用例块。

```
-- Environment    : <ENV_NAME>
-- Unit(s) Under Test: <Unit1> <Unit2> ...

-- Script Features
TEST.SCRIPT_FEATURE:C_DIRECT_ARRAY_INDEXING
TEST.SCRIPT_FEATURE:CPP_CLASS_OBJECT_REVISION
TEST.SCRIPT_FEATURE:MULTIPLE_UUT_SUPPORT
TEST.SCRIPT_FEATURE:REMOVED_CL_PREFIX
TEST.SCRIPT_FEATURE:MIXED_CASE_NAMES
TEST.SCRIPT_FEATURE:STATIC_HEADER_FUNCS_IN_UUTS
TEST.SCRIPT_FEATURE:VCAST_MAIN_NOT_RENAMED
```

单个测试用例骨架：

```
TEST.UNIT:<Unit>
TEST.SUBPROGRAM:<FunctionName>
TEST.NEW
TEST.NAME:<UniqueTestName>
TEST.STUB:<Unit>.<StubbedFunction>      -- 可选，按需打桩

TEST.VALUE:<赋值>                        -- 输入 / 全局变量 / 桩返回值
TEST.EXPECTED:<期望>                     -- 输出 / 返回值断言
TEST.END
```

- 每个用例命名必须**唯一**，建议用连续编号（如 `SUT_Xxx_001`）。
- 在每个用例上方用 `--` 注释写明：Description（场景）和 MC/DC（覆盖的判定分支），便于审查。

## 2. 指针 / 数组 / 内存分配写法（关键规则）

VectorCAST 对**指针数组类型的全局变量**有固定的索引展开规则，最常见的错误源。

### 规则：指针数组的成员访问用 `[index][0].FIELD`，**不要用 `->`**

源码若声明为：
```c
static TT_ICC_Type *      ICC_Hw_PubBasePtr[N];          // 指针数组
static const CfgType *    ICC_Hw_PubCtrlConfigPtr[N];    // const 指针数组
```

`.tst` 写法：
```
-- 先为该数组元素分配内存
TEST.VALUE:<Unit>.<<GLOBAL>>.ICC_Hw_PubBasePtr[0]:<<malloc 1>>
-- 再访问其成员：注意是 [index][0].FIELD
TEST.VALUE:<Unit>.<<GLOBAL>>.ICC_Hw_PubBasePtr[0][0].FLG:0x10
TEST.VALUE:<Unit>.<<GLOBAL>>.ICC_Hw_PubCtrlConfigPtr[0]:<<malloc 1>>
TEST.VALUE:<Unit>.<<GLOBAL>>.ICC_Hw_PubCtrlConfigPtr[0][0].WorkMode:ICC_WORK_IRQ
```

### 内存分配 / 空指针

| 目的 | 写法 |
|------|------|
| 分配 1 个元素 | `...ArrayName[index]:<<malloc 1>>` |
| 分配 N 字节缓冲区（如 TxBuf） | `....TxBuf:<<malloc 10>>` |
| 置空指针 | `...Ptr:<<null>>` |

- 必须**先 `<<malloc>>` 分配，再对其成员赋值**，否则解引用空指针。
- 若要测试"先有效后置空"的场景，可先 `<<malloc 1>>` 再在后面用 `<<null>>` 覆盖。

### 寄存器基址指针分配（强制）

当被测路径会访问硬件寄存器（例如出现 `Uart_Hw_PubBasePtr[HwUnit]->CTRL/STAT/DATA/...` 读写）时，测试用例中必须先为对应基址指针分配空间：

```
TEST.VALUE:<Unit>.<<GLOBAL>>.Uart_Hw_PubBasePtr[0]:<<malloc 1>>
```

- 即使该用例主要验证参数校验，也要确保不会在执行路径中触发空指针寄存器访问。
- 建议把该分配放在每个相关用例里，而不是依赖前一个用例状态。

### 全局变量与参数

```
-- 全局变量（含 static 文件级变量）
TEST.VALUE:<Unit>.<<GLOBAL>>.<GlobalVar>:<value>
-- 被测函数的入参
TEST.VALUE:<Unit>.<Function>.<ParamName>:<value>
-- 桩函数的返回值
TEST.VALUE:<Unit>.<StubFunction>.return:<value>
-- 通过指针出参返回（如 PriGetChannel 的 Channel*）
TEST.VALUE:<Unit>.<StubFunction>.<OutParam>[0]:<value>
```

> **`const` 全局字段可直接写入**：VectorCAST 测试环境下，即使源码中声明为 `const` 的全局变量（如配置宏数据结构），也可以直接用 `TEST.VALUE` 赋值，无需 `VALUE_USER_CODE` 强制类型转换。例如：
> ```
> TEST.VALUE:<Unit>.<<GLOBAL>>.MacroData.CfgCls:CFG_CLS_PRECOMPILE
> ```

## 3. 期望值（断言）写法

```
-- 简单数值断言
TEST.EXPECTED:<Unit>.<Function>.return:0x0
TEST.EXPECTED:<Unit>.<<GLOBAL>>.<GlobalVar>:1
-- 寄存器/结构体成员断言
TEST.EXPECTED:<Unit>.<<GLOBAL>>.ICC_Hw_PubBasePtr[0][0].INT_CLR:0x2000
```

### 复杂返回值用 `EXPECTED_USER_CODE`（宏组合无法直接写常量时）

当期望值是宏运算结果（例如 `TT_SET_STATUS_TYPE(...)`）时，用用户代码块：

```
TEST.EXPECTED_USER_CODE:<Unit>.<Function>.return
 {{ <<<Unit>.<Function>.return>> == ( TT_SET_STATUS_TYPE(MODULE_STATUS_DEFAULT(SI_XXX), TT_RET_PARAM_NULL_PTR_ERR) ) }}
TEST.END_EXPECTED_USER_CODE:
```

### 无返回值函数（`void`）的预期输出规则（强制）

- `void` 函数**禁止**写 `TEST.EXPECTED:<Unit>.<Function>.return:...`。
- `void` 函数必须改为断言可观测副作用：全局变量、寄存器字段、出参、状态位等。
- **项目强制规则**：每个测试用例至少包含 1 条预期输出（`TEST.EXPECTED` 或 `TEST.EXPECTED_USER_CODE`），即使被测函数是 `void`。

示例：

```
-- void 函数：不写 return 断言，改为断言副作用
TEST.UNIT:<Unit>
TEST.SUBPROGRAM:<VoidFunction>
TEST.NEW
TEST.NAME:<UniqueName>
TEST.VALUE:<Unit>.<<GLOBAL>>.<StateBefore>:0
TEST.EXPECTED:<Unit>.<<GLOBAL>>.<StateAfter>:1
TEST.END
```

## 4. 打桩与桩出参写法

### 何时应该打桩

以下被调用函数**优先打桩**，以便独立控制被测函数的各分支：

- 硬件寄存器访问 / DMA 操作
- DET（错误检测）、DEM（诊断事件）上报
- 回调函数（cb_event）
- 私有/底层函数（`*_Pri*`、底层 `Hw` 例程）
- OS / 中断相关调用

> **每个被打桩的函数至少覆盖「成功返回」与「失败返回」两种值**——很多分支恰恰来自桩的返回值。
> 例：`ICC_Hw_Init.return:0`（成功）与 `ICC_Hw_Init.return:TT_RET_STATUS_ERR`（失败）各一个用例。

```
-- 声明打桩（替换真实实现）
TEST.STUB:<Unit>.<Function>
-- 桩返回值
TEST.VALUE:<Unit>.<Function>.return:0
```

### 桩函数需要修改出参 / 全局变量时用 `STUB_VAL_USER_CODE`

当需要让桩函数通过指针出参写值（例如设置 `*Event` 或清 busy 标志）以覆盖调用方的特定分支时：

```
TEST.STUB_VAL_USER_CODE:<Unit>.<PrivateFunction>
  *Event = 0x100U;
  ICC_Hw_PubTransferInfo[0].rx_isbusy = FALSE;
TEST.END_STUB_VAL_USER_CODE:
```

### `STUB_VAL_USER_CODE` 控制桩函数返回值

当需要通过用户代码计算桩返回值时，必须在路径末尾加 `.return` 后缀，并用 `<<path>>` 赋值语法。**禁止**使用外层 `<<` `>>` 代码块 + `return` 语句。

```
-- ✅ 正确：用 .return 后缀 + <<path>> 赋值
TEST.STUB_VAL_USER_CODE:Base_Tools.Base_MemSet.return
<<uut_prototype_stubs.Base_MemSet.return>> = ((uint8 *)(&TransferInfo[0] + 1) - (uint8 *)(&TransferInfo[0]));
TEST.END_STUB_VAL_USER_CODE:

-- ✅ 正确：返回固定值
TEST.STUB_VAL_USER_CODE:Base_Tools.Base_MemSet.return
<<uut_prototype_stubs.Base_MemSet.return>> = 0U;
TEST.END_STUB_VAL_USER_CODE:

-- ❌ 错误：用 << >> 代码块包裹 + return 语句（参数名在桩上下文中不可用）
TEST.STUB_VAL_USER_CODE:Base_Tools.Base_MemSet
<<
  return n_bytes;   -- n_bytes 在桩环境中不存在！
>>
TEST.END_STUB_VAL_USER_CODE:
```

### 用户代码块（`VALUE_USER_CODE` / `STUB_VAL_USER_CODE` / `EXPECTED_USER_CODE`）格式规则（强制）

**所有用户代码块内容直接跟在标签行后面，不要用独立的 `<<` 和 `>>` 行包裹。**

```
-- ✅ 正确：代码直接写在标签行之后
TEST.VALUE_USER_CODE:<Unit>.<<GLOBAL>>.TT_LINU[0][0].STAT
<<Unit.<<GLOBAL>>.TT_LINU[0][0].STAT>> = (uint32)(1U << TT_LIN_STAT_STAT_Pos);
TEST.END_VALUE_USER_CODE:

TEST.STUB_VAL_USER_CODE:Dma_Drv.Dma_Drv_ChannelDisable
  static int call_count = 0;
  call_count++;
  if (call_count == 1) { return TT_RET_SUCCESS; }
  else { call_count = 0; return TT_RET_STATUS_ERR; }
TEST.END_STUB_VAL_USER_CODE:

-- ❌ 错误：用独立的 << 和 >> 行包裹（VectorCAST 解析报错）
TEST.VALUE_USER_CODE:<Unit>.<<GLOBAL>>.TT_LINU[0][0].STAT
<<
  <<Unit.<<GLOBAL>>.TT_LINU[0][0].STAT>> = ...;
>>
TEST.END_VALUE_USER_CODE:
```

### 多次调用桩：循环场景中的动态返回值

当被测函数在 `while` 循环中多次调用同一函数（如每次读寄存器 STAT），需用静态计数器模拟动态响应：

```
TEST.STUB_VAL_USER_CODE:<Unit>.<TargetFunction>
  static int call_count = 0;
  call_count++;
  if (call_count == 1) {
    <<uut_prototype_stubs.<TargetFunction>.return>> = 0x1U;  /* 第 1 次：循环继续 */
  } else {
    call_count = 0;
    <<uut_prototype_stubs.<TargetFunction>.return>> = 0x0U;  /* 第 2 次：循环退出 */
  }
TEST.END_STUB_VAL_USER_CODE:
```

> 注意：静态局部变量 `call_count` 会跨用例保留。若用例执行顺序影响计数，可在代码开头加 `call_count = 0;` 复位逻辑，或改用全局标志。

> **常见陷阱**：若被测函数在入口处会用入参覆盖某全局字段（如 `tx_num = Length;`），则无法通过预设该全局字段来控制内部 `while`/`if` 行为。此时应改用**打桩 + `STUB_VAL_USER_CODE`** 强制设定内部状态/出参，而不是靠"实际执行"。

## 5. 错误码常量表（TT_RetType）

| 常量 | 值 | 含义 |
|------|-----|------|
| `TT_RET_SUCCESS` | 0x0 | 成功 |
| `TT_RET_STATUS_ERR` | 0x10 | 状态错误（busy 冲突等） |
| `TT_RET_PARAM_ERR` | 0x30 | 通用参数错误 |
| `TT_RET_PARAM_CHANNEL_ERR` | 0x3B | 通道号越界 |

常配合 `TT_SET_STATUS_TYPE(MODULE_STATUS_DEFAULT(SI_XXX), <错误码>)` 使用的语义错误码（值由模块封装，断言用 `EXPECTED_USER_CODE`）：
`TT_RET_PARAM_INVALID_PTR_ERR`、`TT_RET_STATUS_INIT_ERR`、`TT_RET_STATUS_UNINIT_ERR`、`TT_RET_PARAM_NULL_PTR_ERR`、`TT_RET_PARAM_LEN_ERR`、`TT_RET_PARAM_VALUE_ERR`、`TT_RET_IO_BUSY_ERR`。

> 注意：当上层把底层原始错误码（如 0x10/0x64）经 `PriRetCheck` 重新封装时，期望返回值应写**封装后**的语义码，而非原始码。

## 6. 寄存器掩码与 DMA 事件常量

| 常量 | 值 | 说明 |
|------|-----|------|
| `TT_ICC_FLG_RX_FIFO_EMPTY_Msk` | 0x10 | RX FIFO 空 |
| `TT_ICC_FLG_TX_FIFO_FULL_Msk` | 0x20 | TX FIFO 满 |
| `TT_ICC_DATA_PARITY_ERR_Msk` | 0x200 | 奇偶校验错误 |
| `DMA_EVENT_TERMINAL_COUNT_REQUEST` | 1 | DMA 传输完成 |
| `DMA_EVENT_ERROR` | 2 | DMA 错误 |

> 设置寄存器 `FLG` 字段时按位组合掩码控制 FIFO 状态，从而精确驱动 `while`/`if` 走向目标分支。

## 7. MC/DC 测试设计与边界值策略

### 生成用例前先写 MC/DC 判定对分析（强制步骤）

对每个复合判定，先**显式列出 Decision、各 Condition、以及每个 Condition 的 MC/DC 独立影响对**，再把每个 Pair 映射到具体用例。这能避免「分支覆盖对了但 MC/DC 不完整」。

示例，判定 `D = (A && B) || (!A && C)`：

```
Decision D: (A && B) || (!A && C)
Conditions: A, B, C

MC/DC Pairs（每个条件需一对：仅该条件翻转导致 D 翻转）
  A: (A=0,B=*,C=0)->FALSE  vs  (A=1,B=1,C=*)->TRUE
  B: (A=1,B=0,C=*)->FALSE  vs  (A=1,B=1,C=*)->TRUE
  C: (A=0,B=*,C=0)->FALSE  vs  (A=0,B=*,C=1)->TRUE
```

要求：**每个 MC/DC Pair 至少映射到一个测试用例**，并在用例注释的 `MC/DC:` 行标明它覆盖的是哪个条件的哪一对。

### 边界值（针对 `>=`、`<`、`>`、`<=` 比较）

对形如 `HwUnit >= COUNT`（COUNT=2）的判定，至少设计三点：

| 取值 | 相对边界 | 期望 |
|------|----------|------|
| COUNT-1（=1） | 边界下 | 判定 FALSE（通过） |
| COUNT（=2） | 等于边界 | 判定 TRUE（拦截） |
| COUNT+1（=3） | 边界上 | 判定 TRUE（拦截） |

### 复合判定（短路逻辑）逐条件独立覆盖

对 `(A || B)`：分别设计 **A=TRUE,B=FALSE** 与 **A=FALSE,B=TRUE** 两个用例，使每个子条件独立决定结果。
对 `(A && B && C)`：为每个子条件各设计一个使其单独为 FALSE 的用例 + 一个全 TRUE 用例。

### 覆盖 FALSE 分支时必须保证判定"可达"

若目标 `if` 位于函数内部（前面有参数校验 / 锁校验等前置判定），需构造"前置全部通过 + 目标条件取 FALSE"的独立用例。否则前置失败会短路，目标判定根本不执行，工具显示未覆盖。

### `TT_ParaCheck` / 短路 AND 模式的覆盖要点

常见实现：`(*RetVal == TT_RET_SUCCESS) && Expr`。若前序 `TT_ParaCheck` 已把 `RetVal` 置为错误码，后续的 `Expr` 不会被求值。要覆盖 `Expr` 的 T/F，需**先保证 `RetVal` 在该行时仍为 `TT_RET_SUCCESS`**——即前面所有校验都通过。

### 复合 OR 判定中右侧条件的独立覆盖（强制）

对如下常见判定：

```
(HwUnit >= UART_CONTROLLER_COUNT) || (Uart_Hw_PriGetChannel(HwUnit,&Channel) != TT_RET_SUCCESS)
```

必须至少设计两类用例，分别证明两个子条件都能独立致真：

- 左侧致真：`HwUnit` 越界（如 `HwUnit = UART_CONTROLLER_COUNT`）
- 右侧致真：`HwUnit` 合法（如 `UART_0`），但映射缺失使 `Uart_Hw_PriGetChannel(...) != TT_RET_SUCCESS` 为 TRUE（例如 `Uart_Hw_PubMap[0]` 设为不匹配值）

若只做越界场景，右侧条件通常因短路而未被执行，不满足 MC/DC 独立影响要求。

### 判定分支完整覆盖清单（编写时逐项核对）

- 每个 `if/else` 的 TRUE 与 FALSE 分支均有用例命中。
- 每个参数校验（NULL、越界、长度=0、非法值）各有独立用例。
- 成功路径 + 每个失败返回点（含桩返回失败）各有用例。
- DMA / IRQ 等模式分支：每种 `WorkMode` 以及 `dma_xx != NULL` 的 TRUE/FALSE 都要覆盖。
- 回调 `cb_event != NULL` 的 TRUE/FALSE 分支均覆盖。
- 用 `STUB_VAL_USER_CODE` 固定内部事件值（如 `*Event`）以覆盖"实际执行难以构造"的分支。

### 编译开关（`#ifdef` / `STD_ON/STD_OFF`）处理（强制）

生成前先确认 `PARAM` 文件或 `*_Cfg.h` 中的编译开关状态（如 `TT_UART_PERF`、`TT_FTM_PERF`）。被宏屏蔽的代码块在当前编译配置下不存在，无需覆盖，但必须在用例注释中说明：

```
-- 注意：因 TT_XX_PERF=STD_ON，目标分支在当前编译配置下不存在，跳过覆盖。
```

**禁止**为不存在的代码块生成用例，否则 VectorCAST 将报"函数/行未找到"错误。

### 用例注释模板

```
-- Test Case: <Name>
-- Description: <一句话场景描述>
-- MC/DC: <被覆盖的判定及其真值，例如 Decision (Length == 0) = TRUE>
```

## 8. AUTOSAR 驱动 API 测试矩阵

对每个对外 API（如 `Icc_Write` / `Icc_Read` 等），按下表逐项核对是否需要用例（不适用的项跳过即可，但要确认"不适用"而非"遗漏"）：

| 场景 | 说明 |
|------|------|
| Uninit | 模块未初始化前调用 → 应拒绝 |
| Init/正常 | 已初始化的正常前置 |
| Null Ptr | 入参指针为空 |
| Invalid Channel | 通道号越界（边界值见 §7） |
| Invalid Length | 长度=0 或超限 |
| Invalid Mode | 非法 WorkMode |
| Busy | tx_isbusy/rx_isbusy 冲突 |
| Timeout | 超时路径 |
| Hardware/Stub Failure | 底层/桩返回失败 |
| Success | 完整成功路径 |

### 状态覆盖（精简）

驱动含状态变量时，覆盖关键状态转换：`UNINIT→INIT`、`INIT→BUSY`、`BUSY→IDLE`、`BUSY→ERROR`。每个转换至少一个用例。

## 9. 编写/审查自检清单

- [ ] 每个用例至少 1 条预期输出（`TEST.EXPECTED` 或 `TEST.EXPECTED_USER_CODE`）
- [ ] 每个 `if` 的 TRUE/FALSE 分支均覆盖
- [ ] 每个 `switch` 的所有 case 覆盖
- [ ] 每个 `return` 点覆盖
- [ ] 每个边界值覆盖（COUNT-1 / COUNT / COUNT+1）
- [ ] 每个 MC/DC Pair 都映射到了用例
- [ ] 每个 DET / DEM 路径覆盖
- [ ] 每个被打桩函数的成功与失败返回均覆盖
- [ ] 每种 WorkMode（DMA/IRQ…）及 `dma_xx != NULL` 的 TRUE/FALSE 覆盖
- [ ] 回调 `cb_event != NULL` 的 TRUE/FALSE 覆盖
- [ ] 关键状态转换覆盖
- [ ] 指针数组成员用 `[i][0].FIELD` 而非 `->`
- [ ] 解引用前都先 `<<malloc>>`
- [ ] 所有可能访问寄存器的路径都已分配对应模块基址指针（如 `<Module>_Hw_PubBasePtr[index]:<<malloc 1>>`，Uart/Ftm/Smu 等各用其自身变量名）
- [ ] 宏组合返回值用 `EXPECTED_USER_CODE`
- [ ] 返回值是封装后的语义码而非底层原始码
- [ ] 用例编号/名称唯一
- [ ] 难以"实际执行"构造的分支改用桩 + `STUB_VAL_USER_CODE`
- [ ] 目标判定可达（前置校验均通过），而非被短路跳过
- [ ] 对 `(A || B)` 复合判定已分别覆盖 A 致真与 B 致真（避免右侧被短路）
- [ ] 新用例确实新增了某个 Decision/Condition 的 T 或 F，不与已有用例冗余
- [ ] `STUB_VAL_USER_CODE` 控制返回值时用 `.return` 后缀 + `<<path>> = value;`，禁止 `<< return x; >>`
- [ ] 所有用户代码块（`VALUE_USER_CODE` / `STUB_VAL_USER_CODE`）无独立 `<<` `>>` 包裹行
- [ ] 打桩 `Base_MemSet` 等返回字节数的函数时，用等价表达式计算 sizeof 而非引用形参名

## 10. 失败案例库（AI 易犯错误，务必规避）

> 这些是实际修正过的真实错误。生成前对照本表自查，价值高于再多读规则。

| # | AI 的错误写法 | 正确写法 | 原因 |
|---|---------------|----------|------|
| 1 | `ICC_Hw_PubBasePtr[0].FLG` | `ICC_Hw_PubBasePtr[0][0].FLG` | 它是**指针数组**，需双层下标 |
| 2 | `Ptr->Field` | `Ptr[0].Field` | VectorCAST 不支持 `->`，用下标解引用 |
| 3 | 对成员赋值但未先分配 | 先 `Array[i]:<<malloc 1>>` 再赋值 | 否则解引用空指针 |
| 4 | `TEST.EXPECTED:...return:TT_SET_STATUS_TYPE(...)` | 用 `EXPECTED_USER_CODE` 块 | 宏组合不能作为字面常量直接断言 |
| 5 | 预设全局 `tx_num` 来控制内部 `while` | 打桩 Pri 函数 + `STUB_VAL_USER_CODE` 设 `*Event` | 函数入口会用入参覆盖 `tx_num`，预设无效 |
| 6 | 期望写底层原始错误码（0x10/0x64） | 写经 `PriRetCheck` 封装后的语义码 | 上层会重新封装错误码 |
| 7 | 只生成 Happy Path | 按 §8 测试矩阵补 Uninit/Null/Busy/失败等 | 漏覆盖失败与异常路径 |
| 8 | 用 `VALUE_USER_CODE` 强制写 `const` 全局 | 直接 `TEST.VALUE:<Unit>.<<GLOBAL>>.Field:val` | VectorCAST 可直接写 `const` 全局，无需强转 |
| 9 | 新增用例但目标判定不可达 | 保证前置校验全通过再覆盖目标 F 分支 | 前置失败短路，目标 `if` 未执行 |
| 10 | 叠加冗余用例（编号增长覆盖率不变） | 改造已有用例或确认新增的 MC/DC Pair | 重复同一 Decision 同一真值无意义 |
| 11 | 用例会写寄存器但未给 `Uart_Hw_PubBasePtr` 分配内存 | 先 `TEST.VALUE:<Unit>.<<GLOBAL>>.Uart_Hw_PubBasePtr[0]:<<malloc 1>>` | 运行时空指针解引用，导致跑飞 |
| 12 | 只测 `(HwUnit>=COUNT)` 越界，不测 `PriGetChannel!=SUCCESS` | 增加 `HwUnit` 合法但映射缺失的用例 | OR 右侧被短路，无法覆盖子条件独立致真 |
| 13 | `STUB_VAL_USER_CODE` 中用 `<< ... return x; >>` 代码块 | 用 `.return` 后缀 + `<<path>> = x;` 赋值 | 桩环境中形参名不可用；外层 `<<` `>>` 会导致解析错误 |
| 14 | `STUB_VAL_USER_CODE:Base_MemSet` 中 `return n_bytes;` | `STUB_VAL_USER_CODE:Base_MemSet.return` + `<<uut_prototype_stubs.Base_MemSet.return>> = expr;` | 桩上下文无法访问被桩函数的形参名 `n_bytes`；需用等价表达式计算 |
| 15 | `VALUE_USER_CODE` / `STUB_VAL_USER_CODE` 用独立 `<<` `>>` 行包裹代码 | 代码直接写在标签行后面，不加外层 `<<` `>>` | VectorCAST 解析器将独立 `<<`/`>>` 视为语法错误 |
| 16 | 为被 `#ifdef`/编译开关屏蔽的代码块生成用例 | 先核对 `PARAM`/`*_Cfg.h` 编译开关，跳过不存在的分支并在注释中说明原因 | 该分支在当前配置下不存在，VectorCAST 报"函数/行未找到"错误 |
