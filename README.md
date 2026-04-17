# FlipGearboxFactoryTest

齿轮箱翻转产线自动化测试系统 —— 基于 Qt 6 / QML 构建的工业级齿轮箱出厂检测工位软件。

## 项目概述

本系统用于齿轮箱翻转执行器在产线上的全自动出厂测试，通过 Modbus RTU 总线与多个下位设备通信，按照预设配方自动执行找零、空载、角度定位、负载锁止等测试流程，并实时采集电流、扭矩、转速、角度等遥测数据，最终输出 PASS/FAIL 判定结果。

### 核心功能

- **自动化测试流程**：找零 → 空载正反转 → 角度定位（3 位置） → 负载锁止 → 回零，全流程无人值守
- **实时遥测监控**：30Hz 采样电机电流、扭矩/转速/功率、编码器角度、制动电流
- **配方管理**：通过 JSON 配置文件灵活定义测试参数和判定限值
- **多设备通信**：4 路 Modbus RTU 串口并行控制，支持不同波特率
- **判定与分类**：三类失败分类（通信故障 / 流程异常 / 数据超限），便于现场快速排障
- **现代工业 UI**：基于 shadcn 风格的 QML 组件库，支持主题切换

## 系统架构

```
┌─────────────────────────────────────────────────────────────┐
│                         QML UI Layer                         │
│  (TestExecutionPage, CommandBar, MetricCard, VerdictPanel)  │
└────────────────────────┬────────────────────────────────────┘
                         │ Q_PROPERTY bindings
                         ▼
┌─────────────────────────────────────────────────────────────┐
│                      ViewModel Layer                         │
│              (TestExecutionViewModel)                        │
└────────────────────────┬────────────────────────────────────┘
                         │ signals/slots
                         ▼
┌─────────────────────────────────────────────────────────────┐
│                       Domain Layer                           │
│              (GearboxTestEngine - 33ms cycle)                │
│  Homing → Idle → Angle → Load → Return → Complete/Failed    │
└────────────────────────┬────────────────────────────────────┘
                         │ device interfaces
                         ▼
┌─────────────────────────────────────────────────────────────┐
│                   Infrastructure Layer                       │
│  Devices: AQMD, DYN200, Encoder, BrakePower                │
│  Bus: ModbusRtuBusController (0x03/0x06)                   │
│  Config: StationRuntimeFactory                               │
└────────────────────────┬────────────────────────────────────┘
                         │ Qt SerialPort
                         ▼
                  ┌──────────────┐
                  │ Serial Ports │
                  │  (Hardware)  │
                  └──────────────┘
```

### 分层说明

| 层级 | 职责 | 关键文件 |
|------|------|----------|
| **UI Layer** | QML 界面渲染与用户交互 | `src/ui/pages/`, `src/ui/components/` |
| **ViewModel Layer** | UI 与业务逻辑的桥梁，属性绑定 | `src/viewmodels/TestExecutionViewModel` |
| **Domain Layer** | 测试状态机与业务规则 | `src/domain/GearboxTestEngine` |
| **Infrastructure Layer** | 硬件通信、设备驱动、配置加载 | `src/infrastructure/` |

## 硬件设备

系统通过 4 路 RS-485 串口连接以下设备：

| 设备 | 型号 | 功能 | 通信协议 |
|------|------|------|----------|
| 电机驱动 | AQMD3610NS-A2 | 驱动齿轮箱电机，读取电流与 AI1 磁铁信号 | Modbus RTU (0x03/0x06) |
| 扭矩传感器 | DYN200 | 测量扭矩 (N·m)、转速 (RPM)、功率 (W) | Modbus RTU (0x03) |
| 单圈编码器 | 通用 Modbus 编码器 | 绝对角度测量 (0°~360°)，支持零点设置 | Modbus RTU (0x03/0x06) |
| 数控电源 | 双通道数控电源 | 提供可编程制动电流 | Modbus RTU (0x03/0x06) |

## 测试流程

系统按以下阶段自动执行测试：

```
┌─────────┐    ┌──────────┐    ┌──────────────┐    ┌────────────┐    ┌───────────┐
│  找零    │ →  │ 空载正转  │ →  │ 空载反转     │ →  │ 角度定位    │ →  │ 负载正转   │
│ (磁铁    │    │ (采样    │    │ (采样        │    │ (3位置     │    │ (制动爬升  │
│  检测)   │    │  判定)   │    │  判定)       │    │  测量)     │    │  锁止检测) │
└─────────┘    └──────────┘    └──────────────┘    └────────────┘    └───────────┘
                                                                      │
┌───────────┐    ┌──────────┐    ┌──────────────┐                      │
│  回零     │ ←  │ 负载反转  │ ←  │ 结果判定     │ ←────────────────────┘
│  完成     │    │ (制动    │    │ (PASS/FAIL)  │
│           │    │  锁止)   │    │              │
└───────────┘    └──────────┘    └──────────────┘
```

### 各阶段说明

1. **找零 (Homing)**：低速驱动电机，通过 AI1 磁铁信号检测零位，然后推进到编码器零点
2. **空载正反转 (Idle Run)**：分别正转和反转，在采样窗口内采集电流和转速，与配方限值比较
3. **角度定位 (Angle Positioning)**：驱动到 3 个预设角度位置，测量实际角度偏差
4. **负载测试 (Load Test)**：正转和反转下逐步增加制动电流，检测锁止状态并记录电流和扭矩
5. **回零 (Return to Zero)**：回到零位，完成测试

## 目录结构

```
FlipGearboxFactoryTest/
├── CMakeLists.txt              # CMake 构建配置
├── main.cpp                    # 应用入口，初始化运行时与 ViewModel
├── Main.qml                    # 主窗口
├── config/
│   ├── station.json            # 工位配置（串口、设备参数）
│   └── recipes/
│       └── GBX-42A.json        # 测试配方（参数与限值）
├── assets/
│   └── fonts/                  # HarmonyOS Sans SC 字体
├── src/
│   ├── domain/                 # 领域层：测试引擎与数据模型
│   │   ├── GearboxTestEngine   # 测试状态机（核心）
│   │   ├── TestRecipe          # 配方数据结构
│   │   ├── TestResults         # 测试结果数据结构
│   │   ├── TestRunState        # 测试阶段/子状态枚举
│   │   ├── TelemetrySnapshot   # 遥测快照
│   │   └── FailureReason       # 失败原因与分类
│   ├── viewmodels/             # ViewModel 层
│   │   └── TestExecutionViewModel
│   ├── infrastructure/         # 基础设施层
│   │   ├── bus/                # Modbus RTU 总线控制
│   │   │   ├── IBusController  # 总线抽象接口
│   │   │   ├── ModbusRtuBusController
│   │   │   └── ModbusFrame
│   │   ├── devices/            # 设备驱动
│   │   │   ├── IMotorDriveDevice / AqmdMotorDriveDevice
│   │   │   ├── ITorqueSensorDevice / Dyn200TorqueSensorDevice
│   │   │   ├── IEncoderDevice / SingleTurnEncoderDevice
│   │   │   └── IBrakePowerDevice / BrakePowerSupplyDevice
│   │   └── config/             # 配置管理
│   │       ├── ConfigLoader
│   │       ├── DeviceConfigService
│   │       ├── StationRuntime / StationRuntimeFactory
│   │       └── RecipeConfig
│   └── ui/
│       ├── pages/              # 页面
│       │   ├── AppShell        # 主 Shell（导航 + 页面栈）
│       │   ├── TestExecutionPage
│       │   ├── RecipePage
│       │   ├── HistoryPage
│       │   ├── DeviceConfigPage
│       │   ├── DiagnosticsPage
│       │   └── ComponentGalleryPage
│       └── components/         # UI 组件库（50+ 组件）
├── tests/
│   └── ui/
│       └── QmlSmokeTests       # QML Smoke 测试
└── Docs/                       # 设计文档与设备参考手册
```

## 构建与运行

### 前提条件

| 依赖 | 最低版本 | 说明 |
|------|----------|------|
| Qt | 6.10+ | 需要 Quick、QuickControls2、SerialPort、Test 模块 |
| CMake | 3.16+ | 构建系统 |
| C++ 编译器 | C++20 | MinGW (LLVM/Clang) 或 MSVC |
| 操作系统 | Windows | 目标平台（使用 COM 串口） |

### 构建步骤

```powershell
# 配置（使用 Qt Creator 或命令行）
cmake -S . -B build -G "MinGW Makefiles"

# 构建
cmake --build build

# 运行
.\build\appFlipGearboxFactoryTest.exe
```

### 运行测试

```powershell
ctest --test-dir build --output-on-failure
```

当前测试为 QML Smoke Tests，验证 UI 组件的加载、导航、交互等基本功能。

## 配置说明

### 工位配置 (`config/station.json`)

定义工位硬件连接参数：

```json
{
  "stationId": "STATION-01",
  "stationName": "Gearbox Test Station 1",
  "brakeChannel": 1,
  "defaultRecipe": "GBX-42A",
  "aqmd":    { "portName": "COM3", "slaveId": 1, "baudRate": 9600 },
  "dyn200":  { "portName": "COM4", "slaveId": 2, "baudRate": 9600 },
  "encoder": { "portName": "COM5", "slaveId": 3, "baudRate": 9600, "resolution": 4096 },
  "brake":   { "portName": "COM6", "slaveId": 4, "baudRate": 9600 }
}
```

### 配方配置 (`config/recipes/*.json`)

定义测试参数和判定限值，关键参数包括：

| 参数组 | 关键字段 | 说明 |
|--------|----------|------|
| 找零 | `homeDutyCycle`, `homeTimeoutMs` | 找零占空比与超时 |
| 空载 | `idleDutyCycle`, `idleForwardSpeedAvgMin/Max` | 空载测试占空比与转速/电流限值 |
| 角度 | `position1TargetDeg`, `position1ToleranceDeg` | 目标角度与容差 |
| 负载 | `brakeRampEndCurrentA`, `loadForwardTorqueMin/Max` | 制动电流与锁止扭矩限值 |
| 锁止检测 | `lockSpeedThresholdRpm`, `lockAngleDeltaDeg`, `lockHoldMs` | 锁止判定条件 |

## 关键设计决策

| 决策 | 说明 |
|------|------|
| 磁铁事件检测 | 只识别 AI1 高→低跳变，防止重复触发 |
| 角度判定 | 基于磁铁事件时刻锁存的编码器角度，而非接近目标角度 |
| 回差补偿 | 只在初始找零阶段执行一次，后续不重复 |
| 锁止检测 | 双条件（转速 ≤2 RPM + 角度变化 ≤5°/100ms）+ 500ms 保持 |
| 失败分类 | 通信/流程/判定三类，便于现场排障 |
| 轮询周期 | 33ms（30Hz）用于 AQMD/编码器，50ms（20Hz）用于 DYN200 |
| 设备接口抽象 | 所有设备通过纯虚接口访问，便于测试和替换 |

## 技术栈

- **语言**：C++20 / QML (Qt Quick)
- **框架**：Qt 6.10 (Quick, QuickControls2, SerialPort, Test)
- **构建**：CMake 3.16+
- **通信**：Modbus RTU over RS-485 (Qt SerialPort)
- **字体**：HarmonyOS Sans SC
- **架构模式**：MVVM (Model-View-ViewModel)

## 参考文档

| 文档 | 路径 |
|------|------|
| 真机测试流程设计 | `Docs/superpowers/specs/2026-04-17-real-device-test-flow-design.md` |
| 设备寄存器修正 | `Docs/superpowers/specs/2026-04-17-device-registers-correction.md` |
| AQMD3610NS-A2 驱动器手册 | `Docs/AQMD3610NS-A2.md` |
| DYN200 扭矩传感器手册 | `Docs/DYN-200使用手册印刷V3.7版本中性(12)(3)_2025-08-02-13_23_22.md` |
| 单圈编码器说明 | `Docs/单圈编码器.md` |
| 双通道数控电源手册 | `Docs/双通道数控电源用户手册.md` |
| 实现进度 | `IMPLEMENTATION_PROGRESS.md` |

## 许可证

内部项目，未指定许可证。
