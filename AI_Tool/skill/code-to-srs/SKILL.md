---
name: "code-to-srs"
description: "从MCAL/SafetyLib C代码生成SRB/SRL格式的详设文档。当用户要求根据C代码编写详设、生成SRB条目或整理单元设计说明时调用此skill。"
---

# Code to SRS - 从C代码生成详设文档

## 适用场景

当用户提供以下内容时调用此skill：
1. 一段MCAL/SafetyLib风格的C代码（如 Icu_Init、Icu_DeInit 等）
2. 要求根据C代码编写 `[SRB_XXX_YYY]` 格式的详设条目
3. 要求整理单元设计说明（Unit Design Description）

## 使用步骤

### 第一步：理解代码结构

读取目标函数的完整实现，关注以下核心模式：

#### 模式1：参数检查（ParamCheck）

```c
#if (XXX_PRECOMPILE_SUPPORT == STD_ON)
    Xxx_PriParamCheck(NULL_PTR != ConfigPtr, XXX_INIT_ID, XXX_E_PARAM_POINTER, &RetVal);
#else
    Xxx_PriParamCheck(NULL_PTR == ConfigPtr, XXX_INIT_ID, XXX_E_PARAM_POINTER, &RetVal);
#endif
```

**详设映射**：
```
[SRB_Xxx_2xx] 如果开启 XXX_PRECOMPILE_SUPPORT，判断 ConfigPtr 是否不为空，如果不为空上报 XXX_E_PARAM_POINTER；
如果未开启 XXX_PRECOMPILE_SUPPORT，判断 ConfigPtr 是否为空，如果为空上报 XXX_E_PARAM_POINTER 错误。(SRL_Xxx_xxx)
```

#### 模式2：全局状态检查

```c
Xxx_PriParamCheck(XXX_STATE_IDLE == Xxx_PubGlobalState, XXX_INIT_ID, XXX_E_ALREADY_INITIALIZED, &RetVal);
```

**详设映射**：
```
[SRB_Xxx_2xx] 检查全局状态是否为空闲（XXX_STATE_IDLE），若是，则上报 XXX_E_ALREADY_INITIALIZED 错误。(SRL_Xxx_xxx)
```

#### 模式3：CRC校验

```c
Xxx_PriInitParaCrcCheck(Xxx_PubConfig, &RetVal);
```

**详设映射**：
```
[SRB_Xxx_2xx] 进行 CRC 计算匹配，若失败，则上报 TT_RET_PARAM_CONFIG_ERR 错误。(SRL_Xxx_xxx)
```

#### 模式4：条件执行

```c
if(E_OK == RetVal)
{
    Ret = Xxx_Hw_FtmInit(Xxx_PubConfig->FtmConfig);
```

**详设映射**：
```
[SRB_Xxx_2xx] 若以上操作未报错，则调用 Xxx_Hw_Init 进行初始化。(SRL_Xxx_xxx)
```

#### 模式5：硬件初始化返回值检查

```c
if(TT_RET_SUCCESS == Ret)
{
    // 成功处理
}
else
{
    Xxx_PriParamCheck(TRUE, XXX_INIT_ID, XXX_E_INIT_FAILED, &RetVal);
}
```

**详设映射**：
```
[SRB_Xxx_2xx] 若 Xxx_Hw_Init 函数返回值不为 TT_RET_SUCCESS，则上报 XXX_E_INIT_FAILED 错误。(SRL_Xxx_xxx)
```

#### 模式6：成功后的状态设置

```c
for(Channel = 0; Channel < Xxx_PubConfig->ChannelConfigNumber; Channel++)
{
    Xxx_PubChannelInfo[Channel].ChannelNotificationStatus = FALSE;
    Xxx_PubChannelInfo[Channel].ChannelStatus = XXX_CH_STATUS_INITIALIZED;
}
Xxx_PubMode = XXX_MODE_NORMAL;
Xxx_PubGlobalState = XXX_STATE_IDLE;
```

**详设映射**：
```
[SRB_Xxx_2xx] 若 Xxx_Hw_Init 函数返回 TT_RET_SUCCESS，则将通道的回调函数设置为 FALSE，
通道状态设置为已初始化（XXX_CH_STATUS_INITIALIZED），功耗模式设置为 XXX_MODE_NORMAL，
全局状态设置为 XXX_STATE_IDLE。(SRL_Xxx_xxx)
```

### 第二步：生成 SRB 条目

#### SRB 编号建议

- SRB_Xxx_2xx：初始化/反初始化类函数
- SRB_Xxx_3xx：配置/设置类函数
- SRB_Xxx_4xx：启动/停止类函数
- SRB_Xxx_5xx：状态获取类函数
- SRB_Xxx_6xx：中断/回调处理类函数

#### SRB 格式

```markdown
[SRB_Xxx_xxx] <功能描述>。(SRL_Xxx_xxx)
```

### 第三步：生成函数信息表

| 项目 | 内容 |
|------|------|
| Service ID | 0xNN |
| Sync/Async | Synchronous |
| Reentrancy | Non Reentrant |
| Parameters(in) | 按实际参数填写 |
| Parameters(out) | 按实际参数填写 |
| Return value | 按实际返回值填写 |
| Description | AUTOSAR规范的函数描述 |

### 第四步：校验

1. SRB编号合理且唯一
2. SRB描述与代码逻辑一致
3. SRL编号与需求追踪矩阵一致
4. 确认ParamCheck条件语义正确：
   - `Xxx_PriParamCheck(CONDITION, ...)` 中当 CONDITION 为 TRUE 时上报错误
   - 所以SRB描述的是"上报错误的条件"，而非"期望的条件"

## ParamCheck 条件对照表

| 代码表达式 | 详设描述 |
|-----------|---------|
| `NULL_PTR != ConfigPtr` | 判断 ConfigPtr 是否不为空，如果不为空 |
| `NULL_PTR == ConfigPtr` | 判断 ConfigPtr 是否为空，如果为空 |
| `XXX_STATE_IDLE == gState` | 检查全局状态是否为空闲，若是 |
| `TRUE` | （无条件）上报错误 |
| `Ret != TT_RET_SUCCESS` | 若返回值不为 TT_RET_SUCCESS |

## 注意事项

1. **错误上报条件语义**：ParamCheck的表达式是"上报错误的条件"，而非"正确条件"

2. **PERF开关**：`TT_XXX_PERF == STD_ON` 时参数检查和CRC校验被排除。详设应标注哪些步骤受PERF开关影响

3. **编号连贯性**：SRB编号使用连贯编号（如 202、203、204），首位数字表示功能分类

4. **描述精确性**：详设描述应与代码逻辑完全一致，包括条件、错误码、变量名、状态值的准确名称

5. **内部私有函数的ParamCheck需要展开为独立SRB条目**：当公开函数中调用了包含参数校验逻辑的私有函数（如 `Xxx_PriParamCheckChannel`）时，必须将该私有函数内部的**每个 ParamCheck 调用展开为一条独立的 SRB 条目**，每个错误码对应一条 SRB。例如 `Icu_PriParamCheckChannel` 内部包含 3 个 ParamCheck，则展开为 3 条独立 SRB：

   - [SRB_Icu_xxx] 检查全局状态是否为未初始化（ICU_STATE_UNINIT），若是，上报 ICU_E_UNINIT 错误。(SRL_Icu_xxx)
   - [SRB_Icu_xxx] 检查 Channel 是否大于等于 ChannelConfigNumber，若是，上报 ICU_E_PARAM_CHANNEL 错误。(SRL_Icu_xxx)
   - [SRB_Icu_xxx] 检查 ChannelConfig[Channel] 是否为空，若是，上报 ICU_E_PARAM_CHANNEL 错误。(SRL_Icu_xxx)

   此规则与规则6的区别：规则6适用于调用实现了"功能语义"的私有函数（如硬件初始化、寄存器操作），这些不展开；而ParamCheckChannel这类专门做参数校验的私有函数，内部每个错误检查需要展开。

6. **私有函数只描述功能语义，不展开内部实现**：当公开函数中调用了私有（static/LOCAL_INLINE）函数时，SRB的描述应该**只描述该调用所实现的功能和效果**，而不需要将私有函数内部的实现细节展开描述。例如：
   - 调用 `Icu_Hw_PriRegReadBack(reg, expected, &Ret)` → 描述为"对寄存器进行回读校验，确认配置已正确写入硬件"
   - 调用 `Ftm_Hw_PriSetChannelInputFilter(FtmBase, ch, val)` → 描述为"设置通道输入滤波值"

   私有函数本身**不需要**生成独立的SRB条目（它们属于内部实现细节，是黑盒封装）。例外：如果私有函数逻辑复杂且具有独立的设计决策，可以为其单独生成内部SRB，但公开函数的SRB中仍然只描述调用该函数的功能语义即可。

## 完整示例

### 输入：Icu_Init 函数代码

```c
void Icu_Init(TT_CONST Icu_ConfigType *ConfigPtr)
{
#if (defined(TT_ICU_PERF) && (TT_ICU_PERF == STD_OFF))
    Std_ReturnType RetVal = E_OK;
    TT_RetType Ret = TT_RET_SUCCESS;

#if (ICU_PRECOMPILE_SUPPORT == STD_ON)
    Icu_PriParamCheck(NULL_PTR != ConfigPtr, ICU_INIT_ID, ICU_E_PARAM_POINTER, &RetVal);
#else
    Icu_PriParamCheck(NULL_PTR == ConfigPtr, ICU_INIT_ID, ICU_E_PARAM_POINTER, &RetVal);
#endif
#endif

#if (ICU_PRECOMPILE_SUPPORT == STD_ON)
    Icu_PubConfig = Icu_Config[BASE_GET_LOGICAL_CORE_ID];
    (void)ConfigPtr;
#else
    Icu_PubConfig = ConfigPtr;
#endif

#if (defined(TT_ICU_PERF) && (TT_ICU_PERF == STD_OFF))
    Icu_PriParamCheck(ICU_STATE_IDLE == Icu_PubGlobalState, ICU_INIT_ID, ICU_E_ALREADY_INITIALIZED, &RetVal);
    Icu_PriInitParaCrcCheck(Icu_PubConfig, &RetVal);

    if(E_OK == RetVal)
#endif
    {
#if (defined(TT_ICU_PERF) && (TT_ICU_PERF == STD_OFF))
        Ret = Icu_Hw_FtmInit(Icu_PubConfig->FtmConfig);
        if(TT_RET_SUCCESS == Ret)
#else
        (void)Icu_Hw_FtmInit(Icu_PubConfig->FtmConfig);
#endif
        {
            for(Icu_ChannelType Channel = 0; Channel < Icu_PubConfig->ChannelConfigNumber; Channel++)
            {
                Icu_PubChannelInfo[Channel].ChannelNotificationStatus = FALSE;
                Icu_PubChannelInfo[Channel].ChannelWakeupStatus = FALSE;
                Icu_PubChannelInfo[Channel].ChannelStatus = ICU_CH_STATUS_INITIALIZED;
            }
            Icu_PubMode = ICU_MODE_NORMAL;
            Icu_PubGlobalState = ICU_STATE_IDLE;
        }
#if (defined(TT_ICU_PERF) && (TT_ICU_PERF == STD_OFF))
        else
        {
            Icu_PriParamCheck(TRUE, ICU_INIT_ID, ICU_E_INIT_FAILED, &RetVal);
        }
#endif
    }
}
```

### 输出：Icu_Init 详设

```
/******************************************************************************
*  Service name    : Icu_Init
*  Syntax          : void Icu_Init(TT_CONST Icu_ConfigType *ConfigPtr)
*  Service ID      : 0x00
*  Sync/Async      : Synchronous
*  Reentrancy      : Non Reentrant
*  Parameters (in) : ConfigPtr : Pointer to a selected configuration structure
*  Parameters (out): None
*  Return value    : None
*  Description     : Initializes the ICU driver.
******************************************************************************/

[SRB_Icu_202] 如果开启 ICU_PRECOMPILE_SUPPORT，判断 ConfigPtr 是否不为空，如果不为空上报 ICU_E_PARAM_POINTER；如果未开启 ICU_PRECOMPILE_SUPPORT，判断 ConfigPtr 是否为空，如果为空上报 ICU_E_PARAM_POINTER 错误。(SRL_Icu_051)

[SRB_Icu_203] 检查全局状态是否为空闲（ICU_STATE_IDLE），若是，则上报 ICU_E_ALREADY_INITIALIZED 错误。(SRL_Icu_051)

[SRB_Icu_204] 进行 CRC 计算匹配，若失败，则上报 TT_RET_PARAM_CONFIG_ERR 错误。(SRL_Icu_051)

[SRB_Icu_205] 若以上操作未报错，则调用 Icu_Hw_Init 进行初始化。(SRL_Icu_051)

[SRB_Icu_206] 若 Icu_Hw_FtmInit 函数返回值不为 TT_RET_SUCCESS，则上报 ICU_E_INIT_FAILED 错误。(SRL_SM_Icu_010)

[SRB_Icu_207] 若 Icu_Hw_FtmInit 函数返回 TT_RET_SUCCESS，则将通道的回调函数设置为 FALSE（ChannelNotificationStatus = FALSE），通道状态设置为已初始化（ICU_CH_STATUS_INITIALIZED），功耗模式设置为 ICU_MODE_NORMAL，全局状态设置为 ICU_STATE_IDLE。(SRL_Icu_051)
```