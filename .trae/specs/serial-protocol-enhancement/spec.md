# 串口通信协议完善 Spec — 覆盖全部设备手册协议

## Why
当前串口通信层存在四个结构性缺陷：
1. **ModbusFrame 缺功能码**：缺 0x10 批量写寄存器（AQMD GPIO 配置/EEPROM 存储、DYN200 参数修改）、0x2B 读设备识别码（AQMD），且异常响应未解析，通信错误无法区分根因
2. **DYN200 四种通信模式仅实现一种**：仅实现了 Modbus RTU 模式（通信模式 1，30 次/秒），遗漏了 HEX 6 字节主动上传（模式 0，1000 次/秒）、HEX 8 字节主动上传（模式 3，1000 次/秒）、ASCII 主动上传（模式 2，1000 次/秒）
3. **编码器四种模式仅实现一种**：仅实现了 Modbus RTU 查询模式（模式 0x00），遗漏了自动回传单圈值（0x01）、自动回传虚拟多圈值（0x04）、自动回传角速度值（0x05），且虚拟多圈值、角速度值等寄存器未实现读取
4. **串口参数硬编码**：校验位、停止位、波特率硬编码在 StationConfig 中，无法运行时切换适配不同设备

## What Changes
- **ModbusFrame**: 补全 0x10 Write Multiple Registers（AQMD/DYN200/编码器都需要）、0x2B Read Device Identification（AQMD）；新增 Modbus 异常响应解析函数
- **ModbusRtuBusController**: 新增串口参数运行时重配置方法
- **DYN200 全部协议**: 新增 `Dyn200ProactiveListener` 类，统一支持 HEX 6 字节主动上传（通信模式 0）、HEX 8 字节主动上传（通信模式 3）、ASCII 主动上传（通信模式 2）；Dyn200TorqueSensorDevice 支持四种通信模式切换
- **编码器全部模式**: `SingleTurnEncoderDevice` 支持查询模式（0x00）、自动回传单圈值（0x01）、自动回传虚拟多圈值（0x04）、自动回传角速度值（0x05）；新增虚拟多圈值和角速度值读取接口
- **AQMD 0x2B 支持**: `AqmdMotorDriveDevice` 新增 `readDeviceIdentification()` 方法
- **DeviceConfig 扩展**: 新增 `communicationMode` 字段，由 JSON 配置文件控制
- **ModbusFrame CRC 字节序兼容**: 新增可选大端 CRC 模式

## Impact
- Affected code: ModbusFrame.h/.cpp, ModbusRtuBusController.h/.cpp, Dyn200TorqueSensorDevice.h/.cpp, SingleTurnEncoderDevice.h/.cpp, IEncoderDevice.h, AqmdMotorDriveDevice.h/.cpp, StationConfig.h, DeviceConfigService.cpp, StationRuntimeFactory.cpp, AcquisitionScheduler.h/.cpp
- New files: Dyn200ProactiveListener.h/.cpp (DYN200 全协议主动上传解析器)
- Affected behavior: DYN200 四种通信模式可切换、编码器四种模式可切换、AQMD 支持设备识别码读取和 0x10 批量写
- **BREAKING**: `DeviceConfig` 新增字段；`IEncoderDevice`/`SingleTurnEncoderDevice` 接口扩展（新增虚拟多圈/角速度读取）

---

## ADDED Requirements

### Requirement: Modbus 0x10 Write Multiple Registers 帧构建与解析
系统 SHALL 提供 `ModbusFrame::buildWriteMultipleRegisters(slaveId, startAddress, values)` 构建功能码 0x10 请求帧，以及 `parseWriteMultipleRegistersResponse(response, expectedAddress, expectedCount)` 解析从站响应。

#### Scenario: AQMD 批量写 4 个 GPIO 寄存器
- **WHEN** 调用 `buildWriteMultipleRegisters(0x01, 0x0050, {0, 1, 0, 1})` 写入 AQMD AI1/AI2 GPIO 配置
- **THEN** 生成帧：`01 10 00 50 00 04 08 00 00 00 01 00 00 00 01 [CRC_L] [CRC_H]`，从站应回送 `01 10 00 50 00 04 [CRC_L] [CRC_H]`

#### Scenario: DYN200 修改滤波参数
- **WHEN** 调用 `buildWriteMultipleRegisters(0x02, 0x0006, {0x00, 0x00, 0x00, 0x14})` 修改滤波值
- **THEN** 正确构建 0x10 帧写入 DYN200 参数寄存器

#### Scenario: 解析正确的 0x10 响应
- **WHEN** 收到 8 字节响应帧，功能码 0x10，地址和数量匹配
- **THEN** 返回 true

#### Scenario: 响应功能码异常
- **WHEN** 收到功能码为 0x90 的异常响应
- **THEN** 返回 false，错误信息包含异常码描述

### Requirement: Modbus 0x2B Read Device Identification 帧构建与解析
系统 SHALL 提供 `ModbusFrame::buildReadDeviceIdentification(slaveId)` 构建功能码 0x2B（MEI 类型 0x0E）请求帧，以及 `parseReadDeviceIdentificationResponse(response, outVendor, outProduct, outVersion)` 解析响应。

#### Scenario: 读取 AQMD 设备识别码
- **WHEN** 向 AQMD 发送 0x2B 请求
- **THEN** 返回设备厂商、产品名称、版本号等字符串信息

### Requirement: Modbus 异常响应解析
系统 SHALL 提供 `ModbusFrame::parseExceptionResponse(response)` 解析 Modbus 异常帧（功能码最高位为 1 + 1 字节异常码），返回异常码值和描述字符串对。所有现有的 `parse*Response` 函数 SHALL 在检测到异常帧时设置可区分的错误信息。

#### Scenario: 设备返回非法数据地址
- **WHEN** 设备回送 `01 83 02 [CRC]`
- **THEN** 错误信息为 "Modbus exception: Illegal Data Address (0x02)"

#### Scenario: 设备忙
- **WHEN** 设备返回异常码 0x06
- **THEN** 错误信息包含 "Slave Device Busy"

#### Scenario: AQMD 扩展异常码
- **WHEN** AQMD 返回异常码 0x40（禁止操作）或 0xFF（未定义错误）
- **THEN** 错误信息包含对应的扩展异常码描述

### Requirement: DYN200 HEX 6 字节主动上传协议支持（通信模式 0）
系统 SHALL 在 `Dyn200ProactiveListener` 中实现 HEX 6 字节帧解析：D1,D2=扭矩值（uint16，最大 65535），D3,D4=转速值（int16，D3 最高位表示扭矩正负号），D5,D6=CRC16。

#### Scenario: 正常扭矩和转速
- **WHEN** 串口收到 6 字节 `[0x00, 0x64, 0x00, 0x1E, CRC_L, CRC_H]`（扭矩=100，转速=30）
- **THEN** 发射 `dataReceived(1.00, 30.0)`

#### Scenario: 负扭矩
- **WHEN** D3 最高位为 1（扭矩为负），D1,D2 为 `[0x00, 0x0A]`
- **THEN** 扭矩值为 -0.10 N·m

#### Scenario: CRC 校验失败
- **WHEN** 收到 6 字节但 CRC 不匹配
- **THEN** 丢弃该帧，发射 `errorOccurred` 信号

#### Scenario: 帧碎片拼接
- **WHEN** `readyRead` 分多次触发
- **THEN** 正确拼接为完整 6 字节帧后解析

### Requirement: DYN200 HEX 8 字节主动上传协议支持（通信模式 3）
系统 SHALL 在 `Dyn200ProactiveListener` 中实现 HEX 8 字节帧解析：D1,D2,D3=扭矩值（3 字节有符号整数，D1 最高位为 1 时为负数，最大 0x7FFFFF），D4,D5,D6=转速值（3 字节无符号整数，最大 0xFFFFFF），D7,D8=CRC16。用于扭矩超过 65535 或转速超过 32767 的场景。

#### Scenario: HEX 8 正常数据
- **WHEN** 收到 8 字节 `[0x00, 0x01, 0x00, 0x00, 0x00, 0x1E, CRC_L, CRC_H]`（扭矩=256，转速=30）
- **THEN** 发射 `dataReceived(2.56, 30.0)`

#### Scenario: HEX 8 负扭矩
- **WHEN** 收到 `[0xFF, 0xFF, 0xFE, 0x00, 0x00, 0x1E, CRC_L, CRC_H]`（D1 最高位为 1，扭矩=-2）
- **THEN** 扭矩值为 -0.02 N·m

#### Scenario: HEX 8 高转速值
- **WHEN** 转速超过 32767
- **THEN** 正确解析 3 字节转速值

### Requirement: DYN200 ASCII 主动上传协议支持（通信模式 2）
系统 SHALL 在 `Dyn200ProactiveListener` 中实现 ASCII 主动上传帧解析：以 `\r`（0x0D）结尾的 ASCII 字符串，格式为 `[+-]X.XXXX\r`（如 `+0.0001\r`），仅包含扭矩数据。

#### Scenario: ASCII 正扭矩
- **WHEN** 收到 `+8.8814\r`
- **THEN** 发射 `dataReceived(8.8814, NaN)`（转速不可用）

#### Scenario: ASCII 负扭矩
- **WHEN** 收到 `-0.0013\r`
- **THEN** 扭矩值为 -0.0013 N·m，转速不可用

#### Scenario: ASCII 帧碎片
- **WHEN** 分多次收到 `+8.88` 和 `14\r`
- **THEN** 拼接后正确解析

### Requirement: DYN200 四种通信模式统一切换
系统 SHALL 允许 `Dyn200TorqueSensorDevice` 在四种通信模式间切换，由 `DeviceConfig::communicationMode` 控制：
- `0`：Modbus RTU 主从轮询（默认，寄存器 0x1CH = 1）— 支持 03H/05H/10H，30 次/秒
- `1`：HEX 6 字节主动上传（寄存器 0x1CH = 0）— 最高 1000 次/秒
- `2`：HEX 8 字节主动上传（寄存器 0x1CH = 3）— 最高 1000 次/秒
- `3`：ASCII 主动上传（寄存器 0x1CH = 2）— 最高 1000 次/秒，仅扭矩

#### Scenario: 配置为 Modbus RTU 模式（默认）
- **WHEN** `communicationMode = 0`
- **THEN** 使用现有轮询机制，`readAll()` 获取扭矩/转速/功率

#### Scenario: 配置为 HEX 6 字节主动上传
- **WHEN** `communicationMode = 1`
- **THEN** `initialize()` 时向 DYN200 写入寄存器 0x1CH = 0；创建 `Dyn200ProactiveListener`（模式 0）；数据从 Listener 缓存读取

#### Scenario: 配置为 HEX 8 字节主动上传
- **WHEN** `communicationMode = 2`
- **THEN** `initialize()` 时向 DYN200 写入寄存器 0x1CH = 3；创建 `Dyn200ProactiveListener`（模式 3）

#### Scenario: 配置为 ASCII 主动上传
- **WHEN** `communicationMode = 3`
- **THEN** `initialize()` 时向 DYN200 写入寄存器 0x1CH = 2；创建 `Dyn200ProactiveListener`（模式 2）；转速不可用

#### Scenario: 从主动上传切回 Modbus RTU
- **WHEN** 用户试图切回 Modbus RTU
- **THEN** 代码和日志明确警告：**手册规定改为其他模式后不能通过通信修改为 Modbus RTU 模式**，需要用户手动通过传感器按键恢复出厂设置

### Requirement: 编码器四种模式支持
系统 SHALL 允许 `SingleTurnEncoderDevice` 支持编码器全部四种工作模式（寄存器 0x0006）：
- `0x00`：查询模式（默认，Modbus RTU 0x03/0x06）
- `0x01`：自动回传编码器单圈值
- `0x04`：自动回传编码器虚拟多圈值
- `0x05`：自动回传编码器角速度值

#### Scenario: 配置为查询模式（默认）
- **WHEN** `communicationMode = 0`
- **THEN** 使用现有轮询机制，`readAngle()` 通过 Modbus 0x03 读取

#### Scenario: 配置为自动回传单圈值
- **WHEN** `communicationMode = 2`
- **THEN** `initialize()` 时写入 0x0006 = 0x01 和 0x0007 = 回传周期；使用异步接收解析单圈计数值并换算角度

#### Scenario: 配置为自动回传虚拟多圈值
- **WHEN** `communicationMode = 3`
- **THEN** `initialize()` 时写入 0x0006 = 0x04 和 0x0007 = 回传周期；解析 2 寄存器（4 字节）虚拟多圈计数值

#### Scenario: 配置为自动回传角速度值
- **WHEN** `communicationMode = 4`
- **THEN** `initialize()` 时写入 0x0006 = 0x05 和 0x0007 = 回传周期；解析角速度有符号整数并换算 RPM

### Requirement: 编码器虚拟多圈值和角速度值读取
`IEncoderDevice` 和 `SingleTurnEncoderDevice` SHALL 新增以下读取接口：
- `readVirtualMultiTurn(double& totalAngleDeg)` — 读取虚拟多圈值（寄存器 0x0000~0x0001），换算为总角度
- `readAngularVelocity(double& velocityRpm)` — 读取角速度值（寄存器 0x0003），结合分辨率和采样时间换算为 RPM

#### Scenario: 读取虚拟多圈角度
- **WHEN** 调用 `readVirtualMultiTurn(angle)`
- **THEN** 读取 2 个寄存器（0x0000~0x0001），组合为 uint32，换算 `angle = value * 360.0 / resolution`

#### Scenario: 读取角速度
- **WHEN** 调用 `readAngularVelocity(rpm)`
- **THEN** 读取 1 个寄存器（0x0003），有符号 int16，换算 `rpm = value / resolution / (sampleTimeMs / 60000.0)`

### Requirement: AQMD 设备识别码读取
`AqmdMotorDriveDevice` SHALL 新增 `readDeviceIdentification(QString& vendor, QString& product, QString& version)` 方法，通过 0x2B 功能码读取设备信息。

#### Scenario: 读取 AQMD 设备信息
- **WHEN** 调用 `readDeviceIdentification()`
- **THEN** 返回设备厂商、产品名称、版本号

### Requirement: AQMD 批量写寄存器支持
`AqmdMotorDriveDevice` SHALL 新增 `writeMultipleRegisters(address, values)` 方法，使用 0x10 功能码批量写入。当需要写入 AQMD GPIO 配置（0x0050~0x0053，4 个寄存器）时，使用 0x10 可触发 EEPROM 存储。

#### Scenario: 批量写 GPIO 配置并存储
- **WHEN** 调用 `writeMultipleRegisters(0x0050, {0, 1, 0, 1})` 写入 4 个寄存器
- **THEN** AQMD 自动将 GPIO 配置保存到 EEPROM

### Requirement: 串口参数运行时重配置
系统 SHALL 提供 `ModbusRtuBusController::reconfigure(baudRate, parity, stopBits)` 方法。

#### Scenario: 运行时修改波特率
- **WHEN** 调用 `reconfigure(19200, "None", 1)`
- **THEN** 串口参数立即更新，interFrameDelay 重新计算

#### Scenario: 串口未打开时重配置
- **WHEN** 在 `open()` 之前调用 `reconfigure()`
- **THEN** 缓存参数供 `open()` 使用

### Requirement: ModbusFrame CRC 字节序兼容选项
系统 SHALL 支持可选的大端 CRC 字节序模式，用于适配编码器手册的 CRC 格式。默认使用标准 Modbus RTU（低字节在前）。

#### Scenario: 标准 CRC（默认）
- **THEN** CRC 字节顺序为低字节在前

#### Scenario: 大端 CRC
- **WHEN** 配置为大端 CRC
- **THEN** CRC 字节顺序为高字节在前

### Requirement: DeviceConfig 通信模式字段
`DeviceConfig` 结构体 SHALL 新增 `communicationMode` 字段（int 类型，默认 0）。

#### Scenario: DYN200 HEX 6 字节主动上传
- **WHEN** JSON `"communicationMode": 1`
- **THEN** DYN200 使用通信模式 0（HEX 6 字节主动上传）

#### Scenario: DYN200 HEX 8 字节主动上传
- **WHEN** JSON `"communicationMode": 2`
- **THEN** DYN200 使用通信模式 3（HEX 8 字节主动上传）

#### Scenario: DYN200 ASCII 主动上传
- **WHEN** JSON `"communicationMode": 3`
- **THEN** DYN200 使用通信模式 2（ASCII 主动上传）

#### Scenario: 编码器自动回传单圈值
- **WHEN** JSON `"communicationMode": 2`
- **THEN** 编码器使用模式 0x01（自动回传单圈值）

#### Scenario: 编码器自动回传虚拟多圈值
- **WHEN** JSON `"communicationMode": 3`
- **THEN** 编码器使用模式 0x04（自动回传虚拟多圈值）

#### Scenario: 编码器自动回传角速度值
- **WHEN** JSON `"communicationMode": 4`
- **THEN** 编码器使用模式 0x05（自动回传角速度值）

---

## MODIFIED Requirements

### Requirement: Dyn200TorqueSensorDevice 初始化流程
初始化时 SHALL 根据 `communicationMode` 字段决定通信策略：
- `0`：Modbus RTU 轮询（默认），验证通信正常
- `1`：先通过 Modbus RTU 写入寄存器 0x1CH = 0，切换到 HEX 6 字节主动上传，然后启动 Listener
- `2`：先通过 Modbus RTU 写入寄存器 0x1CH = 3，切换到 HEX 8 字节主动上传，然后启动 Listener
- `3`：先通过 Modbus RTU 写入寄存器 0x1CH = 2，切换到 ASCII 主动上传，然后启动 Listener

### Requirement: SingleTurnEncoderDevice 初始化流程
初始化时 SHALL 根据 `communicationMode` 字段决定通信策略：
- `0`：查询模式（默认），验证通信正常
- `2`：写入 0x0006 = 0x01 和 0x0007 = 回传周期，启动自动回传单圈值
- `3`：写入 0x0006 = 0x04 和 0x0007 = 回传周期，启动自动回传虚拟多圈值
- `4`：写入 0x0006 = 0x05 和 0x0007 = 回传周期，启动自动回传角速度值

### Requirement: StationConfig 默认值
`StationConfig` 构造函数中的默认配置 SHALL 与各设备手册一致：
- AQMD: `parity = "Even"`, `stopBits = 1`, `baudRate = 9600`（已正确）
- DYN200: `parity = "None"`, `stopBits = 2`, `baudRate = 19200`（手册默认 2 位停止位）
- 编码器: `parity = "None"`, `stopBits = 1`, `baudRate = 9600`
- 数控电源: `parity = "None"`, `stopBits = 1`, `baudRate = 9600`

---

## 协议覆盖完整矩阵

### DYN200 扭矩传感器

| 通信模式 | 名称 | 帧格式 | 速率 | 扭矩 | 转速 | 功率 | 状态 |
|----------|------|--------|------|------|------|------|------|
| 1 (默认) | Modbus RTU | 03H/05H/10H | 30次/秒 | ✅已实现 | ✅已实现 | ✅已实现 | 本次新增 10H |
| 0 | HEX 6 字节主动上传 | D1D2 D3D4 D5D6 | 1000次/秒 | 🔴未实现 | 🔴未实现 | ❌不支持 | 本次实现 |
| 3 | HEX 8 字节主动上传 | D1D2D3 D4D5D6 D7D8 | 1000次/秒 | 🔴未实现 | 🔴未实现 | ❌不支持 | 本次实现 |
| 2 | ASCII 主动上传 | `[+-]X.XXXX\r` | 1000次/秒 | 🔴未实现 | ❌不支持 | ❌不支持 | 本次实现 |

### 编码器

| 模式码 | 名称 | 数据类型 | 速率 | 状态 |
|--------|------|----------|------|------|
| 0x00 (默认) | 查询模式 | 0x03 读寄存器 | 轮询 | ✅已实现 |
| 0x01 | 自动回传单圈值 | uint16 计数值 | 最高 50Hz | 🔴未实现 → 本次实现 |
| 0x04 | 自动回传虚拟多圈值 | uint32 (2 寄存器) | 最高 50Hz | 🔴未实现 → 本次实现 |
| 0x05 | 自动回传角速度值 | int16 有符号 | 最高 50Hz | 🔴未实现 → 本次实现 |
| - | 虚拟多圈值读取 (0x0000~0x0001) | uint32 | 轮询 | 🔴未实现 → 本次实现 |
| - | 角速度值读取 (0x0003) | int16 | 轮询 | 🔴未实现 → 本次实现 |

### AQMD3610NS-A2 驱动器

| 功能码 | 名称 | 用途 | 状态 |
|--------|------|------|------|
| 0x03 | 读保持寄存器 | 实时状态读取 | ✅已实现 |
| 0x06 | 写单个寄存器 | 速度控制/GPIO | ✅已实现 |
| 0x10 | 写多个寄存器 | GPIO 批量配置+EEPROM | 🔴未实现 → 本次实现 |
| 0x2B | 读设备识别码 | 设备信息读取 | 🔴未实现 → 本次实现 |

### 数控电源

| 功能码/类型 | 名称 | 状态 |
|-------------|------|------|
| 0x03 读保持寄存器 | 电压/电流设定值 | ✅已实现 |
| 0x04 读输入寄存器 | 实际电压/电流/功率 | ✅已实现 |
| 0x05 写线圈 | 输出使能开关 | ✅已实现 |
| 0x06 写保持寄存器 | 电压/电流设定 | ✅已实现 |
| 0x01 读线圈 | 输出使能状态读取 | ✅已实现 |
