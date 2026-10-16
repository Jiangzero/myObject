/* 中断时间测试参考：以 ADC 组转换完成通知回调为例
 * 其他模块按此模式替换回调名与触发方式即可。
 * 前提：SYSTICK_GET_COUNTER / SYSTICK_DELTA_INNER 已在用例 .c 中定义。 */
#include "TT_Common.h"
#include "test.h"
#include "Adc.h"

uint32 isrTime1 = 0;
volatile uint32 isrTime2 = 0;    /* ISR 执行耗时（周期数），由主循环读取打印 */
volatile uint32 isrLatency = 0;  /* 触发 → 回调入口 总耗时（周期数） */

void Adc0Group_0_CallBack(void)
{
    /* 1) 中断延迟：主循环在触发转换前已执行 SYSTICK_GET_COUNTER(time1) */
    SYSTICK_DELTA_INNER(time1, isrLatency);

    /* 2) ISR 执行耗时：入口记录，出口前计算 */
    SYSTICK_GET_COUNTER(isrTime1);
    /* ... 回调自身逻辑（如有） ... */
    SYSTICK_DELTA_INNER(isrTime1, isrTime2);
    /* 不要在 ISR 内直接 printf，除非环境支持中断安全 printf */
}

/* 主循环中打印 ISR 结果（在 SIT_<Module>_TEST 用例末尾调用） */
void PrintIsrResults(void)
{
    printf("ADC ISR runtime:%d latency:%d\r\n", isrTime2, isrLatency);
}

/* 用例体中的触发段（示意）：
   DEF_CASE(SIT_Adc_411)
   {
       SYSTICK_GET_COUNTER(time1);
       Adc_StartGroupConversion(Adc0Group_0);   /* 触发后由 ISR 回调记录延迟与耗时 */
       Base_Drv_Msdelay(100);
       PrintIsrResults();
   }
*/
