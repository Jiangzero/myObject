---
name: "mcal-commenter"
description: "Adds ThinkTech/AUTOSAR-style comments to MCAL driver files (file header, enums, structs, typedef aliases, macros, variables, private functions, Hw-layer interfaces, AUTOSAR service prototypes). Invoke when the user asks to add/annotate comments for MCAL driver files such as the Gtm_TS_TTAMM1I0P0 plugin files (Gtm_Drv_Types.h, Gtm_Drv.h, Gtm_Drv.c, Gtm_Hw_Cmu.c) or any file following the same ThinkTech MCAL style."
---

# MCAL 驱动文件注释添加 Skill（mcal-commenter）

## 用途

为 ThinkTech MCAL 驱动源文件（如 `Gtm_TS_TTAMM1I0P0` 插件）中的代码元素**补充注释**，注释风格与该插件现有代码完全一致。类型不同，注释格式不同：

| 代码元素 | 注释格式 |
| --- | --- |
| 文件头 | 框式版权块（FILENAME / VERSION / PLATFORM / VENDOR / DESCRIPTION） |
| 章节分隔条 | `/***** 标题 *****/` 框式 |
| `#define` 宏 | 宏上方单行 `/* 描述 */`，可按组加组注释 |
| 枚举类型 | 类型上方 `/* 描述 */`，每个枚举成员上方 `/* 描述 */` |
| 结构体类型 | 类型上方 `/* 描述 */`，每个结构体成员上方 `/* 描述 */` |
| typedef 别名 | 固定格式 `/* An alias for the <原类型>. */` |
| 全局/私有变量 | 变量上方单行 `/* 描述 */` |
| 私有函数（Drv 层 `static`/`LOCAL_INLINE`/含 `Pri`） | Doxygen 风格（`@brief`/`@details`/`@param[in]`/`@param[out]`/`@return`/`@retval`） |
| Hw 层所有接口（私有与公共，`Gtm_Hw_*`） | Doxygen 风格（同私有函数格式） |
| Drv 层非私有公共服务函数（`Gtm_Drv_*`，有 Service ID） | AUTOSAR 服务块格式 |

## 参考文件

风格来源于以下文件（注释格式以此为准）：

- `m7/assembly/plugins/Gtm_TS_TTAMM1I0P0/include/Gtm_Drv_Types.h` —— 宏、枚举、结构体、typedef 别名
- `m7/assembly/plugins/Gtm_TS_TTAMM1I0P0/include/Gtm_Drv.h` —— 全局变量、Doxygen 私有函数、AUTOSAR 服务块原型
- `m7/assembly/plugins/Gtm_TS_TTAMM1I0P0/src/Gtm_Drv.c` —— 私有变量、私有函数声明/定义、AUTOSAR 服务块定义、函数体内注释
- `m7/assembly/plugins/Gtm_TS_TTAMM1I0P0/src/Gtm_Hw_Cmu.c` —— Hw 层私有函数与公共接口的 Doxygen 风格

## 详细格式规则

### 1. 文件头块

置于每个文件最顶部，`FILENAME` 改为实际文件名，`DESCRIPTION` 为该模块一句话描述（如 `Gtm Driver Source File`），其余字段保持不变：

```c
/*******************************************************************************
**                                                                            **
** Copyright (C) ThinkTech Technologies (2026)                                **
**                                                                            **
** All rights reserved.                                                       **
**                                                                            **
** This document contains proprietary information belonging to ThinkTech      **
** Technologies. Passing on and copying of this document, and communication   **
** of its contents is not permitted without prior written authorization.      **
**                                                                            **
********************************************************************************
**                                                                            **
**  FILENAME     : <FileName>                                                 **
**                                                                            **
**  VERSION      : 1.0.0                                                      **
**                                                                            **
**  PLATFORM     : ThinkTech Alioth Series                                    **
**                                                                            **
**  VENDOR       : ThinkTech Technologies                                     **
**                                                                            **
**  DESCRIPTION  : <Module> Driver Source File                                **
**                                                                            **
**  MAY BE CHANGED BY USER : no                                               **
**                                                                            **
*******************************************************************************/
```

### 2. 章节分隔条

按章节放置，标题居中对齐。**头文件（.h）中格式**（内容行无前导空格）：

```c
/******************************************************************************
*                            Defines And Macros
*******************************************************************************/
```

**源文件（.c）中格式**（内容行前有一个空格）：

```c
/******************************************************************************
 *                            Includes
 ******************************************************************************/
```

常见章节（按文件类型选用）：`Includes`（Hw 层用 `Include Files`）、`Source File Version Information`、`File Version Checks`、`Defines And Macros`、`Enums`、`Structures And Typedefs`、`Global Variable Declarations`（Hw 层为 `Global Variables Declaration`）、`Private Constants`、`Private Variables`、`Public Constants`、`Public Variables`、`Private Function Declaration`、`Public Prototypes`、`Public Functions`。

### 3. `#define` 宏

每个宏上方一行 `/* 描述 */`；同组宏可先在组前加一行组注释：

```c
/* CMU API Service IDs */
/* API Service ID of the CMU clock start function */
#define GTM_CMU_START_ID                            (102U)
/* API Service ID of the CMU clock stop function */
#define GTM_CMU_STOP_ID                             (103U)
```

宏名与数值列保持对齐。版本信息宏（`*_VENDOR_ID`、`*_MODULE_ID`、`*_AR_RELEASE_*`、`*_SW_*`）视文件现状决定是否加逐行注释（`Gtm_Drv_Types.h` 中加了，`Gtm_Drv.h` 中未加——与所在文件保持一致）。

### 4. 枚举类型

类型上方一行 `/* 描述 */`，每个成员上方一行 `/* 描述 */`：

```c
/* GTM module operating status flag */
typedef enum {
    /* GTM module is not initialized */
    GTM_STATUS_UNINITIALIZED,
    /* GTM module is initialized */
    GTM_STATUS_INITIALIZED,
    /* GTM module is running */
    GTM_STATUS_RUNNING,
    /* GTM module is stopped */
    GTM_STATUS_STOPPED
} Gtm_Drv_StatusType;
```

### 5. 结构体类型

类型上方一行 `/* 描述 */`，每个成员上方一行 `/* 描述 */`，成员类型与名称列对齐：

```c
/* GTM all submodule status flag structure type */
typedef struct {
    /* Gtm initialization flag */
    Gtm_Drv_StateType    GtmInitState;
    /* Cmu working status flag */
    Gtm_Drv_CmuFlagType  CmuFlag;
} Gtm_Drv_FlagType;
```

### 6. typedef 别名

固定格式 `/* An alias for the <原类型>. */`：

```c
/* An alias for the Dtm_ConnectionsConfigType. */
typedef Dtm_ConnectionsConfigType          Gtm_Drv_ConnectionsConfigType;
```

### 7. 全局 / 私有变量

变量上方单行 `/* 描述 */`：

```c
/* The status flag pointer of the Cmu module */
static Gtm_Drv_CmuFlagType *Cmu_PubFlagPtr = &Gtm_PubFlag.CmuFlag;
```

```c
/* Configuration data of the GTM driver. */
extern TT_CONST Gtm_Drv_ConfigType Gtm_Config;
```

### 8. 私有函数（Doxygen 风格）

Drv 层中 `static`、`LOCAL_INLINE`、名字含 `Pri` 的函数——**声明（Private Function Declaration / Public Prototypes 中的 LOCAL_INLINE）与定义（Private Functions 区）使用相同 Doxygen 注释**：

```c
/**
* @brief        Gtm_Drv_PriInitStatusUpdate
* @details      Update gtm and submodule initialization status.
* @param[in]    ConfigPtr   Configuration data pointer
* @param[out]   None
* @return       None
* @retval       None
*/
static void Gtm_Drv_PriInitStatusUpdate(TT_CONST Gtm_Drv_ConfigType *ConfigPtr);
```

格式要点：

- `@brief` 后接**函数名**；
- `@details` 一句话描述功能；
- 每个入参一行 `@param[in]    <参数名> : <描述>`（或 `-` 分隔，与参考文件一致即可），多个参数名对齐；
- `@param[out]` 出参，无则写 `None`；
- `@return` 写返回类型（如 `None`、`uint32`、`TT_RetType`）；
- `@retval` 写返回值含义（如 `TT_RET_SUCCESS - success`、`TRUE - valid`），返回 `void` 时写 `None`（或按参考文件省略该行）。

返回值有含义的示例：

```c
/**
* @brief        Gtm_Drv_PriGetCdtmIndex
* @details      Gets the Cdtm instance number.
* @param[in]    CdtmDtmIndex:    Cdtm instance
* @param[out]   None
* @return       uint32
* @retval       Cdtm instance
*/
LOCAL_INLINE uint32 Gtm_Drv_PriGetCdtmIndex(Gtm_Hw_Dtm_IndexType CdtmDtmIndex)
```

多返回值含义分行：

```c
/**
* @brief        Gtm_Drv_PriIsIndexValid
* @details      Check that ClockIndex is within the valid range of a particular ClockSource.
* @param[in]    ClockSource:clock source
* @param[in]    ClockIndex:clock index
* @return       boolean
* @retval       TRUE - valid
                FALSE - invalid
*/
```

### 9. Hw 层接口（同样使用 Doxygen 风格，即"按私有函数的注释格式"）

**Hw 层（`Gtm_Hw_*`）的私有函数与公共接口全部使用 Doxygen 风格**，不用 AUTOSAR 服务块。私有函数声明（Private Function Declaration）：

```c
/**
* @brief        Gtm_Hw_Cmu_PriGetClusterClk
* @details      Get cluster clock.
* @param[in]    ClusterIndex : Index of the cluster instance
* @param[out]   Freq         : Gets parameters for clock data
* @return       None
* @retval       None
*/
static void Gtm_Hw_Cmu_PriGetClusterClk(Gtm_Hw_Cmu_ClusterClockType ClusterIndex, float32 *Freq);
```

Hw 层公共接口（Public Functions 区，同样 Doxygen 风格）：

```c
/**
* @brief        Gtm_Hw_Cmu_ArchInit
* @details      Initialize the GTM architecture and CMU submodules.
* @param[in]    CmuConfig :Cmu Config
* @param[out]   None
* @return       TT_RetType
* @retval       TT_RET_SUCCESS - success
*               Others - fail
*/
TT_RetType Gtm_Hw_Cmu_ArchInit(TT_CONST Gtm_Hw_Cmu_ConfigType *CmuConfig)
```

### 10. Drv 层非私有公共服务函数（AUTOSAR 服务块格式）

Drv 层导出的服务函数（`Gtm_Drv_*`，非 `Pri`，有对应的 API Service ID）——**原型（.h）与定义（.c）均使用 AUTOSAR 服务块**：

```
/******************************************************************************
*  Service name     : Gtm_Drv_Init
*  Syntax           : TT_RetType Gtm_Drv_Init(TT_CONST Gtm_Drv_ConfigType *ConfigPtr)
*  Service ID       : 100
*  Sync/Async       : Synchronous
*  Reentrancy       : Non Reentrant
*  Parameters (in)  : ConfigPtr : Configuration data pointer
*  Parameters (out) : None
*  Return value     : TT_RET_SUCCESS: accepted
*                     Other         : not been accepted
*  Description      : This service is used to initialize the GTM module.
******************************************************************************/
```

格式要点：

- `Service name`：函数名；
- `Syntax`：完整的函数签名（与原型/定义一致）；
- `Service ID`：与 `Gtm_Drv_Types.h` 中的 API Service ID 宏值一致（如 `GTM_INIT_ID` = 100）；
- `Sync/Async`：通常 `Synchronous`；
- `Reentrancy`：`Non Reentrant` 或 `Reentrant`（查阅参考头文件中同类函数的实际值，如 `Gtm_Drv_CmuGetClock` 为 `Reentrant`）；
- `Parameters (in)`：每个入参一行 `<参数名> : <描述>`；
- `Parameters (out)`：出参，无则 `None`；
- `Return value`：`TT_RET_SUCCESS: accepted`，换行 `Other         : not been accepted`；
- `Description`：一句话功能描述（结尾带句号）。

多入参示例（对齐参数名）：

```
/******************************************************************************
*  Service name     : Gtm_Drv_CmuGetClock
*  Syntax           : TT_RetType Gtm_Drv_CmuGetClock(TT_CONST Gtm_Drv_Cmu_ClkSourceType ClockSource, TT_CONST uint8 ClockIndex, float32 *Clk)
*  Service ID       : 104
*  Sync/Async       : Synchronous
*  Reentrancy       : Reentrant
*  Parameters (in)  : ClockSource : GTM CMU clock source type (cluster cfgu, fixed and external clock subunit)
*  Parameters (in)  : ClockIndex  : GTM CMU clock index (index of the subunit clock)
*  Parameters (out) : Clk         : Save the read clock frequency (unit: Hz)
*  Return value     : TT_RET_SUCCESS: accepted
*                     Other         : not been accepted
*  Description      : Get the clock frequency of the specified clock source.
******************************************************************************/
```

> 注：参考头文件中既有 `*  Service name`（一个空格）也有 ` *  Service name`（两个空格）两种写法，编辑已有文件时保持该文件原有写法。

### 11. 函数体内行内注释

按逻辑语句块添加缩进对齐的 `/* 描述 */` 单行注释：

```c
    /* Update module status */
    Gtm_PubFlag.GtmInitState = GTM_STATE_INITIALIZED;

    /* Update cmu status */
    Cmu_PubFlagPtr->CmuStatus = GTM_STATUS_RUNNING;
```

对晦涩的硬件操作可加较长说明：

```c
    /* Local reset to prevent data remaining in the registers when the SMU is reset and reinitialized. */
    TT_GTM->CTRL.B.RF_PROT = 0;
```

空分支保持参考文件原样（如 `/* do nothing*/`），**不要**为显而易见的代码添加无意义注释。

## 通用规则

1. 注释一律使用**英文**，术语与现有代码一致（module、submodule、status flag、clock source、frequency 等）。
2. **已有注释不重复添加**；缺失的才补。若某文件/区域已有既定风格（如某文件版本宏不加逐行注释），保持一致。
3. 枚举/结构体成员、参数名、宏值等**列对齐**。
4. 条件编译块（`#if defined(...)` / `#if (… == STD_ON)`）内的元素同样要注释。
5. 函数注释的 `Service ID` 必须与 Types 文件中的 API Service ID 宏一致。
6. 判断函数属于"私有函数"还是"服务函数"：`static`、`LOCAL_INLINE`、名字含 `Pri` → Doxygen；`Gtm_Drv_*` 导出的带 Service ID 服务函数 → AUTOSAR 服务块；`Gtm_Hw_*` 全部 → Doxygen。
7. 不修改代码逻辑，仅添加/修正注释。

## 执行流程

1. 读取目标文件，确定其类型（Types 头 / 公共头 / Drv 源文件 / Hw 源文件）。
2. 按顺序检查：文件头 → 章节分隔条是否缺失 → 版本信息 → 宏 → 枚举 → 结构体/typedef → 变量 → 函数（声明与定义）。
3. 对照上述规则为缺失注释的元素补注释；已有注释保持不动。
4. 完成后通读检查：对齐、拼写、术语、Service ID 一致性。