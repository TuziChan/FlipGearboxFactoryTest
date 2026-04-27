# 仿真与 Mock 设备使用指南

**最后更新**: 2026-04-24  
**适用版本**: FlipGearboxFactoryTest v0.1

本文档整合了 Mock 设备模拟器和仿真测试框架的使用说明，提供无硬件开发和测试的完整方案。

---

## 目录

1. [概述](#概述)
2. [Mock 设备模拟器](#mock-设备模拟器)
3. [仿真测试框架](#仿真测试框架)
4. [使用场景](#使用场景)
5. [故障注入](#故障注入)

---

## 概述

FlipGearboxFactoryTest 提供两套仿真机制：

| 机制 | 用途 | 特点 |
|------|------|------|
| **Mock 设备** | 替代真实硬件运行应用 | 完整 Modbus RTU 协议仿真，支持串口通信 |
| **仿真框架** | 自动化测试 | 确定性时间控制，事件触发，状态检查 |

### 架构对比

**Mock 设备架构**:
```
MockDeviceManager
    └── MockSerialBusController (多串口)
            ├── COM1 → MockMotorDevice (slave 1)
            ├── COM2 → MockTorqueDevice (slave 2)
            ├── COM3 → MockEncoderDevice (slave 3)
            └── COM4 → MockBrakeDevice (slave 4)
```

**仿真测试架构**:
```
SimulationTestRuntime
├── SimulationContext (共享状态)
├── SimulationTestHelper (控制与验证)
├── Simulated Devices (内存设备)
│   ├── SimulatedMotorDevice
│   ├── SimulatedTorqueDevice
│   ├── SimulatedEncoderDevice
│   └── SimulatedBrakeDevice
└── GearboxTestEngine
```

---

## Mock 设备模拟器

### 1. 核心组件

#### MockModbusDevice (基类)

- 实现完整 Modbus RTU 协议
- 支持功能码: 0x01, 0x03, 0x04, 0x05, 0x06, 0x10
- 提供错误注入机制 (CRC 错误、超时、延迟、异常响应)
- 管理寄存器存储 (Holding/Input Registers, Coils)

#### MockMotorDevice (电机驱动器)

**寄存器映射**:
- `0x0000`: 设备 ID (只读)
- `0x0011`: 实时电流 (×0.01A, 只读)
- `0x0040`: 设置速度 (-1000 to 1000, ×0.1%)
- `0x0042`: 停止并锁定 (写 1 制动)
- `0x0044`: 自然停止 (写 1 滑行)
- `0x0050`: AI1 端口方向 (0=输入, 1=输出)
- `0x0052`: AI1 端口电平 (0=低, 1=高)

**特性**:
- 模拟电流与速度成正比
- 支持磁铁检测信号 (AI1 输入)

#### MockTorqueDevice (扭矩传感器)

**寄存器映射 (32位大端)**:
- `0x0000-0x0001`: 扭矩 (×0.01 N·m)
- `0x0002-0x0003`: 转速 (×1 RPM)
- `0x0004-0x0005`: 功率 (×0.1 W)
- `0x001C`: 通信模式 (0=Modbus RTU)

**特性**:
- 32位数据大端存储
- 支持动态更新扭矩、转速、功率

#### MockEncoderDevice (编码器)

**寄存器映射**:
- `0x0000`: 单圈原始计数 (0 to resolution)
- `0x0000-0x0001`: 虚拟多圈计数 (32位)
- `0x0003`: 角速度 (有符号 16位 RPM)
- `0x0006`: 自动上报模式
- `0x0007`: 自动上报间隔 (ms)
- `0x0008`: 设置零点 (写 1)

**特性**:
- 分辨率可配置 (默认 4096)
- 角度转换: `angleDeg = (count / resolution) × 360°`
- 支持零点设置

#### MockBrakeDevice (制动器)

**Holding Registers (0x03/0x06)**:
- `0x0000`: CH1 设置电压 (×0.01V)
- `0x0001`: CH1 设置电流 (×0.01A)
- `0x0002`: CH2 设置电压 (×0.01V)
- `0x0003`: CH2 设置电流 (×0.01A)

**Input Registers (0x04)**:
- `0x0000`: CH1 实际电压 (×0.01V)
- `0x0001`: CH1 实际电流 (×0.01A)
- `0x0002`: CH1 实际功率 (×0.1W)
- `0x0003`: CH2 实际电压 (×0.01V)
- `0x0005`: CH2 实际电流 (×0.01A)
- `0x0006`: CH2 实际功率 (×0.1W)

**Coils (0x01/0x05)**:
- `0x0000`: CH1 输出使能
- `0x0001`: CH2 输出使能

### 2. 使用 Mock 设备运行应用

应用程序在串口不可用时自动回退到 Mock 模式：

```cpp
// main.cpp 中的自动回退逻辑
auto runtime = StationRuntimeFactory::create(config);
// 如果 COM 口打开失败，自动使用 MockSerialBusController
```

**手动启用 Mock 模式**:

```cpp
StationConfig config;
config.useMockDevices = true;  // 强制使用 Mock 设备
auto runtime = StationRuntimeFactory::create(config);
```

---

## 仿真测试框架

### 1. 快速开始

```cpp
#include "src/infrastructure/simulation/SimulationTestRuntime.h"

class MyTest : public QObject {
    Q_OBJECT
private:
    std::unique_ptr<SimulationTestRuntime> m_runtime;
    SimulationTestHelper* m_helper;

private slots:
    void init() {
        m_runtime = std::make_unique<SimulationTestRuntime>();
        QVERIFY(m_runtime->initialize());
        m_helper = m_runtime->helper();
    }

    void cleanup() {
        m_runtime->reset();
    }

    void testMotorControl() {
        // 启动电机
        m_helper->setMotorForward(50.0);
        
        // 推进时间
        m_helper->advanceMs(200);
        
        // 验证速度
        QVERIFY(m_helper->motorSpeedRpm() > 0.0);
    }
};
```

### 2. 核心 API

#### 时间控制

```cpp
// 按 tick 推进 (1 tick = 10ms)
m_helper->advanceTicks(10);

// 按毫秒推进
m_helper->advanceMs(100);

// 获取当前 tick
uint64_t tick = m_helper->currentTick();
```

#### 电机控制

```cpp
// 正转
m_helper->setMotorForward(50.0);  // 50% 占空比

// 反转
m_helper->setMotorReverse(30.0);

// 制动
m_helper->brakeMotor();

// 滑行
m_helper->coastMotor();

// 读取状态
double current = m_helper->motorCurrentA();
double speed = m_helper->motorSpeedRpm();
```

#### 编码器控制

```cpp
// 设置角度
m_helper->setEncoderAngle(180.0);

// 设置零点
m_helper->setEncoderZero();

// 读取状态
double angle = m_helper->encoderAngleDeg();
```

#### 扭矩传感器控制

```cpp
// 设置扭矩
m_helper->setTorque(5.0);  // 5.0 N·m

// 设置转速
m_helper->setSpeed(100.0);  // 100 RPM

// 读取状态
double torque = m_helper->torqueNm();
double speed = m_helper->speedRpm();
double power = m_helper->powerW();
```

#### 制动器控制

```cpp
// 设置 CH1 电压和电流
m_helper->setBrakeVoltage(24.0, 1);
m_helper->setBrakeCurrent(2.0, 1);

// 使能输出
m_helper->enableBrakeOutput(1);

// 读取状态
double voltage = m_helper->brakeVoltageV(1);
double current = m_helper->brakeCurrentA(1);
```

#### 事件触发

```cpp
// 触发磁铁检测
m_helper->triggerMagnetDetection();

// 触发角度到达
m_helper->triggerAngleArrival(180.0);

// 触发锁死
m_helper->triggerLock();
```

### 3. 场景助手

```cpp
// 快速设置找零场景
m_helper->setupHomingScenario();

// 快速设置空载场景
m_helper->setupIdleScenario();

// 快速设置角度定位场景
m_helper->setupAngleScenario(180.0);

// 快速设置负载测试场景
m_helper->setupLoadScenario(5.0);
```

---

## 使用场景

### 场景 1: 开发阶段无硬件测试

**问题**: 硬件未到货，需要开发和测试上位机软件

**解决方案**: 使用 Mock 设备运行应用

```bash
# 直接运行，自动使用 Mock 设备
.\build\appFlipGearboxFactoryTest.exe
```

应用会显示 "Mock 模式" 标识，所有设备操作使用仿真数据。

### 场景 2: 自动化测试

**问题**: 需要快速、可重复的自动化测试

**解决方案**: 使用仿真测试框架

```cpp
void testHomingPhase() {
    // 设置场景
    m_helper->setupHomingScenario();
    
    // 启动测试
    m_engine->startTest(recipe);
    
    // 推进到磁铁检测
    m_helper->advanceMs(500);
    m_helper->triggerMagnetDetection();
    
    // 验证零点设置
    m_helper->advanceMs(100);
    QCOMPARE(m_helper->encoderAngleDeg(), 0.0);
}
```

### 场景 3: 边界条件测试

**问题**: 需要测试极端情况 (锁死、超时、通信错误)

**解决方案**: 使用故障注入

```cpp
// 注入 CRC 错误
mockDevice->injectCrcError(true);

// 注入超时
mockDevice->injectTimeout(true);

// 注入延迟
mockDevice->injectDelay(500);  // 500ms 延迟

// 注入异常响应
mockDevice->injectException(0x03, 0x02);  // 非法数据地址
```

---

## 故障注入

### 1. 通信故障

```cpp
// CRC 错误
mockMotor->injectCrcError(true);
m_helper->setMotorForward(50.0);
// 预期: 通信失败，重试

// 超时
mockMotor->injectTimeout(true);
m_helper->setMotorForward(50.0);
// 预期: 超时错误

// 延迟响应
mockMotor->injectDelay(200);  // 200ms 延迟
m_helper->setMotorForward(50.0);
// 预期: 响应慢但成功
```

### 2. 设备异常

```cpp
// Modbus 异常码
mockMotor->injectException(0x06, 0x01);  // 功能码 0x06, 异常码 0x01 (非法功能)
m_helper->setMotorForward(50.0);
// 预期: 收到异常响应

// 非法数据地址
mockMotor->injectException(0x03, 0x02);  // 功能码 0x03, 异常码 0x02
// 预期: 地址错误
```

### 3. 物理故障

```cpp
// 编码器断线
m_helper->setEncoderAngle(-999.0);  // 无效角度
// 预期: 角度读取失败

// 扭矩传感器过载
m_helper->setTorque(999.0);  // 超量程
// 预期: 过载保护触发

// 制动器短路
m_helper->setBrakeCurrent(50.0, 1);  // 过流
// 预期: 过流保护触发
```

---

## 测试示例

### 完整测试用例

```cpp
void testLoadPhaseWithLockDetection() {
    // 1. 初始化
    TestRecipe recipe;
    recipe.loadTorqueNm = 5.0;
    recipe.lockThresholdNm = 0.5;
    recipe.loadDurationMs = 2000;
    
    // 2. 设置场景
    m_helper->setupLoadScenario(5.0);
    
    // 3. 启动测试
    m_engine->startTest(recipe);
    m_helper->advanceToPhase("Load");
    
    // 4. 施加负载
    m_helper->setBrakeVoltage(24.0, 1);
    m_helper->enableBrakeOutput(1);
    m_helper->advanceMs(500);
    
    // 5. 验证扭矩上升
    QVERIFY(m_helper->torqueNm() > 4.0);
    
    // 6. 触发锁死
    m_helper->setSpeed(0.0);  // 转速降为 0
    m_helper->advanceMs(100);
    
    // 7. 验证锁死检测
    QVERIFY(m_engine->isLocked());
    QCOMPARE(m_engine->currentPhase(), "Return");
}
```

---

## 最佳实践

### 1. 测试隔离

每个测试用例使用独立的 `SimulationTestRuntime` 实例：

```cpp
void init() {
    m_runtime = std::make_unique<SimulationTestRuntime>();
    m_runtime->initialize();
}

void cleanup() {
    m_runtime->reset();  // 清理状态
}
```

### 2. 确定性时间

避免使用 `QTest::qWait()`，使用 `advanceMs()` 代替：

```cpp
// ❌ 不推荐
m_helper->setMotorForward(50.0);
QTest::qWait(200);  // 不确定性

// ✅ 推荐
m_helper->setMotorForward(50.0);
m_helper->advanceMs(200);  // 确定性
```

### 3. 状态验证

在关键点验证状态：

```cpp
// 启动电机
m_helper->setMotorForward(50.0);
m_helper->advanceMs(100);

// 验证中间状态
QVERIFY(m_helper->motorSpeedRpm() > 0.0);
QVERIFY(m_helper->motorCurrentA() > 0.0);

// 继续推进
m_helper->advanceMs(100);

// 验证最终状态
QVERIFY(m_helper->motorSpeedRpm() > 100.0);
```

---

## 故障排查

### Mock 设备未生效

**症状**: 应用仍尝试打开真实串口

**原因**: `StationConfig` 中未设置 `useMockDevices`

**解决**:
```cpp
config.useMockDevices = true;
```

### 仿真时间不推进

**症状**: `advanceMs()` 调用后状态未变化

**原因**: 未调用 `QCoreApplication::processEvents()`

**解决**:
```cpp
m_helper->advanceMs(100);
QCoreApplication::processEvents();  // 处理信号槽
```

### 测试不稳定

**症状**: 测试结果随机失败

**原因**: 使用了真实时间或异步操作

**解决**: 使用确定性时间控制，避免 `QTimer` 和 `QTest::qWait()`

---

## 相关文档

- [用户手册](USER_MANUAL.md) - Mock 模式使用说明
- [故障排查](TROUBLESHOOTING.md) - 常见问题
- [测试指南](../tests/README.md) - 测试套件说明

---

**文档版本**: 1.0  
**合并自**: MockDeviceSimulator.md, SimulationTestFramework.md, MockDeviceImplementationSummary.md, SimulationTestFramework_Summary.md, SimulationTestFramework_Integration.md
