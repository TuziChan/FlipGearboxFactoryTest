# 系统架构设计方案

## 一、系统架构分层

```
┌─────────────────────────────────────────────────────────────┐
│                    UI 层 (Presentation)                      │
│  - TestExecutionViewModel (MVVM 模式)                        │
│  - QML/Widgets UI 组件                                       │
│  - ChartPainter (实时数据可视化)                             │
└─────────────────────────────────────────────────────────────┘
                            ↓ 信号槽 / Q_PROPERTY
┌─────────────────────────────────────────────────────────────┐
│                  业务逻辑层 (Domain)                          │
│  - GearboxTestEngine (核心状态机)                            │
│  - TestRecipe (测试配方)                                     │
│  - TestRunState (运行时状态)                                 │
│  - TestResults (测试结果)                                    │
│  - RecipeValidator (配方验证)                                │
└─────────────────────────────────────────────────────────────┘
                            ↓ 接口调用
┌─────────────────────────────────────────────────────────────┐
│              设备抽象层 (Infrastructure/Devices)              │
│  - IMotorDriveDevice (电机接口)                              │
│  - ITorqueSensorDevice (扭矩传感器接口)                      │
│  - IEncoderDevice (编码器接口)                               │
│  - IBrakePowerDevice (制动电源接口)                          │
│  - DeviceConfigService (设备配置服务)                        │
└─────────────────────────────────────────────────────────────┘
                            ↓ 实现切换
┌──────────────────────┬──────────────────────────────────────┐
│   真实硬件实现        │      Mock/仿真层 (Simulation)         │
│  - AqmdMotorDevice   │  - SimulatedMotorDevice              │
│  - Dyn200TorqueSensor│  - SimulatedTorqueDevice             │
│  - SingleTurnEncoder │  - SimulatedEncoderDevice            │
│  - BrakePowerSupply  │  - SimulatedBrakeDevice              │
│                      │  - MockDeviceManager (统一管理)       │
└──────────────────────┴──────────────────────────────────────┘
                            ↓ 总线通信
┌─────────────────────────────────────────────────────────────┐
│              通信层 (Infrastructure/Bus)                      │
│  - IBusController (总线抽象)                                 │
│  - ModbusRtuController (真实 Modbus)                         │
│  - MockSerialBusController (Mock 总线)                       │
└─────────────────────────────────────────────────────────────┘
                            ↓ 数据采集
┌─────────────────────────────────────────────────────────────┐
│           数据采集层 (Infrastructure/Acquisition)             │
│  - AcquisitionScheduler (采集调度器)                         │
│  - DevicePoller (设备轮询线程)                               │
│  - TelemetryBuffer (无锁遥测缓冲区)                          │
└─────────────────────────────────────────────────────────────┘
```

### 各层职责边界

**UI 层**
- ✅ 负责：用户交互、数据展示、图表绘制
- ❌ 禁止：直接调用设备接口、包含业务逻辑、管理测试状态

**业务逻辑层**
- ✅ 负责：测试流程编排、状态机管理、结果判定、配方验证
- ❌ 禁止：直接操作硬件寄存器、UI 渲染、线程管理

**设备抽象层**
- ✅ 负责：统一设备接口、参数校验、错误处理
- ❌ 禁止：包含测试逻辑、UI 交互、数据持久化

**Mock/仿真层**
- ✅ 负责：模拟设备行为、错误注入、场景仿真
- ❌ 禁止：修改真实硬件代码、影响生产逻辑

**数据采集层**
- ✅ 负责：高频数据采集、线程管理、无锁缓冲
- ❌ 禁止：数据分析、业务判断、UI 更新

---

## 二、测试执行页模块切分

### 2.1 状态机设计（QStateMachine）

**核心状态（TestPhase）**
```
Idle → PrepareAndHome → IdleRun → AnglePositioning → LoadRampAndLock → ReturnToZero → Completed/Failed
```

**详细子状态（TestSubState）**
- **Homing**: SeekingMagnet → AdvancingToEncoderZero → HomeSettled
- **IdleRun**: SpinupForward → SampleForward → SpinupReverse → SampleReverse
- **AnglePositioning**: MoveToPosition1 → Hold → MoveToPosition2 → Hold → MoveBackToPosition1 → Hold → MoveToPosition3 → Hold → MoveBackToZero
- **LoadTest**: SpinupLoadForward → RampBrakeForward → ConfirmLockForward → SpinupLoadReverse → RampBrakeReverse → ConfirmLockReverse
- **ReturnToZero**: ReturnFinalZero → FinalZeroSettled

**状态转换触发条件**
- 时间超时（QTimer::singleShot）
- 传感器事件（磁铁检测、角度到位）
- 锁定确认（速度窗口 + 持续时间）
- 异常中断（emergencyStop）

**Qt 6.11 实现方案**
```cpp
// 使用 QStateMachine 替代当前的 switch-case 状态机
QStateMachine* m_stateMachine;
QState* m_idleState;
QState* m_homingState;
QState* m_idleRunState;
// ...

// 状态转换
m_idleState->addTransition(this, &GearboxTestEngine::startRequested, m_homingState);
m_homingState->addTransition(this, &GearboxTestEngine::homingCompleted, m_idleRunState);

// 子状态嵌套
QState* m_seekingMagnetState = new QState(m_homingState);
QState* m_advancingToZeroState = new QState(m_homingState);
m_homingState->setInitialState(m_seekingMagnetState);
```

### 2.2 线程模型（QThreadPool）

**当前实现（已有）**
- `DevicePoller` 继承 `QThread`，每个设备独立线程
- 采用无锁 `std::atomic` 缓冲区（TelemetryBuffer）

**优化方案（保持现有架构）**
```
主线程 (UI + 状态机)
  ↓ 信号槽
GearboxTestEngine (33ms 周期定时器)
  ↓ 读取
TelemetryBuffer (无锁原子变量)
  ↑ 写入
DevicePoller 线程池 (4 个独立线程)
  - MotorPoller (10ms 周期)
  - TorquePoller (20ms 周期)
  - EncoderPoller (10ms 周期)
  - BrakePoller (50ms 周期)
```

**不采用 QThreadPool 的原因**
- 当前 `QThread` 实现已满足需求（固定 4 个设备，无动态任务）
- 无锁缓冲区性能优异，无需改造
- `QThreadPool` 适合短任务，不适合长期运行的轮询线程

**线程安全保证**
- ✅ 采集线程只写 `TelemetryBuffer`
- ✅ 主线程只读 `TelemetryBuffer`
- ✅ 使用 `std::atomic` 保证原子性
- ❌ 禁止跨线程直接调用设备接口

### 2.3 数据流设计

```
设备硬件
  ↓ Modbus RTU (串口通信)
DevicePoller 线程
  ↓ std::atomic 写入
TelemetryBuffer (无锁缓冲区)
  ↓ 定时读取 (33ms)
GearboxTestEngine::acquireTelemetry()
  ↓ 状态机处理
TestRunState (当前状态 + 遥测快照)
  ↓ 信号发射
TestExecutionViewModel
  ↓ Q_PROPERTY 通知
QML/Widgets UI
  ↓ 实时绘制
ChartPainter
```

**关键数据结构**
```cpp
// 遥测快照（单次采样）
struct TelemetrySnapshot {
    double motorCurrentA;
    double torqueNm;
    double speedRpm;
    double powerW;
    double angleDeg;
    double brakeCurrentA;
    bool ai1Level;
    uint64_t timestampNs;
};

// 运行时状态（投影到 UI）
struct TestRunState {
    TestPhase phase;
    TestSubState subState;
    TelemetrySnapshot currentTelemetry;
    TestResults results;
    QString statusMessage;
    int progressPercent;
};
```

---

## 三、状态总线设计

### 3.1 当前实现（信号槽总线）

**优势**
- Qt 原生机制，跨线程安全
- 自动队列化，无需手动同步
- 支持多订阅者（一对多广播）

**现有信号**
```cpp
// GearboxTestEngine → TestExecutionViewModel
signals:
    void stateChanged(const TestRunState& state);
    void testCompleted(const TestResults& results);
    void testFailed(const FailureReason& reason);

// DevicePoller → Engine
signals:
    void errorOccurred(const QString& error);
```

### 3.2 不采用 QSharedMemory 的原因

- ❌ 当前为单进程架构，无跨进程需求
- ❌ 信号槽已满足解耦需求
- ❌ 共享内存增加复杂度，无性能收益

### 3.3 状态总线扩展方案（可选）

**场景**：需要多个 UI 组件同时监听状态
```cpp
// 引入事件总线（仅在必要时）
class TestEventBus : public QObject {
    Q_OBJECT
public:
    static TestEventBus* instance();
    
signals:
    void stateChanged(const TestRunState& state);
    void telemetryUpdated(const TelemetrySnapshot& snapshot);
    void phaseTransitioned(TestPhase from, TestPhase to);
};

// 使用 QSignalMapper 替代方案（Lambda）
connect(engine, &GearboxTestEngine::stateChanged, 
        [=](const TestRunState& state) {
            TestEventBus::instance()->stateChanged(state);
        });
```

**边界约束**
- ✅ 仅用于 UI 层多组件通信
- ❌ 禁止业务逻辑层依赖事件总线
- ❌ 禁止用于设备层通信

---

## 四、设备抽象层接口定义

### 4.1 核心接口（已实现）

**IMotorDriveDevice**
```cpp
class IMotorDriveDevice : public QObject {
    Q_OBJECT
public:
    enum class Direction { Forward, Reverse, Brake };
    
    virtual bool initialize() = 0;
    virtual bool setMotor(Direction dir, double dutyCyclePercent) = 0;
    virtual bool brake() = 0;
    virtual bool coast() = 0;
    virtual bool readCurrent(double& currentA) = 0;
    virtual bool readAI1Level(bool& level) = 0;  // 磁铁检测
    virtual QString lastError() const = 0;
    
signals:
    void errorOccurred(const QString& error);
};
```

**ITorqueSensorDevice**
```cpp
class ITorqueSensorDevice : public QObject {
    Q_OBJECT
public:
    virtual bool initialize() = 0;
    virtual bool readAll(double& torqueNm, double& speedRpm, double& powerW) = 0;
    virtual QString lastError() const = 0;
    
signals:
    void errorOccurred(const QString& error);
};
```

**IEncoderDevice**
```cpp
class IEncoderDevice : public QObject {
    Q_OBJECT
public:
    virtual bool initialize() = 0;
    virtual bool readAngle(double& angleDeg) = 0;
    virtual QString lastError() const = 0;
    
signals:
    void errorOccurred(const QString& error);
};
```

**IBrakePowerDevice**
```cpp
class IBrakePowerDevice : public QObject {
    Q_OBJECT
public:
    virtual bool initialize() = 0;
    virtual bool setVoltage(int channel, double voltageV) = 0;
    virtual bool readCurrent(int channel, double& currentA) = 0;
    virtual QString lastError() const = 0;
    
signals:
    void errorOccurred(const QString& error);
};
```

### 4.2 接口设计原则

**✅ 遵循的原则**
- 同步阻塞调用（简化状态机逻辑）
- 返回 `bool` 表示成功/失败
- 通过引用参数返回数据
- 错误信息通过 `lastError()` 获取
- 异步错误通过信号 `errorOccurred` 通知

**❌ 避免的设计**
- 异步回调（增加状态机复杂度）
- 抛出异常（Qt 不推荐）
- 使用 `QVariant` 传参（类型不安全）
- 全局单例（影响测试）

### 4.3 设备工厂模式（插件化）

**当前实现**：硬编码创建设备
```cpp
// StationRuntime.cpp
m_motor = new AqmdMotorDriveDevice(busController, 1, this);
m_torque = new Dyn200TorqueSensorDevice(busController, 2, this);
```

**插件化方案（可选）**
```cpp
// 设备工厂接口
class IDeviceFactory {
public:
    virtual IMotorDriveDevice* createMotor(IBusController* bus, int slaveId) = 0;
    virtual ITorqueSensorDevice* createTorque(IBusController* bus, int slaveId) = 0;
    // ...
};

// 真实硬件工厂
class HardwareDeviceFactory : public IDeviceFactory {
    IMotorDriveDevice* createMotor(IBusController* bus, int slaveId) override {
        return new AqmdMotorDriveDevice(bus, slaveId);
    }
};

// Mock 工厂
class MockDeviceFactory : public IDeviceFactory {
    IMotorDriveDevice* createMotor(IBusController* bus, int slaveId) override {
        return new SimulatedMotorDevice(bus, slaveId);
    }
};

// 使用 QPluginLoader 动态加载（高级）
QPluginLoader loader("devices/mock_factory.dll");
IDeviceFactory* factory = qobject_cast<IDeviceFactory*>(loader.instance());
```

**边界约束**
- ✅ 仅在需要运行时切换 Mock/真实硬件时引入
- ❌ 禁止为单一实现引入工厂模式
- ❌ 禁止过度抽象（如设备参数用 QVariant）

---

## 五、Mock/仿真层架构

### 5.1 当前实现（已完善）

**MockDeviceManager**
- 统一管理 4 个 Mock 设备
- 提供场景仿真 API（角度测试、负载测试）
- 支持错误注入（CRC 错误、超时、延迟）

**MockSerialBusController**
- 模拟 Modbus RTU 协议
- 虚拟寄存器映射
- 可配置响应延迟和错误率

**设备 Mock 实现**
- `SimulatedMotorDevice`：模拟电机惯性、电流响应
- `SimulatedTorqueDevice`：模拟扭矩-速度曲线
- `SimulatedEncoderDevice`：模拟角度变化、磁铁位置
- `SimulatedBrakeDevice`：模拟制动电流

### 5.2 Mock 层架构图

```
MockDeviceManager (场景编排)
  ↓ 控制
┌─────────────────────────────────────────┐
│  MockSerialBusController (虚拟总线)      │
│  - 虚拟寄存器表                          │
│  - CRC 校验模拟                          │
│  - 延迟/错误注入                         │
└─────────────────────────────────────────┘
  ↓ 注册设备
┌──────────────┬──────────────┬──────────────┬──────────────┐
│ MockMotor    │ MockTorque   │ MockEncoder  │ MockBrake    │
│ Device       │ Device       │ Device       │ Device       │
│ (从站 1)     │ (从站 2)     │ (从站 3)     │ (从站 4)     │
└──────────────┴──────────────┴──────────────┴──────────────┘
  ↓ 实现接口
┌──────────────┬──────────────┬──────────────┬──────────────┐
│ IMotor       │ ITorque      │ IEncoder     │ IBrakePower  │
│ DriveDevice  │ SensorDevice │ Device       │ Device       │
└──────────────┴──────────────┴──────────────┴──────────────┘
```

### 5.3 Mock 层使用场景

**开发阶段**
- 无需真实硬件即可开发测试逻辑
- 快速验证状态机转换
- UI 界面调试

**测试阶段**
- 单元测试（隔离硬件依赖）
- 集成测试（模拟异常场景）
- 压力测试（高频数据采集）

**演示阶段**
- 客户演示（无需搭建硬件环境）
- 培训教学（可重复场景）

### 5.4 Mock/真实硬件切换机制

**方案 1：编译时切换（当前）**
```cpp
#ifdef USE_MOCK_DEVICES
    m_motor = new SimulatedMotorDevice(mockBus, 1, this);
#else
    m_motor = new AqmdMotorDriveDevice(realBus, 1, this);
#endif
```

**方案 2：配置文件切换（推荐）**
```json
// config/station.json
{
    "deviceMode": "mock",  // "mock" | "hardware"
    "devices": {
        "motor": { "type": "AQMD", "slaveId": 1 },
        "torque": { "type": "DYN200", "slaveId": 2 }
    }
}
```

```cpp
// DeviceConfigService
IMotorDriveDevice* DeviceConfigService::createMotor() {
    QString mode = config["deviceMode"].toString();
    if (mode == "mock") {
        return new SimulatedMotorDevice(mockBus, 1);
    } else {
        return new AqmdMotorDriveDevice(realBus, 1);
    }
}
```

**方案 3：运行时插件切换（高级）**
- 使用 `QPluginLoader` 动态加载设备驱动
- 适用于需要支持多种硬件型号的场景
- 当前项目暂不需要

---

## 六、测试闭环与日志

### 6.1 日志分级（QLoggingCategory）

**推荐分类**
```cpp
// 定义日志类别
Q_LOGGING_CATEGORY(logTest, "test")
Q_LOGGING_CATEGORY(logDevice, "device")
Q_LOGGING_CATEGORY(logBus, "bus")
Q_LOGGING_CATEGORY(logUI, "ui")
Q_LOGGING_CATEGORY(logPerf, "perf")

// 使用
qCDebug(logTest) << "Test started:" << serialNumber;
qCWarning(logDevice) << "Motor read failed:" << motor->lastError();
qCCritical(logBus) << "CRC error on slave" << slaveId;
```

**运行时控制**
```ini
# qtlogging.ini
[Rules]
test.debug=true
device.debug=false
bus.debug=true
*.critical=true
```

### 6.2 测试报告生成

**CSV 格式（数据分析）**
```cpp
QFile file("reports/" + serialNumber + ".csv");
QTextStream out(&file);
out << "Timestamp,Phase,Torque,Speed,Angle,Current\n";
out << timestamp << "," << phase << "," << torque << "," << speed << "\n";
```

**JSON 格式（结构化存储）**
```cpp
QJsonObject report;
report["serialNumber"] = serialNumber;
report["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
report["verdict"] = results.overallVerdict;
report["idleForward"] = QJsonObject{
    {"avgCurrent", results.idleForward.avgCurrentA},
    {"avgSpeed", results.idleForward.avgSpeedRpm},
    {"verdict", results.idleForward.verdict}
};
```

### 6.3 测试闭环流程

```
开始测试
  ↓
初始化设备 → 记录日志 (logDevice)
  ↓
执行测试 → 实时采集数据 → 写入缓冲区
  ↓
状态转换 → 记录日志 (logTest)
  ↓
判定结果 → 生成报告 (CSV + JSON)
  ↓
上传数据库/MES 系统（可选）
  ↓
清理资源 → 记录日志 (logTest)
```

---

## 七、关键技术决策

### 7.1 采用的 Qt 6.11 特性

| 特性 | 用途 | 优先级 |
|------|------|--------|
| QStateMachine | 测试流程状态管理 | P0 |
| QThread | 设备数据采集 | P0（已实现） |
| QTimer + QElapsedTimer | 超时控制、性能计时 | P0（已实现） |
| QLoggingCategory | 分级日志 | P1 |
| QTextStream + QFile | 测试报告生成 | P1 |
| QVariant + QMetaType | 设备参数传递 | P2（可选） |
| QPluginLoader | 设备驱动插件化 | P2（可选） |

### 7.2 不采用的特性及原因

| 特性 | 不采用原因 |
|------|-----------|
| QThreadPool | 当前固定 4 个设备，QThread 已满足需求 |
| QSharedMemory | 单进程架构，无跨进程需求 |
| Qt Quick/QML | 现有 Widgets 界面，重写成本高 |
| Qt D-Bus | Windows 平台支持有限 |
| Qt Concurrent | 无复杂数据流水线需求 |
| Qt Remote Objects | 无跨进程对象代理需求 |

### 7.3 过度设计警示

**❌ 禁止行为**
1. 为简单状态管理引入 QStateMachine（若状态 ≤3 个）
2. 为单进程应用引入 QSharedMemory
3. 全局使用 QVariant 传参（仅设备抽象层使用）
4. 为每个测试步骤创建独立线程
5. 引入 QML 重写现有 Widgets 界面

**✅ 合理边界**
- 状态机：仅用于测试执行页主流程（≥5 个状态）
- 插件系统：仅用于设备驱动切换（Mock/真实硬件）
- 日志分类：限 3-5 个类别
- 线程池：仅用于耗时 >100ms 的操作

---

## 八、实施优先级

### P0（立即采用）
1. ✅ QThread - 设备数据采集（已实现）
2. ✅ QTimer + QElapsedTimer - 超时控制（已实现）
3. 🔄 QStateMachine - 重构 GearboxTestEngine 状态机

### P1（基础完成后）
4. QLoggingCategory - 分级日志
5. QTextStream - 测试报告生成
6. 设备工厂模式 - Mock/真实硬件切换

### P2（可选增强）
7. QPluginLoader - 设备驱动插件化
8. QVariant - 设备接口统一
9. QTest - 单元测试覆盖

---

## 九、架构约束与边界

### 9.1 明确不做的内容

**❌ 禁止范围**
1. 不重构无关模块（配置管理、总线层已稳定）
2. 不引入新框架（如 QML、D-Bus）
3. 不大规模重命名/迁移文件
4. 不修改 Mock 层以外的设备实现
5. 不为单一场景过度抽象

### 9.2 架构演进路径

**阶段 1：状态机重构（当前）**
- 将 GearboxTestEngine 的 switch-case 改为 QStateMachine
- 保持现有接口不变
- 验证功能等价性

**阶段 2：日志与报告（后续）**
- 引入 QLoggingCategory
- 实现 CSV/JSON 报告生成
- 集成 MES 系统（可选）

**阶段 3：插件化（可选）**
- 设备工厂模式
- 配置文件驱动设备创建
- QPluginLoader 动态加载（高级）

### 9.3 性能指标

**实时性要求**
- 状态机周期：33ms（30Hz）
- 设备采集周期：10-50ms（根据设备）
- UI 刷新率：≥20Hz
- 测试总时长：<5 分钟

**资源约束**
- CPU 占用：<30%（单核）
- 内存占用：<200MB
- 线程数：≤10 个

---

## 十、总结

本架构设计遵循以下原则：
1. **分层清晰**：UI/业务/设备/通信/采集 五层解耦
2. **接口稳定**：设备抽象层接口已验证，无需大改
3. **Mock 完善**：仿真层已实现，支持无硬件开发
4. **线程安全**：无锁缓冲区 + 信号槽跨线程通信
5. **可测试性**：Mock 层 + 单元测试覆盖
6. **避免过度设计**：仅在必要时引入新特性

**关键改进点**
- 引入 QStateMachine 替代 switch-case 状态机
- 引入 QLoggingCategory 实现分级日志
- 完善测试报告生成（CSV/JSON）
- 明确架构边界，防止范围漂移
