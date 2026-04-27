# Mock 设备模拟器使用指南

## 概述

Mock 设备模拟器提供了完整的多串口 Modbus RTU 设备仿真环境，用于测试上位机软件的所有功能，无需真实硬件。

## 架构

```
MockDeviceManager (管理器)
    └── MockSerialBusController (多串口总线控制器)
            ├── COM1 → MockMotorDevice (电机驱动器, slave 1)
            ├── COM2 → MockTorqueDevice (扭矩传感器, slave 2)
            ├── COM3 → MockEncoderDevice (编码器, slave 3)
            └── COM4 → MockBrakeDevice (制动器, slave 4)
```

## 核心组件

### 1. MockModbusDevice (基类)
- 实现完整的 Modbus RTU 协议处理
- 支持功能码: 0x01, 0x03, 0x04, 0x05, 0x06, 0x10
- 提供错误注入机制 (CRC 错误、超时、延迟、异常响应)
- 管理寄存器存储 (Holding/Input Registers, Coils)

### 2. MockMotorDevice (电机驱动器)
**寄存器映射:**
- `0x0000`: 设备 ID (只读)
- `0x0011`: 实时电流 (×0.01A, 只读)
- `0x0040`: 设置速度 (-1000 to 1000, ×0.1%)
- `0x0042`: 停止并锁定 (写 1 制动)
- `0x0044`: 自然停止 (写 1 滑行)
- `0x0050`: AI1 端口方向 (0=输入, 1=输出)
- `0x0052`: AI1 端口电平 (0=低, 1=高)

**特性:**
- 模拟电流与速度成正比
- 支持磁铁检测信号 (AI1 输入)

### 3. MockTorqueDevice (扭矩传感器)
**寄存器映射 (32位大端):**
- `0x0000-0x0001`: 扭矩 (×0.01 N·m)
- `0x0002-0x0003`: 转速 (×1 RPM)
- `0x0004-0x0005`: 功率 (×0.1 W)
- `0x001C`: 通信模式 (0=Modbus RTU)

**特性:**
- 32位数据大端存储
- 支持动态更新扭矩、转速、功率

### 4. MockEncoderDevice (编码器)
**寄存器映射:**
- `0x0000`: 单圈原始计数 (0 to resolution)
- `0x0000-0x0001`: 虚拟多圈计数 (32位)
- `0x0003`: 角速度 (有符号 16位 RPM)
- `0x0006`: 自动上报模式
- `0x0007`: 自动上报间隔 (ms)
- `0x0008`: 设置零点 (写 1)

**特性:**
- 分辨率可配置 (默认 4096)
- 角度转换: `angleDeg = (count / resolution) × 360°`
- 支持零点设置

### 5. MockBrakeDevice (制动器)
**寄存器映射:**

Holding Registers (0x03/0x06):
- `0x0000`: CH1 设置电压 (×0.01V)
- `0x0001`: CH1 设置电流 (×0.01A)
- `0x0002`: CH2 设置电压 (×0.01V)
- `0x0003`: CH2 设置电流 (×0.01A)

Input Registers (0x04):
- `0x0000`: CH1 实际电压 (×0.01V)
- `0x0001`: CH1 实际电流 (×0.01A)
- `0x0002`: CH1 实际功率 (×0.1W)
- `0x0003`: CH2 实际电压 (×0.01V)
- `0x0005`: CH2 实际电流 (×0.01A)
- `0x0006`: CH2 实际功率 (×0.1W)
- `0x0009`: 模式寄存器 (bit 0=CH1, bit 1=CH2, 0=CC, 1=CV)

Coils (0x01/0x05):
- `0x0000`: CH1 输出使能
- `0x0001`: CH2 输出使能

**特性:**
- 双通道独立控制
- 支持 CC/CV 模式
- 输出使能控制

## 使用示例

### 基本使用

```cpp
#include "src/infrastructure/simulation/MockDeviceManager.h"
#include "src/infrastructure/devices/AqmdMotorDriveDevice.h"

// 1. 创建管理器并初始化设备
auto* manager = new MockDeviceManager();
manager->initializeDefaultDevices();

// 2. 获取总线控制器
auto* busController = manager->busController();

// 3. 创建设备驱动
auto* motorDriver = new AqmdMotorDriveDevice(busController, 1);

// 4. 打开串口
busController->open("COM1", 9600, 1000, "Even", 1);

// 5. 初始化设备
motorDriver->initialize();

// 6. 控制电机
motorDriver->setMotor(IMotorDriveDevice::Direction::Forward, 50.0);

// 7. 读取电流
double currentA = 0.0;
motorDriver->readCurrent(currentA);
qDebug() << "Motor current:" << currentA << "A";

// 8. 关闭串口
busController->close();
```

### 角度测试场景 (磁铁检测)

```cpp
// 磁铁位置: 3°, 49°, 113°
// 检测窗口: ±2°

// 模拟角度并检测磁铁
manager->simulateAngleTestScenario(3.0);  // 在第一个磁铁位置

// 读取编码器角度
busController->open("COM3", 9600, 1000, "None", 1);
double angle = 0.0;
encoder->readAngle(angle);

// 读取磁铁检测信号
busController->open("COM1", 9600, 1000, "Even", 1);
bool magnetDetected = false;
motorDriver->readAI1Level(magnetDetected);

qDebug() << "Angle:" << angle << "° Magnet:" << (magnetDetected ? "YES" : "NO");
```

### 电机负载模拟

```cpp
// 模拟电机以 50% 速度运行，负载扭矩 5.0 N·m
manager->simulateMotorWithLoad(50.0, 5.0);

// 读取扭矩传感器数据
busController->open("COM2", 19200, 1000, "None", 2);
double torque = 0.0, speed = 0.0, power = 0.0;
torqueSensor->readAll(torque, speed, power);

qDebug() << "Torque:" << torque << "Nm";
qDebug() << "Speed:" << speed << "RPM";
qDebug() << "Power:" << power << "W";
```

### 错误注入测试

```cpp
// 启用错误注入
// 参数: CRC错误率, 超时率, 延迟率
manager->enableErrorInjection(0.1, 0.05, 0.1);  // 10% CRC, 5% 超时, 10% 延迟

// 执行通信测试
for (int i = 0; i < 100; ++i) {
    busController->open("COM3", 9600, 1000, "None", 1);
    double angle = 0.0;
    bool success = encoder->readAngle(angle);
    
    if (!success) {
        qDebug() << "Error:" << encoder->lastError();
    }
    busController->close();
}

// 禁用错误注入
manager->disableErrorInjection();
```

### 高速数据采集测试

```cpp
// 模拟高速采集: 1000ms 持续时间, 100Hz 更新率
manager->simulateHighSpeedAcquisition(1000, 100);
```

### 多设备并发测试

```cpp
// 快速切换多个设备
for (int i = 0; i < 20; ++i) {
    // 读取电机电流
    busController->open("COM1", 9600, 500, "Even", 1);
    double current = 0.0;
    motorDriver->readCurrent(current);
    busController->close();
    
    // 读取扭矩
    busController->open("COM2", 19200, 500, "None", 2);
    double torque = 0.0;
    torqueSensor->readTorque(torque);
    busController->close();
    
    // 读取角度
    busController->open("COM3", 9600, 500, "None", 1);
    double angle = 0.0;
    encoder->readAngle(angle);
    busController->close();
}
```

## 测试场景

### 1. 基本通信测试
- 验证所有设备的初始化
- 验证读写操作
- 验证数据正确性

### 2. 角度磁铁检测测试
- 测试 3°, 49°, 113° 磁铁位置
- 验证检测窗口 (±2°)
- 测试边界条件和环绕

### 3. 高速采集测试
- 100+ Hz 数据采集
- 多设备并发读取
- 性能和稳定性验证

### 4. 错误注入测试
- CRC 错误恢复
- 超时处理
- 延迟响应处理
- 异常响应处理

### 5. 数据链路阻塞测试
- 快速设备切换
- 并发请求处理
- 队列管理

### 6. 边界条件测试
- 零值测试
- 最大值测试
- 负值测试
- 环绕测试 (360°)

## 运行完整测试套件

```cpp
#include "src/infrastructure/simulation/MockDeviceTestExample.h"

auto* testExample = new MockDeviceTestExample();

// 连接信号
QObject::connect(testExample, &MockDeviceTestExample::testCompleted,
    [](const QString& testName, bool success, const QString& message) {
        qDebug() << (success ? "[PASS]" : "[FAIL]") << testName << "-" << message;
    });

QObject::connect(testExample, &MockDeviceTestExample::allTestsCompleted,
    [](int passed, int failed) {
        qDebug() << "Tests completed. Passed:" << passed << "Failed:" << failed;
    });

// 运行所有测试
testExample->runAllTests();
```

## 错误注入配置

```cpp
MockModbusDevice::ErrorInjectionConfig config;
config.enabled = true;
config.crcErrorRate = 0.1;      // 10% CRC 错误
config.timeoutRate = 0.05;      // 5% 超时
config.delayRate = 0.1;         // 10% 延迟
config.minDelayMs = 100;        // 最小延迟 100ms
config.maxDelayMs = 500;        // 最大延迟 500ms
config.exceptionRate = 0.02;    // 2% 异常响应

// 应用到所有设备
manager->busController()->setGlobalErrorInjection(config);

// 或应用到特定端口
manager->busController()->setPortErrorInjection("COM1", config);
```

## 设备配置

### 电机驱动器 (COM1)
- Slave ID: 1
- 波特率: 9600
- 校验: Even
- 停止位: 1

### 扭矩传感器 (COM2)
- Slave ID: 2
- 波特率: 19200
- 校验: None
- 停止位: 2

### 编码器 (COM3)
- Slave ID: 3
- 波特率: 9600
- 校验: None
- 停止位: 1
- 分辨率: 4096

### 制动器 (COM4)
- Slave ID: 4
- 波特率: 9600
- 校验: None
- 停止位: 1

## 注意事项

1. **线程安全**: MockSerialBusController 使用 QMutex 保护并发访问
2. **内存管理**: 设备注册后由 MockSerialBusController 管理生命周期
3. **串口切换**: 每次操作前需要 open() 对应端口，操作后 close()
4. **错误处理**: 所有操作返回 bool，失败时通过 lastError() 获取错误信息
5. **数据缩放**: 注意各设备的数据缩放因子 (见寄存器映射)

## 调试技巧

1. **启用详细日志**:
```cpp
qSetMessagePattern("[%{time}] %{type}: %{message}");
```

2. **单步测试**: 先测试单个设备，再测试多设备并发

3. **错误率调整**: 从低错误率开始 (1-5%)，逐步增加

4. **性能监控**: 使用 QElapsedTimer 测量操作耗时

## 文件清单

- `MockModbusDevice.h/cpp` - Modbus 设备基类
- `MockMotorDevice.h/cpp` - 电机驱动器模拟
- `MockTorqueDevice.h/cpp` - 扭矩传感器模拟
- `MockEncoderDevice.h/cpp` - 编码器模拟
- `MockBrakeDevice.h/cpp` - 制动器模拟
- `MockSerialBusController.h/cpp` - 多串口总线控制器
- `MockDeviceManager.h/cpp` - 设备管理器
- `MockDeviceTestExample.h/cpp` - 测试示例

## 扩展

### 添加新设备

1. 继承 `MockModbusDevice`
2. 实现 `onRegisterWrite()` 和 `updateDynamicRegisters()`
3. 在 `MockDeviceManager` 中注册

### 自定义错误场景

```cpp
// 创建自定义错误配置
MockModbusDevice::ErrorInjectionConfig customConfig;
customConfig.enabled = true;
customConfig.crcErrorRate = 0.5;  // 50% CRC 错误
customConfig.timeoutRate = 0.0;
customConfig.delayRate = 0.0;

// 应用到特定设备
manager->busController()->setPortErrorInjection("COM2", customConfig);
```

## 总结

Mock 设备模拟器提供了完整的硬件仿真环境，支持:
- ✅ 多串口并发通信
- ✅ 完整的 Modbus RTU 协议
- ✅ 错误注入和异常场景
- ✅ 高速数据采集测试
- ✅ 角度磁铁检测场景
- ✅ 数据链路阻塞测试
- ✅ 边界条件和异常数据处理

使用 Mock 设备可以在没有真实硬件的情况下，全面测试上位机软件的功能和稳定性。
