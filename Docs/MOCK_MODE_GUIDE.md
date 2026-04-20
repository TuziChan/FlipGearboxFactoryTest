# Mock模式使用指南

## 概述

本项目支持两种运行模式：
- **真实硬件模式**：连接实际的Modbus设备（电机驱动器、扭矩传感器、编码器、制动电源）
- **Mock模式（模拟模式）**：使用软件模拟设备，无需物理硬件，适用于开发、测试和演示

Mock模式通过 `SimulationContext` 提供确定性的物理仿真，所有设备行为基于统一的仿真时钟推进，确保测试结果可重现。

---

## 1. 如何启动Mock模式

### 1.1 命令行参数启动

在启动应用程序时添加 `--mock` 参数：

```bash
# Windows
FlipGearboxFactoryTest.exe --mock

# Linux/macOS
./FlipGearboxFactoryTest --mock
```

**实现原理**（main.cpp:48-50）：
```cpp
const QStringList args = app.arguments();
bool mockMode = args.contains("--mock");
```

应用程序检测到 `--mock` 参数后，会将 `mockMode=true` 传递给 `RuntimeManager`，后者调用 `StationRuntimeFactory::create(config, true)` 创建模拟设备。

### 1.2 默认启动模式

如果不添加 `--mock` 参数，应用程序默认以**真实硬件模式**启动，尝试连接配置文件中指定的串口设备。

---

## 2. Mock模式与真实硬件模式的区别

### 2.1 设备实现对比

| 组件 | 真实硬件模式 | Mock模式 |
|------|------------|---------|
| **总线控制器** | `ModbusRtuBusController`<br/>（真实串口通信） | `SimulatedBusController`<br/>（内存模拟，无实际I/O） |
| **电机驱动器** | `AqmdMotorDriveDevice`<br/>（Modbus协议） | `SimulatedMotorDevice`<br/>（基于物理模型） |
| **扭矩传感器** | `Dyn200TorqueSensorDevice`<br/>（Modbus协议） | `SimulatedTorqueDevice`<br/>（根据制动负载计算） |
| **编码器** | `SingleTurnEncoderDevice`<br/>（Modbus协议） | `SimulatedEncoderDevice`<br/>（根据电机转速积分） |
| **制动电源** | `BrakePowerSupplyDevice`<br/>（Modbus协议） | `SimulatedBrakeDevice`<br/>（模拟恒流/恒压输出） |

### 2.2 核心差异

**真实硬件模式**：
- 需要物理设备和串口连接
- 受硬件响应时间、通信延迟影响
- 可能出现通信超时、CRC错误等硬件故障
- 配置来自 `config/station.json`（串口号、波特率、从站地址等）

**Mock模式**：
- 无需物理设备，纯软件仿真
- 确定性行为，相同输入产生相同输出（见 `SimulationRuntimeTests.cpp:652-671`）
- 无通信故障，所有操作立即成功
- 使用虚拟串口配置（如 `SIM_AQMD`），实际不打开串口

### 2.3 物理仿真特性

Mock模式通过 `SimulationContext` 实现物理仿真：

- **电机加速/减速**：根据占空比和负载计算转速变化（`SimulationContext::advanceTick()`）
- **制动负载效应**：制动电流增加会降低电机转速
- **编码器角度积分**：根据转速累积角度，自动处理360°回绕
- **扭矩计算**：基于制动电流和转速计算扭矩值
- **磁铁检测模拟**：电机旋转时周期性触发AI1电平变化

---

## 3. 在UI中切换模式

### 3.1 运行时切换功能

应用程序支持在运行时动态切换模式，无需重启。

**操作步骤**：
1. 打开 **I/O 诊断** 页面（DiagnosticsPage.qml）
2. 查看页面顶部的模式徽章：
   - 显示 **"模拟模式"**（蓝色）或 **"真实硬件"**（灰色）
3. 点击 **"切换到真实硬件"** 或 **"切换到模拟模式"** 按钮

**实现原理**（DiagnosticsPage.qml:71-82）：
```qml
Components.AppButton {
    text: root.viewModel && root.viewModel.isMockMode 
          ? "切换到真实硬件" 
          : "切换到模拟模式"
    variant: "outline"
    size: "sm"
    theme: root.theme
    onClicked: {
        if (root.viewModel) {
            var newMode = !root.viewModel.isMockMode
            root.viewModel.switchMockMode(newMode)
        }
    }
}
```

### 3.2 切换流程

当用户点击切换按钮时，执行以下步骤：

1. **DiagnosticsViewModel** 调用 `RuntimeManager::switchMode(mockMode)`
2. **RuntimeManager** 执行：
   - 关闭旧的 `StationRuntime`（调用 `shutdown()`）
   - 销毁旧设备对象
   - 调用 `StationRuntimeFactory::create(config, newMockMode)` 创建新运行时
   - 初始化新运行时（调用 `initialize()`）
   - 发出 `runtimeRecreated` 信号
3. **DiagnosticsViewModel** 接收信号，更新设备引用并刷新UI

**关键代码**（RuntimeManager.cpp:16-30）：
```cpp
void RuntimeManager::switchMode(bool mockMode) {
    if (m_isMockMode == mockMode) {
        qDebug() << "Already in" << (mockMode ? "mock" : "real") 
                 << "mode, no switch needed";
        return;
    }

    qDebug() << "Switching from" << (m_isMockMode ? "mock" : "real")
             << "to" << (mockMode ? "mock" : "real") << "mode";

    recreateRuntime(mockMode);
    m_isMockMode = mockMode;

    emit mockModeChanged();
    emit runtimeRecreated(m_runtime.get());
}
```

### 3.3 注意事项

- 切换模式会**中断正在运行的测试**，测试状态会丢失
- 切换后需要重新初始化设备（自动完成）
- 从Mock模式切换到真实硬件模式时，如果硬件未连接，设备将显示为离线

---

## 4. 在测试中使用Mock模式

### 4.1 创建Mock运行时

测试代码中使用 `StationRuntimeFactory::create()` 的第二个参数控制模式：

```cpp
#include "src/infrastructure/config/StationRuntimeFactory.h"
#include "src/infrastructure/config/StationConfig.h"

// 创建测试配置
StationConfig config;
config.stationName = "TestStation";
config.brakeChannel = 1;

// 启用所有设备
config.aqmdConfig.enabled = true;
config.dyn200Config.enabled = true;
config.encoderConfig.enabled = true;
config.brakeConfig.enabled = true;

// 创建Mock运行时
auto runtime = StationRuntimeFactory::create(config, true);  // true = mock mode

// 初始化
QVERIFY(runtime->initialize());

// 使用设备
runtime->motor()->setMotor(IMotorDriveDevice::Direction::Forward, 50.0);
runtime->brake()->setCurrent(1, 2.0);

// 清理
runtime->shutdown();
```

### 4.2 测试示例：电机控制

**示例1：测试电机正转**（SimulationRuntimeTests.cpp:338-345）
```cpp
void testSimulatedMotorSetForward() {
    SimulationContext ctx;
    SimulatedMotorDevice motor(&ctx);
    motor.initialize();
    
    QVERIFY(motor.setMotor(IMotorDriveDevice::Direction::Forward, 50.0));
    QCOMPARE(ctx.motorDirection(), SimulationContext::MotorDirection::Forward);
    QCOMPARE(ctx.motorDutyCycle(), 50.0);
}
```

**示例2：测试电机加速**（SimulationRuntimeTests.cpp:193-212）
```cpp
void testSimulationContextMotorAcceleration() {
    SimulationContext ctx;
    
    // 启动电机，50%占空比正转
    ctx.setMotorDirection(SimulationContext::MotorDirection::Forward);
    ctx.setMotorDutyCycle(50.0);
    
    // 初始转速为0
    QCOMPARE(ctx.encoderAngularVelocityRpm(), 0.0);
    
    // 推进100个仿真tick
    for (int i = 0; i < 100; i++) {
        ctx.advanceTick();
    }
    
    // 转速应为正值，且接近目标（50% * 1500 RPM = 750 RPM）
    double speed = ctx.encoderAngularVelocityRpm();
    QVERIFY(speed > 0.0);
    QVERIFY(speed <= 750.0);
}
```

### 4.3 测试示例：制动负载效应

**示例3：验证制动器降低转速**（SimulationRuntimeTests.cpp:238-261）
```cpp
void testSimulationContextBrakeLoadEffect() {
    SimulationContext ctx;
    
    // 加速电机到全速
    ctx.setMotorDirection(SimulationContext::MotorDirection::Forward);
    ctx.setMotorDutyCycle(100.0);
    for (int i = 0; i < 300; i++) {
        ctx.advanceTick();
    }
    
    double speedWithoutBrake = ctx.encoderAngularVelocityRpm();
    
    // 施加制动负载
    ctx.setBrakeCurrent(2.0);
    ctx.setBrakeOutputEnabled(true);
    for (int i = 0; i < 100; i++) {
        ctx.advanceTick();
    }
    
    double speedWithBrake = ctx.encoderAngularVelocityRpm();
    
    // 制动应降低转速
    QVERIFY(speedWithBrake < speedWithoutBrake);
}
```

### 4.4 测试示例：完整测试流程

**示例4：集成测试**（SimulationRuntimeTests.cpp:626-650）
```cpp
void testFullSimulationCycle() {
    StationConfig config = makeTestConfig();
    auto runtime = StationRuntimeFactory::create(config, true);
    
    QVERIFY(runtime->initialize());
    
    // 设置电机正转
    QVERIFY(runtime->motor()->setMotor(
        IMotorDriveDevice::Direction::Forward, 50.0));
    
    // 施加制动
    QVERIFY(runtime->brake()->setBrakeMode(1, "CC"));
    QVERIFY(runtime->brake()->setCurrent(1, 1.5));
    QVERIFY(runtime->brake()->setOutputEnable(1, true));
    
    // 读取编码器角度
    double angle = 0.0;
    QVERIFY(runtime->encoder()->readAngle(angle));
    
    // 读取扭矩
    double torque = 0.0;
    QVERIFY(runtime->torque()->readTorque(torque));
    
    runtime->shutdown();
    QVERIFY(!runtime->isInitialized());
}
```

### 4.5 确定性测试

Mock模式的关键优势是**确定性**：相同的输入序列总是产生相同的输出。

**示例5：验证确定性**（SimulationRuntimeTests.cpp:652-671）
```cpp
void testSimulationDeterminism() {
    // 运行相同仿真两次，验证结果一致
    SimulationContext ctx1, ctx2;
    
    // 配置相同
    ctx1.setMotorDirection(SimulationContext::MotorDirection::Forward);
    ctx1.setMotorDutyCycle(50.0);
    ctx2.setMotorDirection(SimulationContext::MotorDirection::Forward);
    ctx2.setMotorDutyCycle(50.0);
    
    // 运行相同tick数
    for (int i = 0; i < 100; i++) {
        ctx1.advanceTick();
        ctx2.advanceTick();
    }
    
    // 结果应完全一致
    QCOMPARE(ctx1.encoderAngleDeg(), ctx2.encoderAngleDeg());
    QCOMPARE(ctx1.encoderAngularVelocityRpm(), ctx2.encoderAngularVelocityRpm());
}
```

---

## 5. Mock模式的实现机制

### 5.1 StationRuntimeFactory创建流程

**关键代码**（StationRuntimeFactory.cpp:18-139）：

```cpp
std::unique_ptr<StationRuntime> StationRuntimeFactory::create(
    const StationConfig& config, bool mockMode) {
    
    auto runtime = std::make_unique<StationRuntime>();
    runtime->m_isMockMode = mockMode;

    if (mockMode) {
        // === Mock模式：创建模拟设备 ===
        auto simContext = std::make_shared<SimulationContext>();
        runtime->m_simulationContext = simContext;

        // 创建模拟总线（无实际I/O）
        runtime->m_aqmdBus = std::make_unique<SimulatedBusController>();
        runtime->m_dyn200Bus = std::make_unique<SimulatedBusController>();
        runtime->m_encoderBus = std::make_unique<SimulatedBusController>();
        runtime->m_brakeBus = std::make_unique<SimulatedBusController>();

        // 创建模拟设备（共享SimulationContext）
        if (config.aqmdConfig.enabled) {
            runtime->m_motor = std::make_unique<SimulatedMotorDevice>(
                simContext.get(), runtime.get());
        }
        if (config.dyn200Config.enabled) {
            runtime->m_torque = std::make_unique<SimulatedTorqueDevice>(
                simContext.get(), runtime.get());
        }
        // ... 其他设备
    } else {
        // === 真实硬件模式：创建Modbus设备 ===
        runtime->m_aqmdBus = std::make_unique<ModbusRtuBusController>();
        runtime->m_aqmdBusConfig = {
            config.aqmdConfig.portName,
            config.aqmdConfig.baudRate,
            config.aqmdConfig.slaveId,
            // ...
        };
        
        if (config.aqmdConfig.enabled) {
            runtime->m_motor = std::make_unique<AqmdMotorDriveDevice>(
                runtime->m_aqmdBus.get(),
                config.aqmdConfig.slaveId,
                runtime.get()
            );
        }
        // ... 其他设备
    }

    // === 通用设置（两种模式共享） ===
    runtime->m_testEngine = std::make_unique<GearboxTestEngine>(runtime.get());
    runtime->m_acquisitionScheduler = std::make_unique<AcquisitionScheduler>(runtime.get());
    // ...

    return runtime;
}
```

### 5.2 SimulationContext：仿真核心

`SimulationContext` 是Mock模式的物理引擎，维护所有仿真状态：

**状态变量**：
- `m_tickCount`：仿真时钟（每次 `advanceTick()` 递增）
- `m_motorDirection`：电机方向（Forward/Reverse/Stopped）
- `m_motorDutyCycle`：电机占空比（0-100%）
- `m_encoderAngleDeg`：编码器角度（0-360°）
- `m_encoderAngularVelocityRpm`：编码器转速（RPM）
- `m_brakeCurrent`：制动电流（A）
- `m_brakeVoltage`：制动电压（V）
- `m_brakeOutputEnabled`：制动输出使能

**物理模型**（简化）：
```cpp
void SimulationContext::advanceTick() {
    m_tickCount++;
    
    // 计算目标转速（基于占空比）
    double targetRpm = m_motorDutyCycle * 15.0;  // 最大1500 RPM
    if (m_motorDirection == MotorDirection::Reverse) {
        targetRpm = -targetRpm;
    } else if (m_motorDirection == MotorDirection::Stopped) {
        targetRpm = 0.0;
    }
    
    // 计算制动负载
    double brakeLoad = m_brakeOutputEnabled ? m_brakeCurrent * 50.0 : 0.0;
    
    // 加速度（考虑制动负载）
    double acceleration = (targetRpm - m_encoderAngularVelocityRpm) * 0.05 - brakeLoad * 0.01;
    
    // 更新转速
    m_encoderAngularVelocityRpm += acceleration;
    
    // 更新角度（积分）
    double deltaAngle = m_encoderAngularVelocityRpm / 60.0 * 360.0 * 0.001;  // 1ms tick
    m_encoderAngleDeg += deltaAngle;
    
    // 角度回绕到0-360°
    while (m_encoderAngleDeg >= 360.0) m_encoderAngleDeg -= 360.0;
    while (m_encoderAngleDeg < 0.0) m_encoderAngleDeg += 360.0;
}
```

### 5.3 模拟设备实现示例

**SimulatedMotorDevice**：
```cpp
bool SimulatedMotorDevice::setMotor(Direction direction, double dutyCyclePercent) {
    if (!m_initialized) return false;
    
    // 直接更新SimulationContext状态
    switch (direction) {
        case Direction::Forward:
            m_context->setMotorDirection(SimulationContext::MotorDirection::Forward);
            break;
        case Direction::Reverse:
            m_context->setMotorDirection(SimulationContext::MotorDirection::Reverse);
            break;
    }
    m_context->setMotorDutyCycle(dutyCyclePercent);
    
    return true;  // Mock模式总是成功
}

bool SimulatedMotorDevice::readCurrent(double& currentA) {
    if (!m_initialized) return false;
    
    // 根据占空比和转速计算电流
    double dutyCycle = m_context->motorDutyCycle();
    double speed = std::abs(m_context->encoderAngularVelocityRpm());
    currentA = dutyCycle * 0.05 + speed * 0.001;  // 简化模型
    
    return true;
}
```

---

## 6. 常见问题

### Q1: Mock模式下设备总是在线吗？
**A**: 是的。Mock模式中所有设备操作立即成功，不会出现通信超时或硬件故障。这是设计目标，确保测试稳定性。

### Q2: Mock模式的物理仿真有多精确？
**A**: 仿真模型是**简化的**，目标是提供合理的行为而非高精度物理模拟。例如：
- 电机加速度是线性的，未考虑惯性、摩擦等复杂因素
- 扭矩计算基于简单的负载模型
- 不模拟电气特性（如电流纹波、电压降）

对于需要高精度物理验证的场景，仍需使用真实硬件。

### Q3: 可以在测试中混合使用Mock和真实设备吗？
**A**: 不可以。`StationRuntimeFactory::create()` 的 `mockMode` 参数是全局的，要么所有设备都是Mock，要么都是真实硬件。

### Q4: 切换模式后测试数据会丢失吗？
**A**: 是的。切换模式会销毁并重建整个运行时，包括：
- 设备对象
- 测试引擎状态
- 采集调度器缓冲区

历史记录（HistoryService）不受影响，因为它独立于运行时。

### Q5: Mock模式下如何模拟设备故障？
**A**: 当前Mock实现不支持故障注入。所有操作总是成功。如果需要测试错误处理逻辑，可以：
- 扩展 `SimulationContext` 添加故障注入API
- 或使用真实硬件并人为制造故障（如断开连接）

---

## 7. 最佳实践

### 7.1 开发阶段
- **优先使用Mock模式**进行UI开发和业务逻辑验证
- 无需等待硬件到位，加快开发迭代
- 使用 `--mock` 参数启动，避免串口冲突

### 7.2 测试阶段
- **单元测试**：使用Mock模式，确保测试快速且确定性
- **集成测试**：Mock模式验证组件协作，真实硬件验证通信协议
- **回归测试**：Mock模式作为CI/CD管道的一部分，无需硬件环境

### 7.3 调试阶段
- 使用DiagnosticsPage的**运行时切换功能**对比Mock和真实硬件行为
- 如果真实硬件出现异常，切换到Mock模式验证是硬件问题还是软件逻辑问题

### 7.4 演示阶段
- 在没有硬件的场合（如客户演示、培训），使用Mock模式展示完整功能
- Mock模式下所有操作响应迅速，无通信延迟

---

## 8. 相关文件

### 核心实现
- `src/infrastructure/config/StationRuntimeFactory.cpp` - 运行时创建逻辑
- `src/infrastructure/config/RuntimeManager.cpp` - 模式切换管理
- `src/infrastructure/simulation/SimulationContext.h/cpp` - 物理仿真引擎

### 模拟设备
- `src/infrastructure/simulation/SimulatedBusController.h/cpp`
- `src/infrastructure/simulation/SimulatedMotorDevice.h/cpp`
- `src/infrastructure/simulation/SimulatedTorqueDevice.h/cpp`
- `src/infrastructure/simulation/SimulatedEncoderDevice.h/cpp`
- `src/infrastructure/simulation/SimulatedBrakeDevice.h/cpp`

### UI集成
- `src/ui/pages/DiagnosticsPage.qml` - 模式切换UI
- `src/viewmodels/DiagnosticsViewModel.h/cpp` - 模式切换逻辑

### 测试
- `tests/simulation/SimulationRuntimeTests.cpp` - Mock模式完整测试套件
- `tests/simulation/README.md` - 测试文档
- `tests/simulation/TEST_COVERAGE.md` - 测试覆盖率报告

---

## 9. 总结

Mock模式是本项目的核心特性，提供以下价值：

✅ **开发效率**：无需硬件即可开发和测试完整功能  
✅ **测试稳定性**：确定性行为，无随机故障，适合自动化测试  
✅ **快速迭代**：即时响应，无通信延迟  
✅ **灵活切换**：运行时动态切换，方便对比验证  
✅ **CI/CD友好**：无硬件依赖，可在任何环境运行测试  

通过合理使用Mock模式和真实硬件模式，可以在开发效率和验证准确性之间取得最佳平衡。
