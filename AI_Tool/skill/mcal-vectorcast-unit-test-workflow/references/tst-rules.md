# TST 生成规则与基础参考修正

两份同目录原始参考保留原文。本文件明确修正其矛盾、模块专用例子和未经环境证明的假设，生成时优先应用以下规则。

## 范围、环境与证据

- **跨配置编号按模块场景复用**：同模块同一函数、输入和路径目标的用例，在不同 Config 中使用相同编号；宏导致的预期差异不自动产生新编号。用 `(Config, TEST.NAME)` 标识执行实例，单份 TST 内名称唯一。先建立场景映射，再分配配置特有场景编号；不能按配置接续编号，也不能仅按排列位置配对。已存在编号优先保留，允许配置间编号缺口。编译裁剪掉整个机制与运行时关闭该机制需按目标区分，不能仅因无回调就认定同场景。

- 以用户指定函数/Unit/模块为范围，不因原 prompt 的“全函数”要求自动扩大单函数任务。
- 原 prompt 的 `read_file` / `grep_search` 表示读取/检索动作，使用当前实际可用工具。
- 读取用户提供的真实 ENVIRO.env、编译参数、生成头文件和导出样例；不硬性假设它们与 TST 同目录，不杜撰 Unit、环境名、`uut_prototype_stubs` 路径或脚本特性。
- 原文固定头块仅为样例，按当前工具/环境导出内容选择，不无条件复制全部 `TEST.SCRIPT_FEATURE`。
- TT_RET 数值、寄存器掩码、DMA 事件、计数宏以及 Base_MemSet 返回语义均从当前源码确认，原始表格不作为跨模块常量真值。
- PERF 常关、`_VCAST_MINGW_C` 已定义；原文 PERF ON 场景仅是历史示例，不建立此类配置。
- ENV/MFG/CFG 不是 TST 的替代证据；TST 不在本次授权范围时保持既有 `.tst` 字节不变，不虚构脚本 features、桩路径或用例头块。环境文件适配规则见 `environment-files.md`。

## 输入、指针与断言

- `TEST.VALUE` 使用环境能解析的字面值/枚举/已确认符号，不写 `COUNT-1` 等计算表达式；依据当前配置算出数值并注释来源。
- TST 数据路径里的指针数组成员用 `[i][0].FIELD`，不写 `->`；不要把该规则误用于用户代码中的所有合法 C 表达式。
- 解引用对象先建立有效存储。使用分配型指针时先 `<<malloc N>>` 再赋成员；已有有效对象/别名按环境支持方式处理，不强迫为真实数组或故意 NULL 路径分配。
- 核实 `malloc N` 对该指针类型的元素数量与所需容量，原文“10 字节”示例不可推广到非字节指针。
- 访问寄存器的路径必须有对应模块、实例索引、结构类型及足够容量的模拟存储，不硬编码 Uart 或索引 0。普通内存不模拟硬件的 W1C、自清零、FIFO/定时行为；必要时使用环境支持的桩并说明限制。
- `const` 字段直接 `TEST.VALUE` 赋值须有该环境能力/样例支持，不强制转型写只读内存；不确定则询问并列为待验证。
- 每例至少一个验证目标行为的 `TEST.EXPECTED` 或 `TEST.EXPECTED_USER_CODE`；void 验证副作用，不写 return 断言。不能用恒真或无关断言凑数。
- 复杂宏组合比较采用环境支持的 EXPECTED_USER_CODE；追踪函数真实返回链，原始透传则断言原始值，实际包装才断言包装值。

## 桩与用户代码冲突修正

- **只设置有效且必要的参数，不是“简单语法优先”**：赋值必须服务于场景前置条件、目标路径、隔离复位或有效预期。不添加未使用的捕获变量、无观察价值的计数器或重复赋值；不能为减少代码而丢失必要状态、输出或错误路径。原始 `vectorcast-tst.instructions.md` 第 93–99 行支持直接设置全局、桩返回和桩输出参数；直接赋值足以表达目标时无需额外用户代码，需要计算、调用序列或副作用时使用环境支持的用户代码。选择依据是语义需要与环境有效性，不是语法长短。
- **函数所属 Unit 不等于调用者 Unit**：真实被测源单元内的函数使用其所属 Unit；ALL_BY_PROTOTYPE 生成的外部桩使用实际环境的 `uut_prototype_stubs` 路径。不能因为 Dio 调用某函数，就构造 `Dio.<外部函数>`。源文件可推导函数归属，最终生成桩的命名以当前环境证据为准。
- 模块专属语法与运行反馈见 `dio-case-study.md`；不得将案例 Unit、宏值或路径推广为其他模块固定规则。
- 不复制样例中的 `TEST.END_IMPORT_FAILURES:` 等导入诊断残留。语法模板依据、当前环境符号证据、源码行为预期和实际运行结果分别记录，不能相互替代。

- 桩路径必须来自当前环境。硬件/DMA/DET/DEM/回调/底层依赖可按隔离目标打桩，不把当前被测函数自身打桩，也不把被替换实现计入已测试覆盖。
- **所有可达错误码必须独立返回、独立验证**：按函数与配置列出错误码、触发条件、返回/记录位置和对应用例。底层桩的每个适用错误返回分别注入并核验上层实际透传或转换结果，不得以一个代表性失败码替代全部错误码；不同检查即便产生相同错误码，也须分别保证目标路径可达。成功场景另行保留。宏裁剪或源码不可达项给出证据，不制造不存在的返回；void 接口验证真实错误记录或其他契约输出，不虚构返回值。参数精简、去重或跨配置编号复用均不得削弱此约束。
- VALUE_USER_CODE、STUB_VAL_USER_CODE、EXPECTED_USER_CODE 使用对应起止标签；不加独立 `<<`、`>>` 外层包裹行，保留合法的 `<<符号路径>>` 引用。
- 桩返回值用户代码遵循 `.return` 路径后缀和 `<<已确认的返回值路径>> = expr;`，不沿用原文 §4 中裸 `return` 的矛盾示例；例中的 `uut_prototype_stubs` 必须按实际环境确认。
- 桩出参/全局状态通过已确认的环境符号引用修改。原文 `*Event`、`TransferInfo` 等裸变量名仅为示意，不能假设在桩上下文可见。
- Base_MemSet 成功值从真实接口/调用实参确定，不机械套用结构指针差式；返回成功但没有写内存的桩可能改变测试含义，必要时模拟对应副作用。
- 动态桩计数器在每个用例开始时复位，不能每次桩调用都置零，否则循环可能永远走第一次响应。末次调用复位不足以保证异常退出后的隔离；选用环境支持且可于用例初始化的状态或调用序列机制。
- 每例独立设置桩返回、出参、计数器、关键全局状态和寄存器初始值；明确循环如何继续、如何退出、如何触发超时。缺少可控退出机制时标阻塞，不交付可能无限等待却声称可执行的用例。

## MC/DC 与覆盖修正

- 一个 MC/DC 独立影响对必须映射两端具体用例，修正原文“每个 Pair 至少映射一个用例”的表述。
- 明确采用的 MC/DC 定义及环境度量方式；未确认时给出判定、条件出现位置、具体赋值及短路未求值标记，不声称工具必然认定达标。
- 不照抄含 `*` 的真值对来证明独立影响；说明不求值/无关条件含义，证明满足所采用的 unique-cause 或 masking 判据。重复条件和逻辑耦合单独分析。
- 对 `A || B` 除 A 独立致真、B 独立致真外，还需要判定为假的基线才能组成独立影响对；对 AND 同样记录全真基线及各条件的独立翻转。
- 全部用例先保证前置判定通过、目标条件实际可达；TT_ParaCheck 链特别核实前序错误不会阻断目标表达式。
- 边界取值先核类型范围，COUNT±1 不可表示时说明限制，不制造溢出值冒充边界。switch 的 default、循环零/一/多次和退出/超时按实际可达性规划。
- 被编译裁掉、schema 禁止或固定约束排除的路径记录为不在当前配置范围，不计入覆盖，也不把预处理组合覆盖与运行时 MC/DC 混为一谈。
- 静态分析只能说明设计映射与待验证项。原始参考的“已覆盖”在无人工执行证据时统一表述为“已设计，待人工验证”。

## 维护与来源

## 生成与验证关卡

- 批量生成前，建立函数→所属Unit→真实实现/原型桩→TST路径的映射；分别核对输入、输出、全局、返回值、指针和宏断言的代表用例，再展开其他场景。已有证据直接复用，不强制反复索取样例；授权候选模式下仅将未知项列待验证。
- 错误预期按源码使用对应错误宏；需要宏表达式时采用原始参考117–124行的 EXPECTED_USER_CODE 比较方法。区分十进制与十六进制，初始化哨兵不是错误码。实测与源码不符时调查版本/配置/桩，不把实测值直接改成预期。
- NULL 是有效测试输入和预期。对可达指针判定构造非NULL与NULL场景，含允许直接测试的私有函数。NULL预期只验证指针状态，另有错误记录契约时必须同时验证，不能据此宣称全部副作用正确。
- 无输出参数时先检查真实可观察副作用。DET关闭且没有其他有效观察点时，可以每例将错误记录初始化0并断言仍为0；注明仅验证错误记录未改变，不证明被桩替代的底层动作。这不是省略可达错误码独立测试的理由。
- 静态核对所有用例名称、块配对、符号归属、必要状态初始化、预期存在性、错误码与用例映射及重复定义；改名/语法修正另检查输入和预期语义保持。仅执行了部分检查就逐项报告，不笼统声称脚本通过验证。
- 证据分为原始语法资料、当前环境符号证据、源码行为推导、实际运行反馈。推导不能升级成通用规则；用户反馈运行正常也不自动证明全覆盖或所有配置全部PASS。

原始参考来源：创建本 skill 时从 `D:/MBS_Test_Project/am/Unit_Test/Vcast_Config_Project/` 同名文件复制，日期 2026-09-29。原文件保持不变。后续更新参考时先比较差异，保留已确认修正；不要用未经核验的新示例覆盖本文件的环境证据要求。
## Authorized source-first candidates
With explicit user authorization, generate a minimal candidate TST from real identifiers and reference grammar even without an exported sample. Do not invent nonexistent functions/branches; annotate inferred paths/features/stubs as pending inner-network validation. This exception permits candidate generation only, not a claim that the script parses, imports, compiles, executes, or covers.

## 经验教训：Pwm 系列 TST 实际交付沉淀（2026-10 补充）

以下为处理 Pwm Drv/Hw TST 时实际出现的遗漏与修正，生成或维护 TST 时对照检查；实例均来自当前 Pwm 驱动。

### 命名、枚举与宏名

- 用例名按内网要求采用 `SUT_<模块>_<连续编号>` 并从指定起始编号递增（实例：SUT_Pwm_241 起）。改名须全文件同步、保持唯一，不跨 Config 全局接续。
- TST 中的枚举/宏名必须逐字 grep 驱动头文件确认，不能凭语义猜测。实例：`PWM_FTM_DEAD_TIME_PAIR` 不存在，实际为 `PWM_FTM_DEAD_TIME_PAIR_CHANNEL`；Fault 控制模式为 `PWM_FTM_FAULT_DISABLED / PWM_FTM_FAULT_EVEN_CHANNELS_MANUAL_CLEAR / PWM_FTM_FAULT_ALL_CHANNELS_MANUAL_CLEAR / PWM_FTM_FAULT_ALL_CHANNELS_AUTO_CLEAR`；Fault 输出状态为 `PWM_FTM_SAFE_VALUE / PWM_FTM_TRI_STATE`；Fault 极性为 `PWM_FTM_FAULT_POLARITY_HIGH / PWM_FTM_FAULT_POLARITY_LOW`。错误宏名会让 VCAST 无法解析、用例不运行，并使目标分支在覆盖报告中表现为"未覆盖"。
- 错误码/枚举数值逐一确认：如 `PWM_E_INIT_FAILED=0x10`、`PWM_E_ALREADY_INITIALIZED=0x14`、`PWM_E_PARAM_POINTER=0x15`、`PWM_FULL_POWER=0`、`PWM_LOW_POWER=1`、`PWM_NODEFINE_POWER=2`。

### 内存与默认值

- 真实数组（BSS）默认 0，不等于期望哨兵。凡以特定值表示"未配置/跳过"的数组元素必须显式赋值。实例：`Pwm_Hw_PubFtmChannelMode[0][1..7]` 未设 `0xFF` 时默认 0，TOF 中断 handler 对全部 7 通道执行回调，回调计数断言失败。
- `<<malloc>>` 不保证零初始化。malloc 结构内的指针数组元素、未使用指针成员必须显式 `<<null>>`（实例：`FtmModuleConfig[1..6]`、`FtmChannelConfig[1..7]`），否则垃圾指针解引用崩溃。
- 写配置结构 `[i].FIELD` 前先确认成员是数组还是指针：指针成员（实例：`FtmDeadTimeChannelConfig`、`FtmFaultChannelConfig`）必须先行 `<<malloc N>>`；未分配时取值不可控（实测为 0）或崩溃。
- `FtmDeadTimeConfig->FtmDeadTimeChannelConfig[i].PwmCrc` 等元素级 CRC 字段在遍历循环内被访问，分配数组后逐元素设置（含 PwmCrc）避免垃圾值影响 CRC 匹配。

### 断言与覆盖

- 寄存器最终值断言必须追踪目标函数内对该寄存器的全部写入点（含其调用的私有/内联函数），不能只按目标子函数计算。实例：FtmInit 中 COMB 寄存器还包含 `PriFtmSetSync` 的 `SetChPairSyncEn`（bit5），期望 `0x23` 而非仅 Star 函数写出的 `0x3`。
- 枚举值为 0 时对应位不置位，按真实枚举数值计算寄存器期望。实例：`FtmFaultOutputState=PWM_FTM_SAFE_VALUE(0)` 不置 FAULT_CTRL 的 OUT_STAT(bit15)，期望 `0x311` 而非 `0x8311`。
- 子函数内部 `for(i < ChannelNumber)` 遍历指针数组：要覆盖循环体必须 `ChannelNumber>=1` 且先 malloc 数组并逐元素赋值；计数置 0 只能覆盖"跳过循环"分支（作为另一用例保留）。
- NULL 参数用例可行性取决于后续调用是否判 NULL：CRC 主函数以 `if(*RetVal == E_OK)` 保护，使 `ConfigPtr=NULL`（前序检查已置 E_NOT_OK）安全不崩溃；但 `Pwm_PubConfig->FtmConfig` 作为实参在调用前解引用，`ConfigPtr=NULL` 且进入 FtmInit 成功路径必然崩溃——PERF=ON 无指针检查时该类用例不可测，不生成。
- 通道/模块配置中的指针成员（`ChannelConfig[i]`、`FtmModuleConfig[i]`、`FtmChannelConfig[i]`）在 CRC 遍历与运行时都会被解引用，malloc 后须把未用元素显式 `<<null>>`；若驱动主流程遍历 `PWM_FTM_MODULE_NUMBER`/`PWM_FTM_CHANNEL_NUMBER` 全部索引，任何垃圾指针都会崩溃。

### 用户代码与桩

- `TEST.USER_CODE` 中的桩函数与记录变量必须**非 static（全局）**并加唯一前缀：`<<function 桩名>>` 函数指针赋值与 `TEST.EXPECTED_USER_CODE` 读取跨编译单元，static 符号不可见会导致编译/链接失败。
- 函数指针成员（如 `Pwm_PubConfig->CallBack`/`FaultCallBack`）用 `TEST.VALUE:...:<<function 桩函数>>` 赋值；不触发回调的用例也要给指针赋空桩或保证目标分支不访问，防调用 NULL 崩溃。
- static `.c` 私有函数直测（实例：`Pwm_Hw_PriFtmInterruptHandler`、`Pwm_PriProcessPowerState`）需 VCAST 静态函数支持（`STATIC_HEADER_FUNCS_IN_UUTS` 已启用，.c static 直测能力待内网确认）；这类用例的 `TEST.UNIT` 用函数所属 Unit。

### 编译配置与文件

- 同一 Unit 的 PRECOMPILE/PERF 组合是不同编译视图，需独立 TST 文件/环境，不能混用：PRECOMPILE=STD_ON 用编译期配置符号（`Pwm_Config[0]`），STD_OFF 用函数参数（`ConfigPtr`）且无需 `Base_GetCoreId` 桩；PERF=STD_ON 裁剪 CRC/状态/指针检查（删除对应 CRC 桩与状态预置），PERF=STD_OFF 需 CRC 桩（`Base_AccumulateCRC`/`Base_FinalizeCRC`）与 DET 断言（`TTDet_ReportErrorId`/`TTDet_ReportRuntimeErrorId`）。
- 固定配置组合（如 `PWM_POWER_STATE_SUPPORTED=STD_ON` 才编译的函数）在用例头部/交付说明标注，TST 必须在该配置环境导入。
- 条件编译包裹一致性检查：同族辅助函数与调用点均被 `#if PERF==STD_OFF` 包裹、而定义/声明裸露时提示（实例：`Pwm_PriParamCheck` 定义未被包裹）。当前功能不受影响（调用点已裁剪），但风格不一致且 PERF=ON 残留无用 static inline；是否修正由用户决定。
- 子配置 CRC 函数内部对指针数组的遍历（实例：`Pwm_PriDeadTimeConfigCrcCalc`/`Pwm_PriFaultConfigCrcCalc` 的 `for(i<ChannelNumber)`）是独立覆盖项，与"指针非 NULL 触发函数调用"不同：前者还需 ChannelNumber≥1 + 数组 malloc。

## 经验教训：Icu 系列 TST 实际交付沉淀（2026-10 补充）

以下为处理 Icu Drv/私有/Hw 三层 TST 时实际出现的遗漏与修正，生成或维护 TST 时对照检查；实例均来自当前 Icu 驱动（Config1=PRECOMPILE=ON/PERF=OFF，Config2=PRECOMPILE=OFF/PERF=OFF，`_VCAST_MINGW_C` 已定义）。

### 枚举入参、宏与符号可解析性

- 枚举类型入参（实例：`Icu_SetMode.Mode`、`Icu_SetActivationCondition.Activation`）VCAST 只接受字面值或已注册枚举常量；自定义宏即使已在环境 C_DEFINE_LIST 定义也无法识别 → 改用字面值（Mode=2、Activation=4）。`Channel` 等 uint8 整型入参可解析自定义宏（如 `ICU_CHANNEL_CONFIG_NUMBER_LESS/EQUAL/MORE`），不受此限制。
- `<<function SchM_Enter_<模块>_<AREA>>` 引用 SchM 接口函数，而 SchM 不在被测环境编译、符号无法解析 → 改为指向已打桩的 Hw 接口函数并补对应 `TEST.STUB`（实例：`<<function Icu_Hw_FtmStopInputCapture>>`）。
- 该环境不识别 `TEST.USER_CODE`/`TEST.EXPECTED_USER_CODE`（用户实测反馈）→ 回调/桩行为验证改用"哨兵寄存器"：预设目标寄存器为已知哨兵（如 `CH_STAT_CTRL[6]:0xFF`），回调指向真实 Icu_Hw 接口，断言回调落点寄存器被修改后的值（如 `0xB0`）证明"被调用且实参落点正确"；不被调用时哨兵保持原值。环境支持 USER_CODE 时仍可按 Pwm 教训使用，两种方式以当前环境证据为准。
- 函数指针实参受控避免失配崩溃：回调签名 `void(uint8,uint16)` 指向 `void(uint8,uint8)` 接口时，实参 1/2 被错位当作模块/通道索引，须保证 Channel∈[0,6]、CaptureValue∈[0,7] 且 `Icu_Hw_PubFtmReg[0..6]` 全 malloc，否则越界/崩溃。

### 指针下标与配置访问

- **单指针成员只解一次**：`FtmConfig` 是 `Icu_Hw_FtmConfigType*` 单指针，访问其成员用 `FtmConfig[0].FtmModuleConfig[i]`；写成 `FtmConfig[0][0].xxx` 会把单指针当指针数组解两次，VCAST 无法解析、指针配置不生效 → CRC 深度分支（`FtmChConfigPtr != NULL_PTR`）与 `Icu_PriInitParaCrcCheck` 大量分支表现为"未覆盖"。指针数组成员（`ChannelConfig`/`FtmModuleConfig`/`FtmChannelConfig`）才用 `[i][0].FIELD`。
- 配置来源随编译变体切换：PRECOMPILE=STD_ON 用全局 `Icu_Config[core][0]`，STD_OFF 用函数参数 `ConfigPtr[0]`；同一 `FtmConfig` 成员路径前缀随之不同。

### 寄存器与桩

- `Ftm_Hw_Pri*`（SetPrescaleFactor/SetChannelEdgeOrLevel/GetStatusFlags/GetChannelValue/GetChIntStatus/GetDualEdgeCaptureMode/ClrChIntFlag 等）是 `A8T8X_FTM.h` 的 `LOCAL_INLINE`，内联进 Icu_Hw.c 直接写寄存器 → 无需外部桩，寄存器最终值可直接断言（`STAT_CTRL`/`CH_STAT_CTRL`/`CFG`/`MODE`/`COMB`/`IN_FLT`/`STAT`/`CH_VAL`）。仅 `Mcu_Clk_StartModules`/`Mcu_Clk_StopModules`/`Mcu_Misc_LocalResetModules` 为外部调用，经 `uut_prototype_stubs.<函数>.return` 打桩。
- 寄存器期望按内联函数体逐位推导：`CH_STAT_CTRL`（DMA bit0/RST_CNT bit1/ELSA-ELSB bit2-3/MSA bit4-5/INT_EN bit6/FLAG bit7）、`STAT_CTRL`（CLK_DIV bit0-2/CLK_SRC_SEL bit3/DECAP_CONT_NEW bit29/EXTEND bit30）、`COMB`（每对占 8bit：DOUBLE_CAPT_EN bit2、DOUBLE_CAPT bit3，偏移 pair*8）、`IN_FLT`（每通道 4bit）、`CFG` DEBUG_MODE bit6-7、`MODE` EN bit0。
- FtmInit PERF 回读（FtmInitCheck）"成功"依赖写入值恰等于回读目标：Prescale=0（DIV1）→ CLK_DIV/EXTEND=0，DebugMode=3 → CFG=0xC0；回读失败只能直测 `Icu_Hw_PriFtmInitCheck`（寄存器与配置失配），经 FtmInit 无法构造（写入与回读同源必一致）。

### 覆盖缺口（短路与分支）

- `A && B` 中 A 先 FALSE 会短路，B 的 FALSE 分支未真正评估，覆盖报告显示"未覆盖"：需 A TRUE + B FALSE 的专用用例。实例 `(FtmChfStatus != 0) && (NULL_PTR != Icu_Hw_PubFtmConfig)`：原用例 STAT=0 只覆盖 A-FALSE；补 STAT≠0 + config=NULL 才覆盖 B-FALSE。
- 双沿判断 `(TRUE == GetDualEdgeCaptureMode) && ((ch % 2) == 1)`：原用例仅奇数通道(全 TRUE)与 COMB=0(首条件 FALSE)；`(ch%2)==1` 的 FALSE（双沿 TRUE + 偶数通道→普通捕获路径）需 COMB 置位 + 偶数通道的用例。
- 多模块"后模块失败"无法用常量桩返回值构造（同一次调用返回固定值），break 分支以单模块失败用例覆盖并注明限制。
- 重复边界用例检查：新增 _LESS/_EQUAL/_MORE 边界组时与既有用例同值同路径会重复，需去重；删除用例后清理残留的孤儿头块（TEST.NEW/UNIT/SUB 无 NAME，表现为 NEW 计数 > NAME 计数）。

### 编译变体与编号（用户约定优先）

- 同 Unit 的 PRECOMPILE/PERF 组合是不同编译视图，需独立 TST 文件/环境：Config1=PRECOMPILE=ON（配置取全局 `Icu_Config`，参数检查 `NULL_PTR != ConfigPtr`，需 `Base_GetCoreId` 桩），Config2=PRECOMPILE=OFF（配置取参数 `ConfigPtr`，参数检查 `NULL_PTR == ConfigPtr`，无 `Base_GetCoreId` 桩）。
- **编号遵循用户明确约定**：Icu 用户要求跨文件、跨变体全局接续编号（Drv 主→私有→Hw→Config2，`SUT_Icu_001..184` 连续）。这与 Pwm 教训"不跨 Config 全局接续"冲突，以用户当前明确约定为准。
- 按 Pwm 先例 Hw 层原用函数命名（`Icu_Hw_FtmInit_001`），用户要求改为续接全局编号；生成前先确认该模块的编号约定，不默认套用另一模块。

## 经验教训：Gpt 系列 TST 实际交付沉淀（2026-10 补充）

以下为处理 Gpt Drv/私有/Hw 三层 TST 时用户明确提出的交付约定（Config1=PRECOMPILE=ON/PERF=OFF），适用于后续所有模块的 TST 生成与维护。

### 交付格式：TST 与宏说明文件无注释

- 用户明确要求删除注释：交付的 `.tst` 文件不包含任何 `--` 注释——无文件头说明块、无章节分隔注释、无每例 Description/MC/DC 标注；仅含一个 `TEST.SCRIPT_FEATURE` 头块 + 用例（与 Icu 主 TST 同款格式）。
- 场景描述、MC/DC 映射、设计依据、限制与待验证项写入交付摘要，不写入 TST 文件。
- 自定义宏说明文件同样只含可编程内容（纯 `#define` 行），无注释、无说明文字（实例：`Gpt_Boundary_Macro_Definitions.txt` 仅 6 个 #define）。

### 边界值必须使用自定义宏

- 用户约定：凡需要边界值的用例，取值用自定义宏，不用裸字面量。实例（宏值以用例中的配置为中心）：
  - 通道边界 `GPT_CHANNEL_CONFIG_NUMBER_LESS/EQUAL/MORE = 1U/2U/3U`（ChannelConfigNumber=2；私有直测覆盖 LESS/EQUAL/MORE，公开 API 越界用例复用 EQUAL）；
  - StartTimer 目标值 `GPT_STARTTIMER_VALUE_ZERO = 0U`、`GPT_CHANNEL_TICK_VALUE_MAX_EQUAL/MORE = 1000U/1001U`（ChannelTickValueMax=1000）。
- 宏命名 `<模块>_<判定对象>_LESS/EQUAL/MORE`（或 `_ZERO`），宏值与用例中的配置值强绑定，调整任一方必须同步修改。
- 宏不写入 TST 文件（TST 只引用宏名），定义放 VCAST 工程/环境 C_DEFINE_LIST 或编译参数。
- 仅整型入参适用（uint8/uint16 等）；枚举入参 VCAST 不识别自定义宏，仍按字面值（沿用 Icu 教训，如 PredefTimer 边界 0/3/4 用字面量）。
- 同路径去重仍适用：LESS 与 EQUAL 同路径时按去重规则不重复设用例，但宏定义保持完整。

### Hw 层补充约定

- ISR 入口包装函数直测：按用户要求对被包装的私有处理函数直接打桩（`TEST.STUB`），以桩入参断言验证包装函数传入的常量实参（实例：FTM1..6_IRQHandler 各一例，断言桩 `Gpt_Hw_PriFtmInterruptHandler.FtmModule` = 1..6）；桩入参断言语法待内网验证，不支持时改用不打桩 + 哨兵寄存器方式。
- ISR 函数名以实际编译链为准：Gpt 经 TT_Common.h→TT_StandardTypes.h `#define ISR(x) void x(void)` 展开为 `FTMx_IRQHandler`（无 Os_Entry_ 前缀）；未定义 `<模块>_FTMx_USED` 的入口不编译，不生成用例。

