# Mock 设备模拟器实现总结

## 任务完成情况

✅ **已完成**: 实现多串口 Mock 设备模拟器，支持多 COM 口串口通信测试

## 实现的组件

### 1. 核心设备模拟器

#### MockModbusDevice (基类)
- **文件**: `MockModbusDevice.h/cpp`
- **功能**:
  - 完整的 Modbus RTU 协议处理
  - 支持功能码: 0x01, 0x03, 0x04, 0x05, 0x06, 0x10
  - CRC16 校验和验证
  - 寄存器存储管理 (Holding/Input Registers, Coils)
  - 错误注入机制 (CRC 错误、超时、延迟、异常响应)

#### MockMotorDevice (电机驱动器)
- **文件**: `MockMotorDevice.h/cpp`
- **Slave ID**: 1
- **寄存器映射**:
  - 0x0000: 设备 ID
  - 0x0011: 实时电流 (×0.01A)
  - 0x0040: 设置速度 (-1000~1000)
  - 0x0042: 停止并锁定
  - 0x0044: 自然停止
  - 0x0050: AI1 端口方向
  - 0x0052: AI1 端口电平 (磁铁检测)
- **特性**: 模拟电流与速度成正比，支持磁铁检测信号

#### MockTorqueDevice (扭矩传感器)
- **文件**: `MockTorqueDevice.h/cpp`
- **Slave ID**: 2
- **寄存器映射** (32位大端):
  - 0x0000-0x0001: 扭矩 (×0.01 N·m)
  - 0x0002-0x0003: 转速 (×1 RPM)
  - 0x0004-0x0005: 功率 (×0.1 W)
  - 0x001C: 通信模式
- **特性**: 32位数据大端存储，符合 DYN200 规范

#### MockEncoderDevice (编码器)
- **文件**: `MockEncoderDevice.h/cpp`
- **Slave ID**: 3
- **寄存器映射**:
  - 0x0000: 单圈原始计数
  - 0x0000-0x0001: 虚拟多圈计数 (32位)
  - 0x0003: 角速度
  - 0x0006: 自动上报模式
  - 0x0007: 自动上报间隔
  - 0x0008: 设置零点
- **特性**: 分辨率 4096，支持零点设置，角度转换公式正确

#### MockBrakeDevice (制动器)
- **文件**: `MockBrakeDevice.h/cpp`
- **Slave ID**: 4
- **寄存器映射**:
  - Holding Registers: 电压/电流设置值
  - Input Registers: 电压/电流/功率读回值
  - Coils: 输出使能控制
- **特性**: 双通道独立控制，支持 CC/CV 模式

### 2. 总线控制器

#### MockSerialBusController
- **文件**: `MockSerialBusController.h/cpp`
- **功能**:
  - 管理多个虚拟串口 (COM1-COM4)
  - 每个端口绑定一个 Mock 设备
  - 模拟波特率传输延迟
  - 线程安全 (QMutex 保护)
  - 支持全局和端口级错误注入
- **特性**: 实现 IBusController 接口，可无缝替换真实总线控制器

### 3. 管理器

#### MockDeviceManager
- **文件**: `MockDeviceManager.h/cpp`
- **功能**:
  - 一键初始化所有设备
  - 角度测试场景模拟 (磁铁位置: 3°, 49°, 113°)
  - 电机负载模拟
  - 错误注入控制
  - 高速数据采集模拟
  - 设备状态重置
- **特性**: 提供高级 API，简化测试代码编写

### 4. 测试套件

#### MockDeviceTestExample
- **文件**: `MockDeviceTestExample.h/cpp`
- **测试场景**:
  1. 基本多设备通信测试
  2. 角度磁铁检测测试 (3°, 49°, 113°)
  3. 高速并发数据采集测试 (100+ Hz)
  4. 错误注入和恢复测试
  5. 数据链路阻塞测试
  6. 边界条件和异常数据测试
- **特性**: 完整的自动化测试套件，覆盖所有关键场景

#### MockDeviceQuickTest
- **文件**: `tests/simulation/MockDeviceQuickTest.cpp`
- **功能**: 快速验证 Mock 设备基本功能

## 关键特性

### ✅ 多串口并发通信
- 支持 4 个独立串口 (COM1-COM4)
- 每个串口独立配置波特率、校验位、停止位
- 线程安全的并发访问

### ✅ Modbus RTU 协议完整实现
- 功能码: 0x01, 0x03, 0x04, 0x05, 0x06, 0x10
- CRC16 校验
- 异常响应处理
- 寄存器地址映射与真实设备一致

### ✅ 错误注入机制
- CRC 错误注入 (可配置概率)
- 超时模拟 (无响应)
- 延迟响应 (可配置延迟范围)
- 异常响应 (Modbus 异常码)

### ✅ 角度磁铁检测场景
- 磁铁位置: 3°, 49°, 113°
- 检测窗口: ±2°
- 考虑前面磁铁位置的影响
- 支持 360° 环绕处理

### ✅ 高速数据采集支持
- 支持 100+ Hz 更新率
- 多设备并发读取
- 波特率传输延迟模拟

### ✅ 数据链路阻塞测试
- 快速设备切换
- 并发请求处理
- 超时和重试机制验证

### ✅ 边界和异常数据处理
- 零值测试
- 最大值测试
- 负值测试
- 360° 环绕测试
- 数据溢出测试

## 设备配置

| 设备 | 端口 | Slave ID | 波特率 | 校验 | 停止位 |
|------|------|----------|--------|------|--------|
| 电机驱动器 | COM1 | 1 | 9600 | Even | 1 |
| 扭矩传感器 | COM2 | 2 | 19200 | None | 2 |
| 编码器 | COM3 | 3 | 9600 | None | 1 |
| 制动器 | COM4 | 4 | 9600 | None | 1 |

## 使用示例

```cpp
// 1. 创建管理器
auto* manager = new MockDeviceManager();
manager->initializeDefaultDevices();

// 2. 创建设备驱动
auto* busController = manager->busController();
auto* motorDriver = new AqmdMotorDriveDevice(busController, 1);

// 3. 打开串口并通信
busController->open("COM1", 9600, 1000, "Even", 1);
motorDriver->initialize();
motorDriver->setMotor(IMotorDriveDevice::Direction::Forward, 50.0);
busController->close();

// 4. 角度测试场景
manager->simulateAngleTestScenario(49.0);  // 在第二个磁铁位置

// 5. 错误注入测试
manager->enableErrorInjection(0.1, 0.05, 0.1);  // 10% CRC, 5% 超时, 10% 延迟
```

## 文档

- **使用指南**: `docs/MockDeviceSimulator.md`
  - 详细的 API 文档
  - 完整的使用示例
  - 测试场景说明
  - 错误注入配置
  - 调试技巧

## 测试验证

### 运行测试
```bash
# 编译项目
cmake --build build

# 运行快速测试
./build/MockDeviceQuickTest

# 运行完整测试套件
# (在代码中调用 MockDeviceTestExample::runAllTests())
```

### 测试覆盖
- ✅ 基本通信功能
- ✅ 所有 Modbus 功能码
- ✅ 角度磁铁检测 (3°, 49°, 113°)
- ✅ 高速并发采集
- ✅ 错误注入和恢复
- ✅ 数据链路阻塞
- ✅ 边界条件

## 技术亮点

1. **架构设计**: 清晰的分层架构，基类提供通用功能，子类实现设备特性
2. **线程安全**: 使用 QMutex 保护并发访问
3. **内存管理**: 设备生命周期由管理器统一管理
4. **错误处理**: 所有操作返回 bool，失败时提供详细错误信息
5. **可扩展性**: 易于添加新设备类型
6. **真实性**: 寄存器映射、数据缩放因子与真实设备完全一致

## 集成到项目

所有文件已添加到 `CMakeLists.txt`:
- 主应用程序包含所有 Mock 设备文件
- 测试可执行文件可以使用 Mock 设备
- 无需修改现有代码即可使用

## 后续建议

1. **性能测试**: 在高负载下测试多设备并发性能
2. **压力测试**: 长时间运行测试，验证稳定性
3. **集成测试**: 将 Mock 设备集成到完整的测试流程中
4. **文档完善**: 根据实际使用情况补充更多示例

## 总结

Mock 设备模拟器已完整实现，支持:
- ✅ 4 个设备的完整模拟 (电机、扭矩、编码器、制动器)
- ✅ 多串口并发通信
- ✅ 完整的 Modbus RTU 协议
- ✅ 错误注入和异常场景
- ✅ 角度磁铁检测测试 (3°, 49°, 113°)
- ✅ 高速数据采集测试
- ✅ 数据链路阻塞测试
- ✅ 边界和异常数据处理

可以在没有真实硬件的情况下，全面测试上位机软件的功能和稳定性。
