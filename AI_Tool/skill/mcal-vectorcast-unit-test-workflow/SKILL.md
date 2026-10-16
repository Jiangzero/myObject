---
name: mcal-vectorcast-unit-test-workflow
description: MCAL 驱动 VectorCAST/VCAST 单元测试准备工作流：按授权阶段规划宏/XLSX、生成 EB、准备 ENV/MFG/CFG、生成或维护 TST。固定定义 _VCAST_MINGW_C、PERF 常关；绝不自动运行 VCAST，导入、编译、执行与覆盖率由内网人工验证。
---

# MCAL VectorCAST 单元测试工作流

## 1. 范围与必读资料

- 按用户本次授权阶段执行宏规划、XLSX、EB、环境文件（ENV/MFG/CFG）或 TST；不得因进入任一阶段自动扩展至其他阶段，更不得自动运行 VCAST。指定函数只处理指定函数；指定 Unit/模块则建立该范围全部可测函数清单，含公有与私有函数。
- 可在本机创建、校验和实际生成 EB 工程。环境文件、TST 的静态自检不等于 VCAST 导入、编译、执行或覆盖率验证，后者由内网人工完成。
- 先读 `references/config-and-handoff.md`；处理 ENV/MFG/CFG 前读 `references/environment-files.md` 与当前真实样例；生成/修改 TST 前必须读 `references/tst-rules.md` 及两份原始参考 `references/gen-vectorcast-tests.prompt.md`、`references/vectorcast-tst.instructions.md`。
- 两份原始参考是基础经验资料，不是所有版本环境均通用的语法证明。当前用户约定、真实源码/环境证据及 `tst-rules.md` 的明确修正优先于原始参考的冲突示例。新证据与规则冲突时先询问。
- 借鉴 `mcal-polyspace-config-workflow` 的宏关系、最小配置集、EB 和 XLSX 方法；本 skill 已收录相关契约，不依赖加载整个 Polyspace 流程。

## 2. 输入与路径

开始时确定模块、源码/插件版本及路径、目标范围、配置根、覆盖目标和本次阶段。信息已确认不重复询问。

- 当前项目配置根：`D:/MBS_Test_Project/am/Unit_Test/Vcast_Config_Project`。
- 跨项目复用时优先用户指定路径，否则确认项目根后采用 `<ProjectRoot>/Unit_Test/Vcast_Config_Project`；不硬编码到当前 am 工程。
- EB：`<ConfigRoot>/<Module>/Configs/<Module>_EB_ConfigN`，编号只用 Config1..N。
- 矩阵：`<ConfigRoot>/<Module>/<Module>_Macro_Config_Matrix.xlsx`。
- TST：由用户指定目标路径、对应 Config、Unit/环境、目标函数范围和起始编号；已有文件先读，编号冲突先解决。不要猜 TST 与环境的目录关系。
- 准备真实 `ENVIRO.env`、MFG、CFG、`PARAM`（若该环境使用）、对应生成头/源文件、相关类型与寄存器头文件、现有 TST 或该版本环境导出的样例。先核验实际内容、格式和职责，不能按扩展名猜字段；缺少确认 Unit、编译宏、符号/桩路径、脚本特性所必需的证据时先索取。环境名、子目录、驱动根、本机生成根、内网部署配置根与 VCAST 环境根分别确认；用户明确路径优先，不能臆测 `VCAST_SourceRoot` 的值。

## 3. 固定编译约束

1. 所有 VCAST 配置都定义 `_VCAST_MINGW_C`。检查源码使用的是 `#ifdef` 还是数值比较；具体定义值从真实环境确认，不能把“已定义”擅自等同于数值 1。
2. 所有相关模块的 PERF 性能宏始终关闭，按各宏真实语义使用 `STD_OFF`/0 等；若以存在性判断，确认有效关闭方式，不能用定义为 0 冒充未定义。
3. 两者作为固定约束记录在矩阵，不为其 ON/OFF 扩展 Config。固定约束排除的分支标记为本测试范围不覆盖，不能计作已覆盖。
4. EB 生成宏与 VCAST 编译器预定义宏分开记录。`_VCAST_MINGW_C` 无需伪造成 EB 配置节点；人工导入时核对其实际存在，确认 PERF 没有被编译参数覆盖。
5. 其余宏依据当前源码、XDM/schema、生成器和资源约束规划；预处理有效值需考虑 include 顺序、重定义及宏别名。头文件推断尚未经内网编译核验时标明待确认。

## 4. 宏关系与最小配置集

- 区分编译宏、运行时配置、PB 实例和实际多变体；PB 支持不等于多配置，同一编译向量的多个 PB 实例不增加宏覆盖。
- 一级/二级/三级按函数中条件嵌套或交互关系划分，不按目录或重要性。一个宏可在不同上下文处于不同级，无三级则留空。
- 父 OFF 时子分支覆盖为 N/A，不计作子 OFF 覆盖；仍记录子宏真实物理值。父 ON 时覆盖可达子 ON/OFF，三级逐级激活。
- 同级耦合按条件真值规划；schema 禁止、资源互斥或固定约束排除的状态说明原因，不强造全开配置。
- 使用最小有效配置集合，不固定配置数量，不机械枚举 `2^N`。每个 Config 说明不可替代目标、主开关向量、PB 运行时差异、合并理由及剩余缺口。
- PreCompile/PostBuild、多核、实例、DMA/IRQ/POLL 能力由当前模块证明，不照搬别的模块限制。
- 先生成 XLSX 计划矩阵请用户确认；已明确授权具体矩阵可继续。EB 生成后回填真实值，计划与实物不符时调整并说明。

## 5. EB 与矩阵交付

按 `references/config-and-handoff.md` 的布局和 EB 规则实施。复制同平台有效模板，同步项目名称、模块版本、路径和引用；实际 verify/generate 后检查宏值及生成产物。记录实际命令、退出码、Errors/Warnings 和日志来源。

仅生成 XLSX 配置矩阵，不另交 Markdown 配置矩阵。该限制不禁止 TST 说明注释、交付摘要或人工结果反馈。

## 5.1 ENV/MFG/CFG 环境准备

- 编写时同时读取 `references/environment-templates.md`，使用 `templates/environment.{cfg,mfg,env}.template` 的已知结构，按真实模块、工具链和部署路径填充；模板不是已验证的目标工程，禁止保留占位符或照搬Dio专用硬件内容。

- 仅在本阶段获授权时生成或适配环境文件；先保存基线并核验字节/hash、编码和换行，已有备份不得覆盖。`originals/` 是可选的本次交付策略，不是所有工程的固定目录。不要创建空报告树。
- 环境文件结构、字段和语法必须来自同工具版本/环境的真实样例；CFG、MFG、ENV 分工独立，不要求文本内容同步，只要求其编译和源配置语义一致。未经样例或版本证明不得添加字段、桩指令或脚本特性。
- 生成头/源与递归 include 依赖清单来自当前驱动和匹配 Config 的真实 EB 输出；生成头目录按 include 顺序优先于插件模板。缺失依赖只可由匹配 EB 项目实际生成，禁止借用旧 Polyspace output 或手写 output。
- 内网部署根本机不可见时，不报作本机缺文件；将其映射到已确认的本机驱动/生成输出后验证。工具链标准或专用头未证实时列待内网确认，不能因源码树未找到便推定其由工具链提供。
- 详情、寄存器与桩门禁、静态检查见 `references/environment-files.md`。

## 6. TST 生成流程

硬约束：只设置构造场景、隔离状态和验证行为所需的有效参数，不以“简单语法优先”为标准；所有可达错误码必须独立触发/返回并分别断言，不以单一失败场景替代。精简赋值不得删减错误码覆盖。具体生成关卡见 `references/tst-rules.md`，模块实例见 `references/dio-case-study.md`，案例不得替代当前源码证据。

1. 绑定当前 Config、环境、Unit、源代码版本和生成配置；读环境及源码，确认有效编译分支、符号路径、可测试/可打桩能力。
2. 建立本次范围的函数清单、Decision/Condition 清单、短路路径、边界、返回点、状态转换和寄存器访问路径。模块/Unit 范围中每个可测函数至少规划一个直接调用用例；私有函数不能直测则经可达入口间接覆盖并记录限制。
3. 对复合判定显式设计 MC/DC 独立影响对，每对映射两个具体用例；前置条件保证目标可达。不可达或环境受限项给出证据，不伪造覆盖。
4. 对适用 API 逐项核对 Uninit、Init、Null、Channel、Length、Mode、Busy、Timeout、底层失败和 Success；只对源码存在且配置可达的行为设预期。
5. 读取现有 TST：空文件按实际环境样例生成一个头块；非空文件默认只追加，不重复头块。需要修正已有错误/冗余用例时说明范围，未经授权不替换历史用例。
6. 按 `tst-rules.md` 编写输入、内存分配、隔离桩行为和有意义的预期；边界值用自定义宏测试（整型入参，宏以用例配置值为中心、TST 只引用不定义，枚举入参按字面值）；交付 TST 不含任何注释（无头块说明、无每例 Description/MC/DC 标注），设计信息写入交付摘要。单份 TST 内名称唯一；同模块相同场景跨 Config 复用编号，配置特有场景才分配新编号，允许各配置编号有缺口，禁止按配置全局接续编号。
7. 静态自检后交付 `.tst`，给出用例编号、设计覆盖项及“已生成/静态检查通过/待人工验证/风险或阻塞”的摘要。无执行报告不得使用“实际已覆盖”“测试通过”或声称达到某覆盖率。
8. 生成/维护 TST 前对照 `references/tst-rules.md` 末尾的“经验教训：Pwm 系列 TST 实际交付沉淀”“经验教训：Icu 系列 TST 实际交付沉淀”“经验教训：Gpt 系列 TST 实际交付沉淀”章节逐项自查：枚举/宏名逐字 grep 确认；真实数组 BSS 默认 0 与 `<<malloc>>` 不保证零初始化（未用指针元素必须显式 `<<null>>`）；配置结构指针成员（如 `FtmDeadTimeChannelConfig`/`FtmFaultChannelConfig`）先 `<<malloc>>` 再写元素；单指针成员（如 `FtmConfig`）只解一次下标 `[0].`，指针数组才用 `[i][0].`；寄存器最终值断言追踪目标函数内全部写入点（含私有/内联调用）；`TEST.USER_CODE` 桩函数与记录变量必须非 static；PRECOMPILE/PERF 配置变体需独立 TST 文件/环境；static `.c` 私有函数直测与 `<<function>>` 函数指针桩标注待内网验证；枚举入参不用自定义宏；不识别 USER_CODE 的环境用哨兵寄存器验证回调；交付 TST 与宏说明文件无注释、边界值整型入参用自定义宏（宏定义放环境，不写入 TST）。

## 7. 静态检查与人工闭环

- 检查头块唯一、`TEST.NEW/TEST.END` 配对、用户代码块起止配对、用例名/编号、Unit/函数路径和每例有效断言；交付 TST 及宏说明文件无注释行。
- 检查无被宏裁掉的函数/分支、无非法 `TEST.VALUE` 运算式、void 无 return 断言、寄存器解引用前有有效存储、用例桩与状态互相隔离。
- 检查 Decision/Condition 对应关系、短路右侧场景、MC/DC 对的两端用例、循环终止条件和每个新用例的测试价值。
- 文本结构检查或本地推理不能替代 VectorCAST 解析、编译或执行；静态检查未覆盖的语法/运行项列入人工待办。
- 按参考交接清单请人工回传导入/编译错误、失败用例及覆盖报告，绑定同一 Config、源版本和 TST 版本再分析；按根因增量修正，保留已有编号和可追溯性。
- 回传部分配置时只更新对应状态，其他配置保持待验证。实际达标依据人工报告与约定覆盖目标判断。

## 8. 文件保护与完成条件

不改被测驱动源码、插件生成器来迎合用例；不手写 EB output 伪造生成产物；不提交 Git。保留原始参考、已有工程和用户结果。临时文件使用当前环境批准的 Temp 根，长期归档路径由用户指定。

完成时按本次阶段列出真实文件路径、已生成项、静态已验证项、未验证项、风险和人工下一步；无执行证据不得写“全部测试通过”。此次仅创建 skill 时只交付 skill 及其参考，不顺带生成业务工程或用例。
### Candidate-generation override
When the user explicitly authorizes source-first candidate TST generation, missing exported examples or unconfirmed TST symbol/stub paths do not block generation. Infer only from real source, ENV and the minimal reference grammar; mark inferred paths, static visibility, script features and stub ownership as unverified. Candidate generation is distinct from a verified delivery: never claim import, compilation, execution or coverage. Reuse across modules only syntax/contract conventions already verified for the same toolchain; recheck each module's units, types, macros, registers and callbacks.