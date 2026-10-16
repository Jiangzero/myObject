/* 性能测试用例参考示例：ADC 驱动（来自 ThinkTech 工程）
 * 其他模块按此模式替换 API 与配置句柄即可。 */
#include "TT_Common.h"
#include "test.h"
#include "Adc.h"
#include "Base_Drv.h"

uint32 time1 = 0;
uint32 time2 = 0;
#define SYSTICK_GET_COUNTER(Time)          TT_SYSTICK->CVR = 0xFFFFFFU;\
                                           MCAL_DATA_SYNC_BARRIER();\
                                           Time = ((TT_SYSTICK->CVR) & TT_SYSTICK_CVR_CURRENT_MASK)
#define SYSTICK_DELTA_INNER(Time1, Time2)  Time2 = ((Time1) - ((TT_SYSTICK->CVR) & TT_SYSTICK_CVR_CURRENT_MASK))

#define DBG_PRINTF(func) do{\
	if(TTDet_ReportRuntimeErrorId != 0 || TTDet_ReportErrorId != 0 || TTDet_ReportTransientFaultId != 0)\
	{\
		printf("%s runtime:0\r\n", #func);\
		if(TTDet_ReportRuntimeErrorId != 0)\
		{\
			printf("TTDet_ReportRuntimeErrorId %x\r\n", TTDet_ReportRuntimeErrorId);\
		}\
		if(TTDet_ReportErrorId != 0)\
		{\
			printf("TTDet_ReportErrorId %x\r\n", TTDet_ReportErrorId);\
		}\
		if(TTDet_ReportTransientFaultId != 0)\
		{\
			printf("TTDet_ReportTransientFaultId %x\r\n", TTDet_ReportTransientFaultId);\
		}\
	}\
	else\
	{\
		printf("%s runtime:%d\r\n", #func, time2);\
	}\
}while(0)

uint16 adc0group0_buff[50] = {0};
uint16 adc0group0_data[50] = {0};

uint16 adc0group1_buff[50] = {0};
uint16 adc0group1_data[50] = {0};

Adc_ValueGroupType *PtrToSamplePtr_adc0group0;
Adc_ValueGroupType *PtrToSamplePtr_adc0group1;

extern volatile uint16 TTDet_ReportErrorId;
extern volatile uint16 TTDet_ReportRuntimeErrorId;
extern volatile uint16 TTDet_ReportTransientFaultId;

void Adc0Group_0_CallBack(void)
{
}

void Adc1Group_0_CallBack(void)
{
}


DEF_CASE(SIT_Adc_410)
{
    SYSTICK_GET_COUNTER(time1);
    SYSTICK_DELTA_INNER(time1,time2);
    printf("Base runtime:%d\r\n",time2);

    TTDet_ReportRuntimeErrorId = 0; TTDet_ReportErrorId = 0; TTDet_ReportTransientFaultId = 0;
    SYSTICK_GET_COUNTER(time1);
    Adc_Init(NULL_PTR);
    SYSTICK_DELTA_INNER(time1,time2);
    DBG_PRINTF(Adc_Init);

    Std_VersionInfoType VersionInf = {0,0,0,0,0};
    SYSTICK_GET_COUNTER(time1);
    Adc_GetVersionInfo(&VersionInf);
    SYSTICK_DELTA_INNER(time1,time2);
    if((VersionInf.vendorID != 214)||(VersionInf.moduleID != 123)||(VersionInf.sw_major_version != 1)||\
	   (VersionInf.sw_minor_version != 0)||(VersionInf.sw_patch_version != 0))
    {
        printf("Adc_GetVersionInfo runtime:0\r\n");
    }
    else
    {
        printf("Adc_GetVersionInfo runtime:%d\r\n",time2);
    }

    TTDet_ReportRuntimeErrorId = 0; TTDet_ReportErrorId = 0; TTDet_ReportTransientFaultId = 0;
    SYSTICK_GET_COUNTER(time1);
    Adc_SetupResultBuffer(Adc0Group_0, adc0group0_buff);
    SYSTICK_DELTA_INNER(time1,time2);
    DBG_PRINTF(Adc_SetupResultBuffer);

    Adc_SetupResultBuffer(Adc1Group_0, adc0group1_buff);

    TTDet_ReportRuntimeErrorId = 0; TTDet_ReportErrorId = 0; TTDet_ReportTransientFaultId = 0;
    SYSTICK_GET_COUNTER(time1);
    Adc_EnableGroupNotification(Adc0Group_0);
    SYSTICK_DELTA_INNER(time1,time2);
    DBG_PRINTF(Adc_EnableGroupNotification);

    TTDet_ReportRuntimeErrorId = 0; TTDet_ReportErrorId = 0; TTDet_ReportTransientFaultId = 0;
    SYSTICK_GET_COUNTER(time1);
    Adc_DisableGroupNotification(Adc0Group_0);
    SYSTICK_DELTA_INNER(time1,time2);
    DBG_PRINTF(Adc_DisableGroupNotification);

    TTDet_ReportRuntimeErrorId = 0; TTDet_ReportErrorId = 0; TTDet_ReportTransientFaultId = 0;
    SYSTICK_GET_COUNTER(time1);
    Adc_StartGroupConversion(Adc1Group_0);
    SYSTICK_DELTA_INNER(time1,time2);
    DBG_PRINTF(Adc_StartGroupConversion);

    Base_Drv_Msdelay(100);

    TTDet_ReportRuntimeErrorId = 0; TTDet_ReportErrorId = 0; TTDet_ReportTransientFaultId = 0;
    SYSTICK_GET_COUNTER(time1);
    Adc_StopGroupConversion(Adc1Group_0);
    SYSTICK_DELTA_INNER(time1,time2);
    DBG_PRINTF(Adc_StopGroupConversion);

    TTDet_ReportRuntimeErrorId = 0; TTDet_ReportErrorId = 0; TTDet_ReportTransientFaultId = 0;
    SYSTICK_GET_COUNTER(time1);
    Adc_EnableHardwareTrigger(Adc0Group_0);
    SYSTICK_DELTA_INNER(time1,time2);
    DBG_PRINTF(Adc_EnableHardwareTrigger);

    TTDet_ReportRuntimeErrorId = 0; TTDet_ReportErrorId = 0; TTDet_ReportTransientFaultId = 0;
    SYSTICK_GET_COUNTER(time1);
    Adc_ReadGroup(Adc0Group_0, adc0group0_data);
    SYSTICK_DELTA_INNER(time1,time2);
    DBG_PRINTF(Adc_ReadGroup);

    TTDet_ReportRuntimeErrorId = 0; TTDet_ReportErrorId = 0; TTDet_ReportTransientFaultId = 0;
    SYSTICK_GET_COUNTER(time1);
    Adc_GetStreamLastPointer(Adc0Group_0, &PtrToSamplePtr_adc0group0);
    SYSTICK_DELTA_INNER(time1,time2);
    DBG_PRINTF(Adc_GetStreamLastPointer);

    TTDet_ReportRuntimeErrorId = 0; TTDet_ReportErrorId = 0; TTDet_ReportTransientFaultId = 0;
    SYSTICK_GET_COUNTER(time1);
    Adc_DisableHardwareTrigger(Adc0Group_0);
    SYSTICK_DELTA_INNER(time1,time2);
    DBG_PRINTF(Adc_DisableHardwareTrigger);

    TTDet_ReportRuntimeErrorId = 0; TTDet_ReportErrorId = 0; TTDet_ReportTransientFaultId = 0;
    SYSTICK_GET_COUNTER(time1);
    Adc_GetGroupStatus(Adc0Group_0);
    SYSTICK_DELTA_INNER(time1,time2);
    DBG_PRINTF(Adc_GetGroupStatus);

    TTDet_ReportRuntimeErrorId = 0; TTDet_ReportErrorId = 0; TTDet_ReportTransientFaultId = 0;
    SYSTICK_GET_COUNTER(time1);
    Adc_DeInit();
    SYSTICK_DELTA_INNER(time1,time2);
    DBG_PRINTF(Adc_DeInit);

    printf("end\r\n");

}
