---
name: mcal-perf-test
description: 为 MCAL/AUTOSAR 驱动生成基于 SysTick 计数的性能测试代码，测量各 API 的 runtime 周期数并做 TTDet 错误检查与 pass/fail 汇总。当用户要求为某个驱动编写性能测试、运行时间测量或 runtime 测试用例时使用。不用于普通单元测试或功能验证。
---

# MCAL 驱动性能测试代码生成

根据 MCAL 驱动（AUTOSAR 模块）生成性能测试代码：对驱动每个 API 用 SysTick 计数器测量执行周期数（runtime），通过 TTDet 错误 ID 检查与 `FailFlag` 做 pass/fail 汇总。框架与模板来自 ThinkTech MCAL 性能测试工程（`test.h` / `test.c` / `main.c` 三件套）。

## 输入

- 目标驱动模块名（如 `Adc`、`Gtm`、`Port`、`Uart`、`Mcu`）
- 模块 AUTOSAR 头文件 `<Module>.h`（API 原型）与配置头 `<Module>_Cfg.h`（配置句柄，如 `Adc0Group_0`）
- 可选：EB 配置文件（.xdm）确认句柄与枚举名

## 输出（三个文件）

| 文件 | 来源 | 说明 |
|------|------|------|
| `test.h` | 复制 `assets/test.h` | 测试框架宏，含 `DEF_CASE` / `dbg_check_*` / `SIT_<Module>_TEST` 映射 |
| `<Module>_PerfTest.c` | 按模板生成 | 各 API 的计时用例体（参考 `references/adc_perf_example.c`） |
| `main.c` | 参考 `assets/main.c.template` | 初始化序列 + 调用 `SIT_<Module>_TEST()` |

## 步骤

1. **准备框架头**：复制 `assets/test.h`，把 `#define SIT_ADC_TEST  SIT_Adc_410` 替换为目标模块，如
   `#define SIT_GTM_TEST   SIT_Gtm_410`。用例编号一般取 `410`，如工程已有编号则顺延。

2. **生成用例体 `<Module>_PerfTest.c`**：
   - include `TT_Common.h`、`test.h`、`<Module>.h`、`Base_Drv.h`。
   - 声明 `time1`/`time2` 全局变量，以及 `SYSTICK_GET_COUNTER`、`SYSTICK_DELTA_INNER`、`DBG_PRINTF` 宏（从 `references/adc_perf_example.c` 复制）。
   - 为每个被测试的 group/channel 配置句柄声明 buffer（如 `uint16 adc0group0_buff[50]`）与结果指针（如 `Adc_ValueGroupType *PtrToSamplePtr_adc0group0`）。
   - 用 `DEF_CASE(SIT_<Module>_<编号>) { ... }` 定义用例，体内对**每个 API** 按固定模式：
     a. 清零 3 个 TTDet 错误 ID：`TTDet_ReportRuntimeErrorId = 0; TTDet_ReportErrorId = 0; TTDet_ReportTransientFaultId = 0;`
     b. `SYSTICK_GET_COUNTER(time1);`
     c. 调用 API
     d. `SYSTICK_DELTA_INNER(time1, time2);`
     e. `DBG_PRINTF(API名);`
   - 需要返回值的 API 用 `dbg_check_compare(Func, 实际值, 期望值)` 校验；`GetVersionInfo` 类校验 vendorID/moduleID/版本号字段（参考示例中 Adc 的 vendorID=214、moduleID=123、1.0.0 检查）。

3. **异步 / 顺序敏感 API 的处理**：
   - `StartGroupConversion` 后加 `Base_Drv_Msdelay(100)` 再测 `StopGroupConversion`，避免测量到未完成状态。
   - 需要 buffer 的 API（如 `Adc_SetupResultBuffer`）先绑定 group 的 buffer 再测后续读操作。
   - 使能/去使能成对的 API（如 `EnableHardwareTrigger`/`DisableHardwareTrigger`、`EnableGroupNotification`/`DisableGroupNotification`）按成对顺序测量。

4. **生成/修改 `main.c`**：按 `assets/main.c.template` 的初始化序列
   `Base_Drv_Init → Mcu_Init → Mcu_InitClock（若 MCU_INIT_CLOCK=ON）→ PLL 锁定等待 + Mcu_DistributePllClock（若 MCU_NO_PLL=OFF）→ Port_Drv_Init → Uart_Drv_Init → Platform_Drv_Init → 打印系统频率`，
   然后调用 `SIT_<Module>_TEST();`，最后 `while(1) {}`。
   - **必须 `#include "test.h"`**：main.c 需要包含 `test.h` 才能获得 `SIT_<Module>_TEST` 宏映射与函数声明；不要用手写 `extern void SIT_GTM_TEST(void);` 替代，否则下次会漏掉 include。
   - 注意：`test.h` 内的 `static uint32 FailFlag` 在 main.c 翻译单元未被使用，可能触发 `-Wunused-variable` 告警；无 `-Werror` 时仅为告警，可接受。

5. **交叉核对**：逐个核对 API 原型参数与配置句柄名（来自 `<Module>.h` / `<Module>_Cfg.h`），确保不出现编译错误；TTDet 三个 ID 均为 `extern volatile`（由 Det 桩提供），如桩未提供需先声明。

## 中断时间测试（ISR 耗时与中断延迟）

除 API 调用耗时外，还可测量驱动中断（ISR）上下文的耗时，复用同一套 `SYSTICK_GET_COUNTER` / `SYSTICK_DELTA_INNER` 宏：

1. **ISR 执行耗时**：在驱动通知回调（如 `Adc0Group_0_CallBack`，由驱动 ISR 调用）入口 `SYSTICK_GET_COUNTER(isrTime1)`，回调退出前 `SYSTICK_DELTA_INNER(isrTime1, isrTime2)`，得到回调/ISR 执行周期数。
   - ISR 中直接 `printf` 有重入与阻塞风险，建议把 `isrTime2` 存入 `volatile` 全局变量，由主循环读取打印；环境支持原子/中断安全 printf 时也可直接打印。
   - 回调体为空时测得的是空回调开销；需测驱动 ISR 全部逻辑时，需在驱动 ISR 源码（如 `Adc_Irq.c`）入口/出口插桩。
2. **中断延迟**：主循环在触发转换/事件前 `SYSTICK_GET_COUNTER(time1)`，回调入口 `SYSTICK_DELTA_INNER(time1, isrLatency)`，得到"触发→回调入口"总耗时（含转换时间 + ISR 响应延迟）；纯 ISR 响应延迟需在驱动 ISR 入口插桩测量。
3. **嵌套与抢占**：存在更高优先级中断抢占时测量结果偏大；需要干净数值时，测量期间关闭同级/低优先级中断，或多次测量取最小值。
4. 参考模板见 `references/isr_time_example.c`。

## Gtm 驱动性能测试经验（ThinkTech Alioth / A8 平台实测）

### API 调用顺序（运行时状态机，顺序错会返回 `TT_RET_STATUS_ERR`）
- **CMU**：`Gtm_Drv_Init` 后 CMU 已处于 RUNNING（`Gtm_Drv_PriInitStatusUpdate` 置位）。因此 `Gtm_Drv_CmuStart` / `Gtm_Drv_CmuSetClock` 必须在 `Gtm_Drv_CmuStop` **之后**调用；`CmuSetClock` 仅在 CMU 停止时可设。推荐顺序：
  `Init → CmuGetClock → CmuStop → CmuSetClock(读回原频) → CmuStart → … → CmuStop → DeInit`。
- **TOM**：`TomChannelEnable` 要求通道 STOPPED，`TomChannelDisable` 要求 RUNNING → 先 Enable 后 Disable（`TOM_AT_START` / `TOM_EN_OUTPUT_AT_START` 为立即模式）。
- **TBU**：`TbuStart` 后所有使能通道 RUNNING；`TbuStopChannel` 要求 RUNNING、`TbuStartChannel` 要求 STOPPED → 测法：`TbuStart → … → TbuStopChannel → TbuStartChannel → TbuStop`。
- **TIM**：`TimStartMeasure` 要求 STOPPED、`TimStopMeasure` 要求 RUNNING；`TimSetInt` 运行时检查要求通道 **STOPPED**，必须先于 `StartMeasure` 调用。`IrqMask` 须 < 4（`IrqMask >> TIM_INTERRUPT_IRQ_BIT_NUM(2) == 0`）。
- **DTM**：`DtmStart` 要求 STOPPED、`DtmStop` 要求 RUNNING。

### 配置相关 API 限制（当前配置不可测/易错）
- `TimGetPulseTime` 要求通道为 `TIM_TPIM_MODE`；若全部配置为 `TIM_TPWM_MODE` 会返回模式错误 → 跳过并注明原因，不要硬测。
- `TimClearStatusFlags` 的 `ClearStatus` 必须 ≤ `TIM_GLITCHDET_FLAG`（0x5）；传组合掩码（如 0x3F）会返回 `TT_RET_PARAM_ENUM_ERR`。

### 中断时间测试（TOM 通道，实测链路）
- 中断链路：`Platform_Hw_IntCtrl` 生成配置使能 `GTM_TOM0_0_IRQn` → 向量表 `Os_Entry_GTM_TOM0_0_IRQHandler`（`ISR(x)=void Os_Entry_##x(void)`，驱动强符号覆盖 weak 默认）→ TOM 通道 `Ccu1TcIrqEn=TRUE` + `TomNotify=Tom0ChannelX_CB` → `CN0==CM1` 触发。
- **测 ISR 整体耗时要在驱动 ISR 源码（如 `Gtm_Hw_Tom.c` 的 `ISR(GTM_TOM0_0_IRQHandler)`）入口/出口插桩**；只插桩通知回调测不到驱动 ISR 框架开销。
- 插桩写法：`extern` 变量提到文件顶部（函数内 extern 声明零运行时开销但风格差）；ISR 内**不要 printf**，只写 volatile 全局，由主循环读取打印。
- 触发方式：`TomSetCounterCn0=0`、`TomSetCounterCm0=60000`、`TomSetCounterCm1=10000`，再 `TomChannelEnable(TOM_ON_GLOBAL_TRIGGER,…)` + `TomSetHostTrigger`；触发前记录 `isrTrigTime`，主循环轮询 `isrCount`（带超时）等第一次 ISR。
- **中断风暴**：若所有通道 `Ccu1TcIrqEn=TRUE` 且 CM0/CM1=0，通道一使能即持续触发中断，严重干扰 API 计时 → 测试前先把 CM0/CM1 调大。
- 通知回调必须 ISR 安全（不 printf），否则 UART 重入/阻塞。

### 其他
- GTM 配置句柄（`TOM_0_CHANNEL_x`、`TBU_CHANNEL0`、`CDTM0_DTM0`、`TIM_0_CHANNEL_0`、`CMU_GLOBAL_CLK` 等）来自生成的 `Gtm_Drv_Cfg.h`（EB 输出目录，如 `C:\EB\tresos\…\output\include`）；EB 重新生成后配置会变（通道数、IRQ 使能、回调绑定），须重新核对。
- 驱动 ISR 插桩引用的测试变量定义在 `<Module>_PerfTest.c`；移除测试文件时须同步删除插桩，否则链接失败。
- 单文件编译验证用工程 `Debug/src/.../subdir.mk` 里的 clang 命令（注意 Gtm 驱动子目录 `-Os`、app `src` 目录 `-O0`，且依赖 arm-llvm 浮动许可证，并发可能超限需重试）。

## 宏与模板说明

- `DEF_CASE(func)`：自动展开为包装函数 `func()`（打印 `start!`，调用 `func_Call()`，按 `FailFlag` 打印 `pass!`/`failed!`）与用例体函数 `func_Call()`。用例体写在 `DEF_CASE(...) { ... }` 内。
- `dbg_check_compare(Func, CompareA, CompareB)`：两者不等时 `FailFlag++` 并打印 `[ Error ]`。
- `dbg_check_error(Func)`：检查 3 个 TTDet 错误 ID，非 0 则打印、`FailFlag++` 并清零。
- `SYSTICK_GET_COUNTER(Time)`：复位 SysTick `CVR` 到 0xFFFFFF 并取当前计数值（24 位递减计数）。
- `SYSTICK_DELTA_INNER(Time1, Time2)`：`Time2 = Time1 - 当前CVR`，差值即 API 执行周期数。
- `DBG_PRINTF(func)`：TTDet 无错误时打印 `runtime:<time2>`；有错误时打印 `runtime:0` 并列出各错误 ID。
- `PRINT_INFO(...)`：受 `_PRINTF_INFO_` 宏开关控制。

## 注意事项

- 测得的数值是 **SysTick 周期数**，不是时间；换算毫秒需先确认 SysTick 时钟频率（示例 main.c 用 `Mcu_Drv_Clk_GetPeriph(MCU_CLOCK_SYSTEM, ...)` 获取系统频率）。
- 每个 API 测量前必须清零 TTDet 错误 ID，避免历史错误污染 `DBG_PRINTF` 判定。
- `FailFlag` 是 `test.h` 内的 `static` 变量，每个 .c 翻译单元各自独立，跨文件断言需各自定义。
- 若驱动有回调（如 `Adc0Group_0_CallBack`），需提供空实现避免链接错误。
- 性能测量受中断影响，如需要精确值可在关中断下测量（本框架未默认关中断）。
