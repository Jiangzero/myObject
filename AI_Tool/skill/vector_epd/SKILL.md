---
name: vector_epd
description: 将 MCAL 插件的 XDM 文件适配为 Vector 达芬奇可用的 EPD 文件（AUTOSAR_MOD_ECUConfigurationParameters），包括替换 ecu 指令为固定值、调用 tresos_cmd.bat 转换、用 Amdc 校验并修复错误码。用于交付 Vector 的 MCAL 配置适配工作
---

# Vector EPD 文件适配 Skill

基于《Vector适配修改指导书》：将 EB tresos 的 XDM 配置文件转换为 Vector 达芬奇（DaVinci）工具可用的 EPD 文件。

## 背景概念

| 术语 | 含义 |
|------|------|
| ARXML | AUTOSAR XML，用 XML 描述 AUTOSAR 模型的人机可读文本格式 |
| EPD | 原始文件来自 `AUTOSAR_MOD_ECUConfigurationParameters.arxml`，Vector 拿到 EPD 后可在达芬奇中配置 MCAL |
| XDM | XML data format，各厂商基于 EPD 适配自家工具（XDM 由 EPD 生成） |

核心问题：XDM→EPD 转换过程中**无法识别 ecu 指令**（调用 resource 文件内容的操作），因此需将所有 `ecu:get()`/`ecu:list()` 等指令替换为固定合理值。

## 关键路径

| 用途 | 路径 |
|------|------|
| EB 命令行 | `D:\EB\tresos\bin\tresos_cmd.bat` |
| Amdc 校验工具 | `D:\EB\tresos\bin\AMDC_1_1_3\Amdc.bat` |
| 插件配置目录 | `D:\am-mcal-safetylib\m7\assembly\plugins\<Module>_TS_TTAMM1I0P0\config\` |
| 模块 XDM 文件 | `...\config\<Module>.xdm`（EB 使用） |
| 模块 EPD 版 XDM | `...\config\<Module>_Epd.xdm`（用于转换 EPD，与 `<Module>.xdm` 内容相同） |
| Resource 芯片定义 | `D:\am-mcal-safetylib\m7\assembly\plugins\Resource_TS_TTAMM1I0P0\resource\ThinkTech_Alioth_A8T8X_BGA292.properties` 等 |

> 注意：不同芯片封装对应不同 resource 文件，各配置项有差异，最终需为每种封装各生成一份 EPD。
> A8T8X 有 `ThinkTech_Alioth_A8T8X_BGA292` 和 `ThinkTech_Alioth_A8T8X_BGA373` 两种。

## 操作流程

### 1. 全局搜索 ecu 指令

在目标 XDM 文件中搜索所有 ecu 相关指令：

```powershell
Select-String -Path "<Module>_Epd.xdm" -Pattern "ecu:(list|get)\(" -Encoding UTF8
```

按 resource 文件确认每个指令对应的固定值。

### 2. 按修改要点替换为固定值

#### 2.1 RANGE（枚举/列表）

resource 文件定义（逗号分隔）：
```
Psi5.HwUnitList:0,1
```
XDM 中：
```xml
<a:da name="RANGE" type="XPath" expr="ecu:list('Psi5.HwUnitList')"/>
```
改为枚举全部选项：
```xml
<a:da name="RANGE">
    <a:v>0</a:v>
    <a:v>1</a:v>
</a:da>
```

#### 2.2 EDITABLE（可编辑状态）

- 若 resource 中对应功能为 TRUE（如 `ExtendedFunctionSupported:TRUE`）→ **删除** EDITABLE 配置项（保持可编辑）
- 若 resource 中为 FALSE → 改为不可编辑：
```xml
<a:a name="EDITABLE" value="false"/>
```

#### 2.3 DEFAULT（默认值）

XDM 中：
```xml
<a:da name="DEFAULT" type="XPath" expr="(ecu:list('Psi5.HwUnitList'))[position()-1=node:fallback(node:current()/../@index,'0')]">
</a:da>
```
改为固定合理值（取列表首个值）：
```xml
<a:da name="DEFAULT" value="0"/>
```

#### 2.4 INVALID（最大值/最小值约束）

**最大最小值类**：`ecu:get()` 改为 resource 对应固定值。
```xml
<a:da name="INVALID" type="Range">
    <a:tst expr="&lt;=ecu:get('Spi.SpiFrameSizeMax')"/>
    <a:tst expr="&gt;=4"/>
</a:da>
```
改为：
```xml
<a:da name="INVALID" type="Range">
    <a:tst expr="&lt;=32"/>
    <a:tst expr="&gt;=4"/>
</a:da>
```

**XPath 计数控制 MAP 最大值类**：
```xml
<v:lst name="SpiPhyUnit" type="MAP">
    <a:da name="MIN" value="1"/>
    <a:da name="INVALID" type="XPath">
        <a:tst expr="num:i(count(node:current()/*)) &gt; num:i(count(ecu:list('...SpiPhyUnitMapping')))"
            true="Maximum ... exceeded."/>
    </a:da>
</v:lst>
```
改为直接定义 MAX：
```xml
<v:lst name="SpiPhyUnit" type="MAP">
    <a:da name="MIN" value="1"/>
    <a:da name="MAX" value="8"/>
</v:lst>
```

**MAP 的 MAX 用 `ecu:get()` 时**（Psi5 常见模式）：
```xml
<a:da name="MAX" type="XPath" expr="num:i(ecu:get('Psi5.HwUnitNumber'))"/>
```
改为：
```xml
<a:da name="MAX" value="2"/>
```

#### 2.5 动态填充内容

Port 中根据 pad number 联动变化的内容，function 默认填 0 即可。

### 3. 转换 XDM→EPD

添加 EB bin 目录到 PATH，执行转换指令：

```powershell
# 注意 asc:4.4.0 为 release 版本，需与 XDM 中 <a:a name="RELEASE" value="asc:4.4"/> 匹配
D:\EB\tresos\bin\tresos_cmd.bat -DValidate=false -DMapOptionalAsList=false legacy convert Psi5_Epd.xdm Psi5.epd@asc:4.4.0
```

### 4. 校验 EPD 文件（Amdc）

```powershell
D:\EB\tresos\bin\AMDC_1_1_3\Amdc.bat -s Psi5.epd
```

## EPD 错误码修复

| 错误码 | 含义 | 修复方法 |
|--------|------|----------|
| A204 / A221 | 参数格式错误 | 不能有中线（`-`），不能是纯数字 |
| A206 | ECUC 参数错误 | 把 `ECUC` 改成 `AUTOSAR_ECUC` |
| A205 | VariantLinkTime 错误 | 删除 `VariantLinkTime` 相关定义 |
| A202 | 节点路径错误 | 节点路径不符合 AUTOSAR 定义，按图中提示修改 |
| A207 | 缺少最大/最小值约束 | 补充缺失的最大最小约束 |

## 临界区约定（Vector MCAL）

- mcal 命名规则：`SchM_Enter_Mcu_MCU_EXCLUSIVE_AREA_00`、`SchM_Exit_Mcu_MCU_EXCLUSIVE_AREA_00`
  - `Mcu` 表示模块名，`00` 表示第几个
- Vector 具体实现这些函数，mcal 只管调用；测试阶段或不带 VectorOS 时可做成桩函数
- 需 include `"SchM_Module.h"`，`SchM_Module.h/c` 放在 interface 目录下
- 使用时机：
  - 初始化函数连续配置寄存器时，期望不被打断
  - 往全局变量写数据时，防止数据一致性问题
  - 获取寄存器配置或状态时
- 注意：临界区代码执行时间要足够短，否则影响中断响应

## 注意事项

1. 转换前先备份原 XDM（`_Epd.xdm` 与业务 `.xdm` 初始内容相同，转换修改只动 `_Epd` 副本）
2. resource 文件在 `Resource_TS_TTAMM1I0P0\resource\` 下，按 `Module.Key:value` 格式定义，逗号分隔
3. `RELEASE` 版本（`asc:4.4.0`）必须与 XDM 中 `<a:a name="RELEASE" value="asc:4.4"/>` 对应
4. 每种芯片封装各需一份 EPD，按对应 resource 文件的数值分别生成
5. `-DValidate=false -DMapOptionalAsList=false` 是转换必需参数