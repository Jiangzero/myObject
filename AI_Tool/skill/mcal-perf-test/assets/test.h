#ifndef _TEST_H_
#define _TEST_H_

/* ------------------------------------------------------------------------------------------- */
static uint32 FailFlag = 0U;

#define DEF_CASE(func) 																\
void func ## _Call(void);                                    						\
void func(void)                                                                     \
{                                                                                   \
    printf("%s " "start!" "\r\n", #func);                                           \
    func ## _Call();                                                      			\
    if(FailFlag == 0)printf("%s %s\r\n", #func, "pass!");  							\
    if(FailFlag != 0)printf("%s %s\r\n", #func, "failed!");							\
}                                                                                   \
void func ## _Call(void)

#define dbg_check_compare(Func, CompareA, CompareB) do{ 							\
	if((CompareA) != (CompareB))													\
	{																				\
    	FailFlag++;                                                                 \
    	printf("[ Error ] %s : 0x%x 0x%x LINE=%d \r\n", #Func, CompareA, CompareB, __LINE__);\
	}																				\
}while(0)

#define dbg_check_error(Func) do{                                                   \
    if(TTDet_ReportErrorId != 0)                                                    \
    {                                                                               \
        printf("[ Error ] %s : ErrorId = 0x%x LINE=%d\r\n", #Func, TTDet_ReportErrorId, __LINE__); \
        FailFlag++;                                                                 \
        TTDet_ReportErrorId = 0;                                                    \
    }                                                                               \
    if(TTDet_ReportRuntimeErrorId != 0)                                             \
    {                                                                               \
        printf("[ Error ] %s : RuntimeErrorId = 0x%x LINE=%d\r\n", #Func, TTDet_ReportRuntimeErrorId, __LINE__); \
        FailFlag++;                                                                 \
        TTDet_ReportRuntimeErrorId = 0;                                             \
    }                                                                               \
    if(TTDet_ReportTransientFaultId != 0)                                           \
    {                                                                               \
        printf("[ Error ] %s : TransientFaultId = 0x%x LINE=%d\r\n", #Func, TTDet_ReportTransientFaultId, __LINE__); \
        FailFlag++;                                                                 \
        TTDet_ReportTransientFaultId = 0;                                           \
    }                                                                               \
}while(0)
/* ------------------------------------------------------------------------------------------- */

//#define _PRINTF_INFO_

#ifdef _PRINTF_INFO_
#define PRINT_INFO(...) 				do { 		\
	printf("[ %s  ] ", "Info"); 			 		\
	printf(__VA_ARGS__);                 			\
} while (0)
#else
#define PRINT_INFO(...)     			do { 		\
} while (0)
#endif

/* 目标模块测试入口映射，按模块替换，例如：
 *   #define SIT_ADC_TEST   SIT_Adc_410
 *   #define SIT_GTM_TEST   SIT_Gtm_410
 */
#define SIT_ADC_TEST                    SIT_Adc_410

extern void SIT_ADC_TEST(void);

#endif
