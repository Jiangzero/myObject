# ENV / MFG / CFG 编写与模板

## 证据及使用范围

模板位于 `../templates/environment.{cfg,mfg,env}.template`，从本次 AM Dio 真实文件提取结构，不可直接导入带占位符的模板。原始模板骨架与字段只适用于经核对兼容的环境，其他版本先比对真实样例，不能假设普适。

来源：`D:/MBS_Test_Project/am/Unit_Test/Vcast_Config_Project/Dio/VectorCAST/Dio_EB_Config{1,2}/DIO_EB_CONFIG{1,2}.{cfg,mfg,env}`。用户2026-09-30反馈运行无明显问题；没有完整构建/覆盖报告或明确产品版本。XML `version="7"` 是格式字段，不是 VectorCAST 产品版本。当前编译参数标识 MinGW/GCC 10.2 兼容模式，不代表其他编译器也适用。

## CFG：编译选项

填入 `COMPILER_DEFINES` 与 `EDG_FLAGS`，保持样例的两条字段。Dio 本次实际值：

```text
C_DEFINE_LIST: _GCC_C_ALIOTHXX_ USING_OS_AUTOSAROS _VCAST_MINGW_C
C_EDG_FLAGS: -w --gcc --gnu_version 100200 --mingw
```

`_VCAST_MINGW_C` 是本工作流固定要求；其他编译器/OS宏必须由目标源码和实际工具链确认。PERF按真实生成宏保持关闭，不随意追加会与生成头冲突的定义。不要将 XML 内容写入此文件。

## MFG：覆盖选项及搜索目录

将覆盖选项填入 `COVERAGE_TYPE`；本次真实值为 `STATEMENT+MCDC`，不猜测额外覆盖字段。填入内网生成 include/src 路径，复制 `DEPENDENCY_DIRECTORY` 所在整行以表达每一个递归依赖目录，删除占位行。所有目录采用实际样例确认的 `TESTABLE_SOURCE_DIR` 类型，不创造其他类型。

AM Dio 的14个目录按顺序为：对应Config的output/include、output/src，随后为插件目录下 Base/header、Base/interface/include、Os/include、SafetyMtl/header、Base/include、Base/interface/memmap_inc、Dio/include、Dio/src、Platform/include、Port/include、Mcu/include、Base/interface/SchM/include。这里的Base/Dio等是说明简称，填写时必须使用完整、版本匹配的插件目录名。

其他模块从源码递归include及匹配EB输出重新求依赖，不能机械照抄14条目录。生成头目录优先；保持已验证的搜索顺序。路径是内网部署路径，不因本机不存在就判错；通过本机映射核验文件。XML特殊字符须转义。

## ENV：Unit及前缀

替换环境名；按真实源文件Unit逐行重复 `ENVIRO.STUB_BY_FUNCTION`。本次Dio有Dio、Dio_Cfg、Dio_Hw、Dio_Hw_Irq四个Unit。按需要逐个重复完整的 `UNIT_PREFIX_USER_CODE_FILE` 至对应结束标签区块；不是每个Unit都必须有前缀。没有前缀需求时，不填入任意C代码，按当前环境样例处理空区块。

`MAX_VARY_RANGE: 20`、`ALL_BY_PROTOTYPE`与两个空字段来自本次样例，应核对目标环境是否沿用。STUB_BY_FUNCTION不等于将当前被测函数替换为空实现；TST中正确选择真实被测函数和所需隔离桩。不要根据调用者Unit构造外部桩路径。

`PREFIX_C_CODE` 必须是实际需要的、可编译的C代码。类型/状态宏头先于依赖它的芯片头；区分指针与存储，先检查工具已有定义。禁止将TTDet_ReportErrorId人工定义作为通用模板。

### Dio硬件前缀实例（片段，不可原样作为完整模块模型）

```c
#include "TT_StandardTypes.h"
#include "Base_Chips_Cfg.h"
#include TT_GPIO_HEADER
TT_GPIO_Type Dio_Hw_VcastGpio[19];
#undef TT_GPIO0_BASE
#define TT_GPIO0_BASE (&Dio_Hw_VcastGpio[0])
```

实际Dio模型必须同样重映射全部GPIO0..18，且前缀要先于源文件static指针数组初始化生效；只复制上述GPIO0片段不安全。IRQ Unit使用独立命名的Dio_Hw_Irq_VcastGpio。不能把19端口、类型或宏名推广到其他模块。RAM不自动模拟SET/CLR/REV/W1C硬件副作用。

Config1启用通知，提供唯一真实回调及有用的通知观察变量；Config2关闭通知，不注入无效回调/计数器。回调签名、名称、启用条件均由对应生成配置确认，不以空函数模板替代分析。

## 填写流程与交付检查

1. 保存原件；绑定模块/配置/环境名，分别记录本机根和内网部署根。
2. 根据目标工具链填写CFG；核对有效编译分支、固定宏及PERF关闭。
3. 从真实EB产物与源码提取依赖，填写MFG并检查目录顺序、版本和路径映射。缺失产物用匹配EB工程生成，不手写output。
4. 建立Unit清单及隔离策略，填写ENV；按真实硬件访问需求添加存储和重映射，不无条件注入全局定义。
5. 检查无剩余 `{{...}}`；XML可解析；ENV外层/单元区块配对；每个Unit存在；回调及存储定义唯一；同一Config的三个文件语义一致。需要单独检查真实符号，结构正确不等于可构建。
6. 内网人工导入、重新生成环境并编译运行。修改ENV/CFG/MFG后仅导入TST不足以更新环境。不得自动运行VectorCAST；分别报告静态检查、用户运行反馈和覆盖报告证据。

更完整的存储、链接诊断及候选交付边界见 `environment-files.md`；测试语法见 `tst-rules.md`。
