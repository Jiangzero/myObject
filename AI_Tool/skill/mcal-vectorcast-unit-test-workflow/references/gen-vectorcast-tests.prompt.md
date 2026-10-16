---
mode: agent
description: "为指定的被测 C 函数生成符合 VectorCAST 规范的 .tst 单元测试用例（含 MC/DC 边界与分支覆盖、正确的 malloc/数组/STUB 写法）。"
---

# 生成 VectorCAST 单元测试用例

为用户指定的被测函数生成一组符合 VectorCAST `.tst` 规范的测试用例。
若以下信息未提供，先向用户确认：
- **目标函数名** 与所属 **Unit**（UUT）。
- 目标 **`.tst` 文件路径**（用例追加到何处）以及**起始用例编号**（保证唯一、连续）。

## 工作流程

0. **读取环境配置（强制）**：读取同目录 `ENVIRO.env` 确认 Unit 名称与环境名；读取相关头文件（`*_Cfg.h`、`P1_*.h`、`*_Types.h`）提取寄存器掩码、计数宏（如 `UART_CONTROLLER_COUNT`）和编译开关（`STD_ON/STD_OFF`），避免 Unit 名写错或宏值硬编码错误。
1. **读取源码并建立函数总清单（强制）**：用 `read_file` / `grep_search` 提取该 Unit 全部可测函数，必须包含公有函数与私有函数（如 `static`/`*_Pri*`）。
2. **全函数调用覆盖规划（强制）**：为函数总清单中的每个函数分配至少 1 个可执行用例，确保每个函数都被 `TEST.SUBPROGRAM` 直接调用；若环境限制无法直测私有函数，需记录原因并通过可达入口间接覆盖，同时给出缺口说明。
3. **读取现有 .tst**：若目标文件为空则先生成完整头块（`TEST.SCRIPT_FEATURE` 行 + 环境注释）再追加用例；若文件非空则**仅追加用例，禁止重复写头块**；检查已有用例命名/编号/函数分布，避免重复；复用已有 malloc/STUB 写法风格。
4. **Decision 清单与可达性分析**：逐函数列出所有判定、边界、返回点、短路路径，先确认目标判定可达，避免前置校验短路导致“看似设计但实际未执行”；对 `(A || B)` 必须分别覆盖 A 致真与 B 致真（禁止只覆盖左侧致真）。
5. **MC/DC 判定对分析**：对每个复合判定显式写出 Decision、Condition、MC/DC 独立影响对，保证每个 Pair 都可映射到具体用例。
6. **API 测试矩阵核对**：对外 API 按知识库 §8 逐项核对 Uninit/Null/Channel/Length/Mode/Busy/Timeout/失败/Success，禁止仅生成 Happy Path。
7. **经验规则闸门（赋值与断言）**：
   - `TEST.VALUE` 禁止写计算表达式（如 `MACRO+1`、`COUNT-1`）；使用工具可稳定解析的常量/枚举/字面值。
   - 复杂返回值或宏组合比较必须使用 `EXPECTED_USER_CODE`，不要直接写到 `TEST.EXPECTED:...return:`。
   - `void` 函数禁止 return 断言，必须断言可观测副作用。
   - 每个测试用例至少包含 1 条预期输出（`TEST.EXPECTED` 或 `TEST.EXPECTED_USER_CODE`）。
8. **经验规则闸门（数据与指针）**：
   - 指针数组成员统一使用 `Array[i][0].FIELD`，禁止 `->`。
   - 所有解引用对象先 `<<malloc>>` 再赋值。
   - 凡是执行路径可能访问寄存器的用例，必须先分配对应模块的基址指针，例如 `TEST.VALUE:<Unit>.<<GLOBAL>>.<Module>_Hw_PubBasePtr[0]:<<malloc 1>>`（Uart → `Uart_Hw_PubBasePtr`，Ftm → `Ftm_Hw_PubBasePtr`，Smu → `Smu_Hw_PubBasePtr`，以此类推）。
   - `const` 全局字段默认允许直接 `TEST.VALUE` 赋值，无需 `VALUE_USER_CODE` 强制转换。
9. **打桩策略与内部状态构造**：优先打桩硬件/DMA/DET/DEM/回调/私有底层函数；每个关键桩至少覆盖成功与失败返回。需要修改出参或内部状态时使用 `TEST.STUB` + `STUB_VAL_USER_CODE`。
   - **`STUB_VAL_USER_CODE` 控制返回值时**：路径必须加 `.return` 后缀，用 `<<uut_prototype_stubs.FuncName.return>> = expr;` 赋值；**禁止**用外层 `<< >>` 代码块 + `return` 语句（桩环境中形参名不可用）。
   - **`VALUE_USER_CODE` 赋值时**：用 `<<Unit.<<GLOBAL>>.path>> = expr;` 直接赋值，**不要**用独立的 `<<` `>>` 行包裹代码块。
   - 打桩 `Base_MemSet` 等返回字节数的函数时，成功路径用 `((uint8 *)(&TransferInfo[0]+1) - (uint8 *)(&TransferInfo[0]))` 等价计算 `sizeof`，失败路径赋 `0U`。
10. **生成用例并保证唯一编号**：按连续编号生成，保持命名唯一；每个用例补充 Description 与 MC/DC 标注。
11. **追加后静态自检（强制）**：检查 `TEST.NEW/TEST.END` 配对、`TEST.NAME` 唯一、每例至少 1 条预期、无非法表达式赋值、无重复等价用例。
   - 额外核对：所有寄存器访问路径均已分配 `Uart_Hw_PubBasePtr`；所有 `((A) || (B))` 判定已覆盖 B 独立致真场景（避免右侧被短路）。
   - **用户代码块格式核对**：确认所有 `STUB_VAL_USER_CODE` / `VALUE_USER_CODE` 块**无**独立 `<<` `>>` 包裹行；桩返回值用 `.return` 后缀 + `<<path>> = value;` 赋值。   - **桩隔离核对**：同一 `.tst` 文件内，每个用例的桩行为通过该用例自身的 `TEST.VALUE` 或 `STUB_VAL_USER_CODE` 独立设置，不依赖前一用例遗留的全局状态。12. **覆盖结果核对与回补**：若有执行报告，按未覆盖 Decision/Condition/行做增量回补；无报告时给出“已覆盖项/待验证项/风险项”清单。

## 输出要求

- 仅输出/写入符合规范的 `.tst` 文本，不改动被测源码。
- 每个用例编号唯一且连续。
- 完成后输出三列汇总表：

| 用例编号 | 覆盖的 Decision/Condition | 状态（已覆盖/待验证/风险） |
|---------|--------------------------|--------------------------|
