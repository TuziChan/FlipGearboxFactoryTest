# FlipGearboxFactoryTest

**齿轮箱工厂测试系统** - 基于 Qt6 的工业自动化测试平台

## 项目简介

FlipGearboxFactoryTest 是一个用于齿轮箱质量检测的自动化测试系统,提供:

- **多设备协同控制**: 电机驱动、扭矩传感器、编码器、制动电源
- **Modbus RTU 通信**: RS485 工业通信协议
- **完整测试流程**: 找零、空载、角度定位、负载测试
- **实时数据采集**: 高频遥测数据和可视化
- **Mock 模式仿真**: 无硬件开发和测试

## 技术栈

- **UI 框架**: Qt 6.11 (QML + Qt Quick)
- **编程语言**: C++20
- **构建系统**: CMake 3.16+
- **通信协议**: Modbus RTU (RS485)
- **架构模式**: MVVM (Model-View-ViewModel)

## 快速开始

### 环境要求

- Qt 6.11+ (包含 Qt Quick 和 Qt SerialPort 模块)
- CMake 3.16+
- MinGW 13.1.0+ (Windows) 或 GCC/Clang (Linux)
- 支持 C++20 的编译器

### 构建

```bash
# 配置
cmake -S . -B build -G "MinGW Makefiles"

# 编译
cmake --build build

# 运行
.\build\appFlipGearboxFactoryTest.exe
```

### 运行测试

```bash
# 运行所有测试
ctest --test-dir build --output-on-failure

# 运行特定测试
.\build\DomainEngineTests.exe
```

## 架构设计

```
┌─────────────────────────────────────────────────────────┐
│                   UI 层 (QML)                           │
│  TestExecutionPage │ RecipePage │ DiagnosticsPage      │
└────────────────────────┬────────────────────────────────┘
                         │
┌────────────────────────┴────────────────────────────────┐
│                ViewModel 层 (C++)                       │
│  TestExecutionViewModel │ RecipeViewModel │ ...         │
└────────────────────────┬────────────────────────────────┘
                         │
┌────────────────────────┴────────────────────────────────┐
│                  Domain 层 (C++)                        │
│  GearboxTestEngine (33ms 周期, 状态机)                  │
│  TestRecipe │ TestResults │ TelemetrySnapshot           │
└────────────────────────┬────────────────────────────────┘
                         │
┌────────────────────────┴────────────────────────────────┐
│             Infrastructure 层 (C++)                     │
│  ┌──────────────────────────────────────────────────┐  │
│  │ 设备实现                                         │  │
│  │  AqmdMotorDriveDevice │ Dyn200TorqueSensorDevice │  │
│  │  SingleTurnEncoderDevice │ BrakePowerSupplyDevice│  │
│  └──────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────┐  │
│  │ 总线通信                                         │  │
│  │  ModbusRtuBusController │ ModbusFrame            │  │
│  └──────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────┐  │
│  │ 配置与服务                                       │  │
│  │  StationRuntime │ ConfigLoader │ RecipeService   │  │
│  └──────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

## 支持的设备

| 设备 | 型号 | 协议 | 功能 |
|------|------|------|------|
| 电机驱动 | AQMD3610NS-A2 | Modbus RTU | 速度控制、磁铁检测 |
| 扭矩传感器 | DYN200 | Modbus RTU | 扭矩、转速、功率测量 |
| 编码器 | 单圈绝对值 | Modbus RTU | 角度定位 |
| 制动电源 | 双通道数控电源 | Modbus RTU | 负载模拟 |

## 测试流程

1. **找零阶段**: 检测磁铁位置并设置编码器零点
2. **空载阶段**: 低速稳定运行,验证基线扭矩
3. **角度阶段**: 定位到目标角度,验证精度
4. **负载阶段**: 施加制动负载,测量扭矩和锁死检测
5. **返回阶段**: 返回零点位置

## 项目状态

**当前进度**: 约 70% 完成

✅ **已完成**:
- 基础设施层 (总线、设备、配置)
- 领域层 (测试引擎、状态机)
- ViewModel 层 (MVVM 绑定)
- Mock/仿真框架
- 完整测试套件

🚧 **进行中**:
- 硬件验证
- 报告生成
- 日志系统

详细状态见 [IMPLEMENTATION_PROGRESS.md](IMPLEMENTATION_PROGRESS.md)

## 文档

- **[用户手册](Docs/USER_MANUAL.md)** - 操作指南
- **[部署指南](Docs/DEPLOYMENT_GUIDE.md)** - 安装和配置
- **[故障排查](Docs/TROUBLESHOOTING.md)** - 常见问题
- **[Mock 模式指南](Docs/MOCK_MODE_GUIDE.md)** - 仿真使用
- **[设备寄存器参考](Docs/Device_Register_Reference_Guide.md)** - 硬件规格

## 配置

### 工站配置

编辑 `config/station.json`:

```json
{
  "stationId": "STATION-001",
  "devices": {
    "motor": {
      "type": "AQMD3610NS-A2",
      "slaveId": 1,
      "serialPort": "COM3"
    },
    "torqueSensor": {
      "type": "DYN200",
      "slaveId": 2,
      "serialPort": "COM4"
    }
  }
}
```

### 配方配置

编辑 `config/recipes/GBX-42A.json`:

```json
{
  "recipeId": "GBX-42A",
  "homingSpeed": 50,
  "idleSpeed": 100,
  "targetAngle": 180.0,
  "loadTorque": 5.0,
  "lockThreshold": 0.5
}
```

## 开发

### 代码结构

```
src/
├── domain/              # 业务逻辑 (测试引擎、配方)
├── infrastructure/      # 技术实现
│   ├── bus/            # Modbus RTU 通信
│   ├── devices/        # 设备驱动
│   ├── config/         # 配置系统
│   ├── simulation/     # Mock 设备
│   └── services/       # 应用服务
├── viewmodels/         # MVVM 视图模型
└── ui/                 # QML 组件和页面
```

### Mock 模式运行

当真实硬件不可用时,应用程序自动使用 mock 设备:

```cpp
// 在 main.cpp 中
auto runtime = StationRuntimeFactory::create(config);
// 串口不可用时自动回退到仿真模式
```

### 添加新设备

1. 在 `src/infrastructure/devices/IYourDevice.h` 定义接口
2. 在 `src/infrastructure/devices/YourDevice.cpp` 实现驱动
3. 在 `src/infrastructure/simulation/MockYourDevice.cpp` 添加 mock
4. 在 `StationRuntimeFactory` 中注册

## 测试

### 测试分类

- **单元测试**: 领域逻辑、协议解析
- **集成测试**: 设备通信、状态机
- **UI 测试**: QML 组件渲染
- **仿真测试**: Mock 设备行为

### 测试覆盖

```bash
# 运行所有测试
cmake --build build --target run-all-tests

# 运行特定测试套件
.\build\DomainEngineTests.exe
.\build\ProtocolLayerTests.exe
.\build\SimulationRuntimeTests.exe
```

## 故障排查

### 串口访问

**Windows**: 确保 COM 口未被其他应用占用

**Linux**: 将用户添加到 `dialout` 组:
```bash
sudo usermod -a -G dialout $USER
```

### 构建问题

**找不到 Qt**: 设置 `CMAKE_PREFIX_PATH`:
```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=F:/Qt/6.11.0/mingw_64
```

**MinGW 链接错误**: 确保 MinGW bin 目录在 PATH 中

更多解决方案见 [TROUBLESHOOTING.md](Docs/TROUBLESHOOTING.md)

## 许可证

[待确定]

## 贡献

欢迎贡献! 请确保:
- 代码遵循 C++20 标准
- 提交 PR 前所有测试通过
- QML 组件遵循 AppTheme 约定
- 设备实现包含 mock 对应物

---

**项目版本**: 0.1  
**最后更新**: 2026-04-24
