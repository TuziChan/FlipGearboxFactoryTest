# 设备底层功能实现完整性审查报告

**审查日期：** 2026-04-17
**审查范围：** `src/infrastructure/devices/` 所有设备实现 + ProactiveListener + Bus 层支持
**审查依据：**
- 设备接口定义（`IMotorDriveDevice.h` / `ITorqueSensorDevice.h` / `IEncoderDevice.h` / `IBrakePowerDevice.h`）
- 硬件手册修正文档（`Docs/superpowers/specs/2026-04-17-device-registers-correction.md`）
- 开发约束（角色定义中的 12 项约束）

---

## 一、AQMD3610NS-A2 电机驱动器（`AqmdMotorDriveDevice`）

### 1.1 接口方法实现清单

| 接口方法 | 实现状态 | 说明 |
|---------|---------|------|
| `initialize()` | ✅ 已实现 | 读取设备 ID 验证通信，配置 AI1 为输入模式 |
| `setMotor(Direction, dutyCyclePercent)` | ✅ 已实现 | 占空比限幅 [0, 100]，转换为有符号 int16 ×10 写入 0x0040 |
| `brake()` | ✅ 已实现 | 调用 `setMotor(Direction::Brake, 0.0)`，等价于写入 0x0040 = 0 |
| `coast()` | ✅ 已实现 | 写入 0x0044 = 1（自然停止/释放自锁） |
| `readCurrent(double&)` | ✅ 已实现 | 读 0x0011，缩放 ×0.01A |
| `readAI1Level(bool&)` | ✅ 已实现 | 读 0x0052，0 = 低电平（磁铁命中） |
| `lastError()` | ✅ 已实现 | |

### 1.2 额外实现方法

| 方法 | 实现状态 | 说明 |
|------|---------|------|
| `readDeviceIdentification()` | ✅ 已实现 | 使用 0x2B 功能码读取厂商/产品/版本 |
| `writeMultipleRegisters()` | ✅ 已实现 | 使用 0x10 功能码批量写寄存器，含 EEPROM 触发提示 |

### 1.3 寄存器映射核查

| 寄存器定义 | 代码地址 | 文档地址 | 状态 |
|-----------|---------|---------|------|
| `REG_DEVICE_ID` | `0x0000` | 未明确列出 | ⚠️ 文档未提及，但通常为设备 ID 寄存器 |
| `REG_REAL_TIME_CURRENT` | `0x0011` | `0x0011` | ✅ 一致 |
| `REG_SET_SPEED` | `0x0040` | `0x0040` | ✅ 一致 |
| `REG_STOP_AND_LOCK` | `0x0042` | `0x0042` | ⚠️ 定义但未在代码中使用 |
| `REG_NATURAL_STOP` | `0x0044` | `0x0044` | ✅ 一致 |
| `REG_AI1_PORT_DIRECTION` | `0x0050` | `0x0050` | ✅ 一致 |
| `REG_AI1_PORT_LEVEL` | `0x0052` | `0x0052` | ✅ 一致 |

### 1.4 发现的问题

| 级别 | 问题描述 |
|------|---------|
| ⚠️ 次要 | `REG_STOP_AND_LOCK`（0x0042）在头文件中定义，但 `brake()` 未使用该寄存器，而是通过 0x0040 = 0 实现刹车。功能上等价，但如需“自锁”物理特性，需补充。 |
| ⚠️ 次要 | 开发约束要求 AQMD 使用 **Even parity**，但修正文档推荐 **无校验 + 1 停止位**。串口配置由上层调用 `IBusController::open()` 时传入，代码本身无此配置逻辑，但需确保上层调用时参数正确。 |

---

## 二、DYN200 扭矩传感器（`Dyn200TorqueSensorDevice`）

### 2.1 接口方法实现清单

| 接口方法 | 实现状态 | 说明 |
|---------|---------|------|
| `initialize()` | ✅ 已实现 | Mode 0 验证通信；Mode 1/2/3 切换 proactive 模式并启动监听器 |
| `readTorque(double&)` | ✅ 已实现 | Proactive 模式读缓存；Modbus 模式读 0x0000-0x0001，×0.01 N·m |
| `readSpeed(double&)` | ✅ 已实现 | Proactive 模式读缓存（ASCII 模式返回 false）；Modbus 模式读 0x0002-0x0003，×1 RPM |
| `readPower(double&)` | ✅ 已实现 | Proactive 模式返回 false；Modbus 模式读 0x0004-0x0005，×0.1 W |
| `readAll(double&, double&, double&)` | ✅ 已实现 | Modbus 模式一次读 6 个寄存器，效率优化 |
| `lastError()` | ✅ 已实现 | |

### 2.2 主动上传模式支持

| 模式 | 代码值 | 支持状态 | 说明 |
|------|--------|---------|------|
| Mode 0 (Modbus RTU) | `0` | ✅ | 默认轮询模式 |
| Mode 1 (HEX 6-byte) | `0` | ✅ | 通过 `Dyn200ProactiveListener::Hex6Byte` |
| Mode 2 (HEX 8-byte) | `3` | ✅ | 通过 `Dyn200ProactiveListener::Hex8Byte` |
| Mode 3 (ASCII) | `2` | ✅ | 通过 `Dyn200ProactiveListener::Ascii` |

### 2.3 寄存器映射核查

| 寄存器定义 | 代码地址 | 文档地址 | 状态 |
|-----------|---------|---------|------|
| `REG_TORQUE` | `0x0000` | `0x0000` | ✅ 一致，2 寄存器 |
| `REG_SPEED` | `0x0002` | `0x0002` | ✅ 一致，2 寄存器 |
| `REG_POWER` | `0x0004` | `0x0004` | ✅ 一致，2 寄存器 |
| `REG_COMM_MODE` | `0x001C` | `0x001C` | ✅ 一致 |

### 2.4 32 位大端处理

`combineToInt32(uint16_t highWord, uint16_t lowWord)` 正确实现大端拼接：
```cpp
uint32_t combined = (static_cast<uint32_t>(highWord) << 16) | static_cast<uint32_t>(lowWord);
return static_cast<int32_t>(combined);
```
✅ **符合约束 7（DYN200 32 位大端）**

### 2.5 发现的问题

| 级别 | 问题描述 |
|------|---------|
| ℹ️ 提示 | 硬件手册修正文档提到 **零点校准寄存器 0x0010**，但 `ITorqueSensorDevice` 接口未定义 `tare()` / `zeroCalibrate()` 方法，因此未实现。如需零点校准功能，需在接口层扩展。 |
| ⚠️ 次要 | 从 proactive 模式切回 Modbus RTU 模式需要 **手动硬件复位**（传感器按钮），代码中已添加警告日志。这是硬件限制，非代码缺陷。 |

---

## 三、单圈绝对值编码器（`SingleTurnEncoderDevice`）

### 3.1 接口方法实现清单

| 接口方法 | 实现状态 | 说明 |
|---------|---------|------|
| `initialize()` | ✅ 已实现 | Mode 0 验证通信；Mode 2/3/4 配置自动上报并启动监听器 |
| `readAngle(double&)` | ✅ 已实现 | Query 模式读 0x0000；Proactive 模式读缓存 |
| `readVirtualMultiTurn(double&)` | ✅ 已实现 | 读 0x0000-0x0001 两个寄存器，32 位拼接 |
| `readAngularVelocity(double&)` | ✅ 已实现 | 读 0x0003 有符号 16 位速度值 |
| `setZeroPoint()` | ✅ 已实现 | 写 0x0008 = 1 |
| `lastError()` | ✅ 已实现 | |

### 3.2 额外实现方法

| 方法 | 实现状态 | 说明 |
|------|---------|------|
| `setAutoReportMode(uint16_t, int)` | ✅ 已实现 | 支持运行时切换查询/上报模式，动态启停 `EncoderProactiveListener` |

### 3.3 寄存器映射核查

| 寄存器定义 | 代码地址 | 文档地址 | 状态 |
|-----------|---------|---------|------|
| `REG_ANGLE` | `0x0000` | `0x0000` | ✅ 一致 |
| `REG_VIRTUAL_MULTITURN` | `0x0000` | 未明确列出 | ⚠️ 与单圈共用地址，但读 2 个寄存器获取 32 位值，文档未提及此功能 |
| `REG_ANGULAR_VELOCITY` | `0x0003` | **未提及** | 🔴 **文档缺失** — 修正文档中无此寄存器 |
| `REG_AUTO_REPORT_MODE` | `0x0006` | **未提及** | 🔴 **文档缺失** — 修正文档中无此寄存器 |
| `REG_AUTO_REPORT_INTERVAL` | `0x0007` | **未提及** | 🔴 **文档缺失** — 修正文档中无此寄存器 |
| `REG_SET_ZERO` | `0x0008` | `0x0008` | ✅ 一致 |

### 3.4 角度换算

代码实现：`angleDeg = (count / m_resolution) * 360.0`
✅ **符合约束 12**

### 3.5 发现的问题

| 级别 | 问题描述 |
|------|---------|
| 🔴 **重要** | **寄存器 0x0003（角速度）、0x0006（自动上报模式）、0x0007（自动上报间隔）在修正文档中完全未提及**。这些地址可能来自原始硬件手册，但未经修正文档确认。如果实际硬件与文档不符，将导致通信失败。 |
| 🔴 **重要** | `EncoderProactiveListener` 帧解析存在逻辑缺陷（详见第 6 节）。 |

---

## 四、双通道数控电源（`BrakePowerSupplyDevice`）

### 4.1 接口方法实现清单

| 接口方法 | 实现状态 | 说明 |
|---------|---------|------|
| `initialize()` | ✅ 已实现 | 关闭双通道输出，读取模式寄存器并警告非 CC 模式 |
| `setCurrent(int, double)` | ✅ 已实现 | 限幅 [0, 5A]，写保持寄存器，缩放 ×0.01A |
| `setOutputEnable(int, bool)` | ✅ 已实现 | 写线圈（功能码 0x05） |
| `readCurrent(int, double&)` | ✅ 已实现 | 读输入寄存器（功能码 0x04），缩放 ×0.01A |
| `setVoltage(int, double)` | ✅ 已实现 | 限幅 [0, 24V]，写保持寄存器，缩放 ×0.01V |
| `readVoltage(int, double&)` | ✅ 已实现 | 读输入寄存器，缩放 ×0.01V |
| `readPower(int, double&)` | ✅ 已实现 | 读输入寄存器，缩放 ×0.01W |
| `readMode(int, int&)` | ✅ 已实现 | 读 0x0009，0=CC, 1=CV |
| `setBrakeMode(int, const QString&)` | ✅ 已实现 | 检查硬件模式与请求是否匹配，发出警告但不修改硬件（SW5 开关控制） |
| `lastError()` | ✅ 已实现 | |

### 4.2 寄存器映射核查

#### 保持寄存器（功能码 0x03/0x06）

| 寄存器定义 | 代码地址 | 文档地址 | 状态 |
|-----------|---------|---------|------|
| `REG_CH1_SET_VOLTAGE` | `0x0000` | `0x0000` | ✅ 一致 |
| `REG_CH1_SET_CURRENT` | `0x0001` | `0x0001` | ✅ 一致 |
| `REG_CH2_SET_VOLTAGE` | `0x0002` | `0x0002` | ✅ 一致 |
| `REG_CH2_SET_CURRENT` | `0x0003` | `0x0003` | ✅ 一致 |

#### 输入寄存器（功能码 0x04）

| 寄存器定义 | 代码地址 | 文档地址 | 状态 |
|-----------|---------|---------|------|
| `REG_CH1_READ_VOLTAGE` | `0x0000` | `0x0000` | ✅ 一致 |
| `REG_CH1_READ_CURRENT` | `0x0001` | `0x0001` | ✅ 一致 |
| `REG_CH1_READ_POWER` | `0x0002` | `0x0002` | ✅ 一致 |
| `REG_CH2_READ_VOLTAGE` | `0x0004` | `0x0004` | ✅ 一致（注释说明修复了旧错误 0x0003） |
| `REG_CH2_READ_CURRENT` | `0x0005` | `0x0005` | ✅ 一致 |
| `REG_CH2_READ_POWER` | `0x0006` | `0x0006` | ✅ 一致 |
| `REG_MODE` | `0x0009` | `0x0009` | ✅ 一致 |

#### 线圈（功能码 0x01/0x05）

| 寄存器定义 | 代码地址 | 文档地址 | 状态 |
|-----------|---------|---------|------|
| `COIL_CH1_OUTPUT_ENABLE` | `0x0000` | `0x0000` | ✅ 一致 |
| `COIL_CH2_OUTPUT_ENABLE` | `0x0001` | `0x0001` | ✅ 一致 |

### 4.3 安全限幅

- 电流上限：`MAX_CURRENT_A = 5.0` ✅ **符合约束 10**
- 电压上限：`MAX_VOLTAGE_V = 24.0` ✅ **符合约束 10**

### 4.4 重试机制

所有写操作（线圈、保持寄存器）和读操作（输入寄存器、保持寄存器）均实现了 `MAX_RETRIES = 3` 的重试逻辑，带 50ms 退避。✅

### 4.5 发现的问题

| 级别 | 问题描述 |
|------|---------|
| ℹ️ 提示 | `setBrakeMode()` 不实际修改硬件模式（因模式由 SW5 开关控制），仅做软件逻辑校验并警告。行为正确，但上层需确保 SW5 拨码与期望模式一致。 |

---

## 五、主动监听器（ProactiveListener）

### 5.1 Dyn200ProactiveListener

| 功能 | 状态 | 说明 |
|------|------|------|
| Hex6Byte 解析 | ✅ | CRC 校验、扭矩符号位处理、速度解析 |
| Hex8Byte 解析 | ✅ | 3 字节有符号扭矩、3 字节无符号速度、CRC 校验 |
| ASCII 解析 | ✅ | `[+-]X.XXXX\r` 格式、防缓冲区溢出（保留最后 32 字节） |
| 信号发射 | ✅ | `dataReceived(torque, speed, speedValid)` |
| 生命周期管理 | ✅ | `start()`/`stop()` 正确连接/断开 `readyRead` 信号 |

**结论：** `Dyn200ProactiveListener` 实现完整、健壮，无明显缺陷。✅

### 5.2 EncoderProactiveListener

| 功能 | 状态 | 说明 |
|------|------|------|
| 生命周期管理 | ✅ | `start()`/`stop()` 正确 |
| 线程安全 | ⚠️ | 使用 `std::atomic<double>` 和 `std::atomic<bool>`，但 `QSerialPort` 跨线程信号槽需确保串口线程安全 |
| 帧解析 | 🔴 | 存在逻辑缺陷（见下） |

#### 🔴 关键缺陷

**文件：** `src/infrastructure/devices/EncoderProactiveListener.cpp`

```cpp
while (m_buffer.size() >= 4) {
    // Parse angle from first 2 bytes
    uint16_t rawAngle = ...;
    double angleDeg = (static_cast<double>(rawAngle) / 65535.0) * 360.0;
    // ...
    m_buffer.remove(0, 2);  // 只移除 2 字节
    break;                  // 每次只处理一帧
}
```

| 问题 | 影响 |
|------|------|
| **1. 帧长度不一致** | 循环条件是 `>= 4` 字节，但只读取并移除 2 字节。如果编码器主动上报帧确实是 4 字节（2 字节数据 + 2 字节 CRC/校验），则每次处理会残留 2 字节，导致后续帧错位。 |
| **2. 满量程假设错误** | 转换公式使用 `65535.0` 作为满量程。但编码器分辨率默认为 **4096**，如果主动上报发送的是原始计数值（0-4095），则正确公式应为 `(rawAngle / 4096.0) * 360.0`。修正文档未描述主动上报帧格式，无法确认。 |
| **3. 模式覆盖不全** | 编码器支持 Mode 2（单圈）、Mode 3（虚拟多圈）、Mode 4（角速度）三种自动上报模式，但 `EncoderProactiveListener` 仅处理单圈角度，未处理 32 位多圈值和角速度值。 |

**建议修复：**
1. 确认编码器主动上报的实际帧格式（字节数、字节序、是否含 CRC）。
2. 若帧为 4 字节（含 CRC），应 `remove(0, 4)` 并去掉多余的 `break`。
3. 若原始值范围是 0-4095，使用 `m_resolution` 而非 65535 进行角度转换。
4. 扩展监听器以支持 Mode 3（32 位多圈）和 Mode 4（角速度）的帧解析。

---

## 六、Bus 层支持检查

### 6.1 ModbusFrame 功能码覆盖

| 功能码 | 名称 | 构建 | 解析 | 设备使用 |
|--------|------|------|------|---------|
| 0x01 | Read Coils | ✅ | ✅ | BrakePowerSupplyDevice |
| 0x02 | Read Discrete Inputs | ❌ 未实现 | ❌ 未实现 | 当前未使用 |
| 0x03 | Read Holding Registers | ✅ | ✅ | 所有设备 |
| 0x04 | Read Input Registers | ✅ | ✅ | BrakePowerSupplyDevice |
| 0x05 | Write Single Coil | ✅ | ✅ | BrakePowerSupplyDevice |
| 0x06 | Write Single Register | ✅ | ✅ | 所有设备 |
| 0x10 | Write Multiple Registers | ✅ | ✅ | AqmdMotorDriveDevice |
| 0x2B | Read Device Identification | ✅ | ✅ | AqmdMotorDriveDevice |

**结论：** 功能码 **0x02（Read Discrete Inputs）未实现**，但当前 4 台设备均不需要此功能码。如需未来扩展支持离散输入设备，需补充实现。

### 6.2 ModbusRtuBusController

| 功能 | 状态 | 说明 |
|------|------|------|
| 串口打开/关闭 | ✅ | 支持波特率、校验位、停止位配置 |
| 同步请求/响应 | ✅ | `sendRequest()` 带 `waitForResponse()` 超时 |
| 超时处理 | ✅ | `m_timeoutMs` 可配置，默认 500ms |
| 帧间延迟 | ✅ | 自动计算 T3.5（波特率 ≥19200 时为 2ms） |
| 重试机制 | ⚠️ | **Bus 层本身无重试**，重试逻辑在 `BrakePowerSupplyDevice` 中实现。建议统一移至 Bus 层 |
| `underlyingSerialPort()` | ✅ | `ModbusRtuBusController` 正确覆盖，返回 `m_serialPort` |

---

## 七、开发约束符合性总览

| 约束 | 内容 | 符合性 |
|------|------|--------|
| 1 | 所有设备操作返回 bool + lastError()，禁止抛异常 | ✅ |
| 2 | 串口操作必须带超时，禁止无限等待 | ✅（Bus 层实现） |
| 3 | 设备接口已定义，只实现接口方法 | ✅ |
| 4 | 寄存器映射与硬件手册一致 | ⚠️ 编码器 0x0003/0x0006/0x0007 未在修正文档中确认 |
| 5 | 数据缩放因子标注来源 | ✅（代码中有注释说明） |
| 6 | AQMD: Even parity, 9600 baud, slave 1 | ⚠️ 代码无串口配置逻辑，由上层传入；但文档推荐“无校验”与约束矛盾 |
| 7 | DYN200: None parity, 19200 baud, 2 stop bits, slave 2, 32 位大端 | ⚠️ 同上；32 位大端处理 ✅ |
| 8 | Encoder: None parity, 9600 baud, slave 3, 分辨率 4096 | ⚠️ 同上；分辨率默认 4096 ✅ |
| 9 | Brake: None parity, 9600 baud, slave 4, 支持 CC/CV 模式 | ⚠️ 同上；CC/CV 模式支持 ✅ |
| 10 | 安全限幅：制动电流 ≤5A，电压 ≤24V | ✅ |
| 11 | DYN200 32 位数据需两次 16 位寄存器读取拼接 | ✅ |
| 12 | 编码器角度 = (count / resolution) × 360° | ✅ |

---

## 八、总体结论

### 8.1 功能实现完整性评估

| 设备 | 接口覆盖率 | 功能完整性 | 主要风险 |
|------|-----------|-----------|---------|
| AQMD 电机驱动器 | 100% (6/6) | 高 | 低 |
| DYN200 扭矩传感器 | 100% (6/6) | 高 | 低 |
| 单圈编码器 | 100% (6/6) | 中 | **EncoderProactiveListener 帧解析缺陷 + 未确认寄存器** |
| 数控电源 | 100% (10/10) | 高 | 低 |

### 8.2 必须修复的问题（P0）

1. **🔴 `EncoderProactiveListener` 帧解析逻辑错误**
   - `m_buffer.remove(0, 2)` 与 `>= 4` 条件不匹配，可能导致帧错位。
   - 满量程 `65535.0` 与编码器分辨率 `4096` 的关系需确认。
   - **影响：** 主动上传模式下角度数据可能完全错误。

2. **🔴 编码器未确认寄存器地址**
   - `0x0003`（角速度）、`0x0006`（自动上报模式）、`0x0007`（自动上报间隔）在修正文档中未提及。
   - **影响：** 如果实际硬件寄存器映射与代码不符，通信失败。

### 8.3 建议修复的问题（P1）

3. **⚠️ `ModbusFrame` 缺少 0x02 功能码**
   - 当前未影响功能，但开发约束要求支持 0x01-0x06。

4. **⚠️ 串口配置分散在各设备外部**
   - 校验位、波特率、停止位等由上层调用 `IBusController::open()` 时传入，代码中没有集中管理各设备通信参数的工厂或配置逻辑。建议增加 `DeviceBusFactory` 确保参数与约束一致。

5. **⚠️ `Dyn200TorqueSensorDevice` 缺少零点校准接口**
   - 硬件支持（寄存器 0x0010），但接口未定义。如需此功能，需扩展 `ITorqueSensorDevice`。

### 8.4 无需修复的观察项

- `AQMD::brake()` 未使用 `0x0042` 寄存器：功能上等价，非缺陷。
- `DYN200` 从 proactive 模式切回 Modbus 需手动复位：硬件限制，代码已充分警告。
- `BrakePowerSupplyDevice::setBrakeMode()` 不改变硬件模式：行为正确，模式由 SW5 开关控制。
