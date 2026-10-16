# AM Dio 案例：证据与边界

本案例记录2026-09-30用户环境反馈，不是跨模块固定模板。两份原始参考保持不变。

## 确认的语法示例

用户提供的示例仅用于说明语法，不是行为正确的测试；以下值为说明形式。

```text
TEST.STUB:Dio_Hw.Dio_Hw_ReadChannel
TEST.VALUE:Dio_Hw.Dio_Hw_ReadChannel.Level[0]:1
TEST.VALUE:uut_prototype_stubs.Mcu_Clk_StatusModules.Status[0]:true
TEST.VALUE:uut_prototype_stubs.Mcu_Clk_StatusModules.return:0
TEST.VALUE:uut_prototype_stubs.Base_GetCoreId.return:0
TEST.VALUE:Dio.<<GLOBAL>>.TTDet_ReportErrorId:238
```

函数属于Dio_Hw，不因被Dio调用而变成Dio所属函数。其他模块重新核对Unit/桩映射。

## 实际纠错

- 用户链接日志证实工具生成TTDet_ReportErrorId存储；ENV人工定义导致multiple definition。移除人工定义而非改为static，保留头文件extern。
- 当前环境MFG为manage-config-options XML，CFG为C_DEFINE_LIST/C_EDG_FLAGS选项。不能按早先错误描述交换职责，也不能向未知版本外推。
- 源码vendorID为214；端口错误宏DIO_E_PARAM_INVALID_PORT_ID为0x14（十进制20），不能写成十进制14。用户早先报告40与本地源码不同，未取得该差异的独立根因证据。
- NULL版本信息应有指针NULL预期及DIO_E_PARAM_POINTER错误断言；私有参数检查的RetVal NULL路径单独构造。
- 同场景跨配置复用编号；通知机制编译关闭与运行时NULL回调属于不同控制流场景。
- Config2无其他观察点的20个用例按用户要求初始化错误记录0并断言0；仅证明错误记录未变。

## 验证状态

用户反馈“现在运行没什么问题了”。未取得完整执行/覆盖率报告，不据此宣称所有错误路径、所有配置或覆盖目标已经达标。后续新增的全错误码独立测试规则仍须按具体模块清单核验，不能倒推现有用例已经满足。
