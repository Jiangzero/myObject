# VectorCAST 环境文件准备规则

编写步骤、占位符模板及与通用规则分离的Dio示例见 `environment-templates.md` 和 `../templates/environment.{cfg,mfg,env}.template`。先核对适用环境再填充，不直接导入模板。

## 范围与证据

- 本参考覆盖 ENV/MFG/CFG 的创建、适配和静态检查；不授权自动运行 VCAST，也不要求生成 TST。
- 先读取当前真实样例及其工具版本，确认字段、区块和语法后再改。ENV、MFG、CFG 的职责分开记录，不能根据扩展名推断格式，也不能为了“同步”复制彼此内容。
- 用户明确路径、环境名、配置名和工具版本优先。路径至少区分本机生成根、内网部署配置根、驱动根和 VCAST 环境根；内网根在本机不存在时以映射后的本机输入/输出验证，不能猜测变量（如 `VCAST_SourceRoot`）的值。
- 写前保护原件的字节、hash、编码和换行。已有备份绝不覆盖；备份目录依工程策略决定，`originals/` 不是强制名称。不得创建空报告树。

## CFG、MFG 与依赖

- 按实际文件内容和用户工具反馈确认职责。2026-09-30 AM Dio 用户纠正：MFG 为 manage-config-options XML（coverage/search-directory），CFG 为 C_DEFINE_LIST/C_EDG_FLAGS 编译选项；早先相反的描述错误。没有同版本样例时，不添加新字段或伪造分支覆盖指标语法。
- `_VCAST_MINGW_C` 必须按固定编译约束保留，PERF 必须按真实生成宏保持关闭；CFG 不必复写 MFG 内容，但两者与生成配置必须语义一致。
- source/include 清单来自实际驱动、递归 include 依赖和匹配 EB 输出。生成 include 目录优先于插件模板，避免同名生成头被遮蔽。
- 缺少 Base、Mcu、Port 等生成依赖时，仅可使用匹配 Config 的 EB 实际生成补齐；不手写 output、不修改生成器、不借用旧 Polyspace output。工具链标准/专用头未由真实环境证明时标为待确认，不能因未在源码找到而断言工具链一定提供。

## ENV、寄存器、回调与桩门禁

- Unit 名、STUB_BY_FUNCTION 策略、prefix 区块和被测源文件必须与真实环境匹配，不能照搬 AS Unit 名。ALL_BY_PROTOTYPE 与 callback/用户定义的潜在重复须在真实环境核验；按函数 stub 策略仍须在内网选择真实被测函数。
- prefix 注入会早于 Unit 源自身 include 时，先按真实依赖链 include 定义所需标准类型与状态宏的头，再 include 依赖这些宏的生成/芯片配置头；不得手写常量掩盖 include 顺序错误。还须在真实环境确认 prefix 是否早于相关 static 基址指针初始化，否则 backing storage 重绑仅是计划，不能声称生效。
- 先区分真实存储、承接指针/指针数组、声明和定义；变量存在不证明其指向 RAM。若环境可安全访问，可重绑既有承接指针，不重复定义变量或寄存器。否则仅借已证实的 prefix 语法，提供正确类型、容量和对齐的 backing storage，并重绑所有可达基址；原 static 初始化时点是否采用新基址必须验证。
- backing storage 命名在每个 Unit 唯一；跨 Unit 是否共享按被测交互决定，不能固定要求隔离或共享。不得从某模块的实例数量外推其他模块。RAM 不自动模拟 SET/CLR/REV/W1C、自清零、FIFO 或硬件副作用，只断言已显式建模的效果。
- callback 仅在对应生成开关启用时，按真实符号和签名提供恰好一个定义；避免与自动 stub 重复。计数/最后参数可用于后续断言，但必须每例隔离。
- 外部变量（例如错误记录 ID）全环境只能有一个存储定义，先核对工具生成的存储，再决定是否人工提供；保留头文件 extern 声明。AM Dio 用户链接日志已证明 VectorCAST 生成 TTDet_ReportErrorId，必须删除 ENV 中重复定义，不能改成 static 制造另一对象。硬地址、汇编或硬件依赖（如 core-ID/SchM）必须隔离；有输出参数的桩不能只返回 0，简单输出优先直接 TEST.VALUE。未获环境证实的路径标为待人工验证。

## 静态检查与交接

- 链接失败先读取 Linker Diagnostic Messages：multiple definition 是重复存储，undefined reference 才是缺失符号。Unstubbed Entities Log 仅解释未打桩原因，标准库函数在该日志出现不等于链接错误，不据此盲目打桩或加库。
- 修改ENV/prefix后必须更新当前工具环境并重新生成构建；仅导入TST不会更新prefix。旧诊断持续出现时对照当前生成文件确认定义来源，不直接修改生成文件作为永久修复，也不擅自自动执行VCAST。

- 检查 ENV 区块成对、Unit/源文件存在性、prefix include 的宏/类型顺序、寄存器所有可达基址、callback 唯一性；检查编译选项文件的固定宏和生成宏中的 PERF 值；检查源配置文件的真实格式、路径映射、include 顺序、AS 遗留路径和递归源/生成头依赖。递归 include 检查须按当前有效编译条件区分活动与非活动路径，不能将条件关闭的头误判为工具链依赖。
- 记录源、生成头和配置版本/hash；核验既有 TST 在不属于范围时未变化。
- 环境文本静态成功、XML 可解析或 EB 成功均不等于 VCAST 可导入、编译、执行或覆盖达标。交付须分别标明已生成、静态已验证、未验证和风险，并请求内网回传工具版本、导入/编译日志、实际 stub 解析结果及覆盖报告。
## Authorized candidate mode
If the user explicitly requests source-first candidate TSTs, ENV/MFG/CFG evidence may be used to infer a minimal test-script organization. Clearly hand off unverified symbol paths, static access, backing-storage timing and stub ownership; retain the requirement for inner-network import/build/run validation.
