# MockMotorDevice 磁铁检测自动化增强

## 概述

本次增强为 `MockMotorDevice` 添加了自动磁铁检测模拟功能，使其能够根据编码器角度自动触发 AI1 电平变化，减少手动控制的需求。

## 实现的功能

### 1. 自动磁铁检测机制

**核心特性：**
- 根据编码器角度自动更新 AI1 电平
- 支持配置多个磁铁位置（默认：3°, 49°, 113°）
- 可调节检测窗口大小（默认：0.5°）
- 自动处理 0°/360° 边界跨越

**工作原理：**
```cpp
// 1. 配置磁铁位置
mockMotor->setMagnetPositions({3.0, 49.0, 113.0});

// 2. 链接编码器角度
mockMotor->linkEncoderAngle(&encoderAngle);

// 3. 启用自动检测
mockMotor->setMagnetDetectionEnabled(true);

// 4. 当编码器角度变化时，AI1 自动更新
// 在磁铁位置 ±0.5° 范围内：AI1 = LOW
// 其他位置：AI1 = HIGH
```

### 2. 磁铁通过计数

**功能：**
- 跟踪每个磁铁被通过的次数
- 支持正向和反向运动检测
- 防止重复计数（离开检测窗口后才能再次计数）

**API：**
```cpp
int passCount = mockMotor->getMagnetPassCount(magnetIndex);
mockMotor->resetMagnetStates();  // 重置所有计数
```

### 3. 配置灵活性

**可配置参数：**
- 磁铁位置：`setMagnetPositions(QVector<double>)`
- 检测窗口：`setMagnetDetectionWindow(double windowDeg)`
- 启用/禁用：`setMagnetDetectionEnabled(bool)`

**示例：**
```cpp
// 自定义磁铁位置
mockMotor->setMagnetPositions({10.0, 90.0, 180.0, 270.0});

// 扩大检测窗口到 2°
mockMotor->setMagnetDetectionWindow(2.0);
```

## 代码变更

### 修改的文件

1. **src/infrastructure/simulation/MockMotorDevice.h**
   - 添加磁铁检测配置方法
   - 添加磁铁状态跟踪结构
   - 添加自动检测更新方法

2. **src/infrastructure/simulation/MockMotorDevice.cpp**
   - 实现 `updateMagnetDetection()` 核心逻辑
   - 在 `updateDynamicRegisters()` 中集成自动检测
   - 实现磁铁通过计数和状态管理

3. **tests/simulation/MockMotorMagnetDetectionTests.cpp** (新建)
   - 15 个测试用例，覆盖所有功能
   - 测试自动检测、配置、边界条件、集成场景

4. **CMakeLists.txt**
   - 添加 `MockMotorMagnetDetectionTests` 测试目标
   - 配置 CTest 集成

## 测试覆盖

### 测试用例列表

| 测试用例 | 验证内容 |
|---------|---------|
| `testAutomaticSingleMagnetDetection` | 单个磁铁自动检测 |
| `testAutomaticMultipleMagnetDetection` | 多个磁铁自动检测 |
| `testMagnetDetectionWithEncoderSimulation` | 与编码器模拟集成 |
| `testCustomMagnetPositions` | 自定义磁铁位置 |
| `testCustomDetectionWindow` | 自定义检测窗口 |
| `testEnableMagnetDetection` | 启用/禁用功能 |
| `testMagnetPassCounting` | 磁铁通过计数 |
| `testMagnetStateReset` | 状态重置 |
| `testMagnetDetectionNearZeroDegree` | 0° 附近检测 |
| `testMagnetDetectionAcross360Boundary` | 360° 边界跨越 |
| `testMagnetDetectionWithMotorMovement` | 电机运动中检测 |
| `testReverseDirectionMagnetDetection` | 反向运动检测 |
| `testNoDetectionWhenDisabled` | 禁用时不检测 |
| `testNoDetectionWithoutLinkedEncoder` | 未链接编码器时不检测 |
| `testManualAI1ControlWhenDetectionDisabled` | 禁用时手动控制 |

### 运行测试

```bash
# 构建测试
cmake --build build --target MockMotorMagnetDetectionTests

# 运行测试
ctest -R MockMotorMagnetDetectionTests --output-on-failure
```

## 使用示例

### 基本用法

```cpp
#include "src/infrastructure/simulation/MockMotorDevice.h"
#include "src/infrastructure/simulation/MockEncoderDevice.h"

// 创建设备
MockMotorDevice motor(1);
MockEncoderDevice encoder(3);

// 配置磁铁检测
motor.setMagnetPositions({3.0, 49.0, 113.0});
motor.linkEncoderAngle(&encoder.mockAngleDeg);
motor.setMagnetDetectionEnabled(true);

// 模拟运动
for (double angle = 0.0; angle <= 120.0; angle += 0.5) {
    encoder.setAngle(angle);
    
    // 读取 AI1 电平（自动更新）
    QByteArray request = QByteArray::fromHex("01 03 0052 0001");
    QByteArray response = motor.processRequest(request);
    
    // 检查磁铁检测
    uint16_t ai1Level = (response[3] << 8) | response[4];
    if (ai1Level == 0) {
        qDebug() << "Magnet detected at" << angle << "degrees";
    }
}

// 查看统计
qDebug() << "Magnet 0 passed" << motor.getMagnetPassCount(0) << "times";
qDebug() << "Magnet 1 passed" << motor.getMagnetPassCount(1) << "times";
qDebug() << "Magnet 2 passed" << motor.getMagnetPassCount(2) << "times";
```

### 高级场景：归零测试

```cpp
// 配置单个磁铁用于归零
motor.setMagnetPositions({3.0});
motor.setMagnetDetectionWindow(0.5);
motor.linkEncoderAngle(&encoder.mockAngleDeg);
motor.setMagnetDetectionEnabled(true);

// 模拟归零过程
bool magnetFound = false;
for (double angle = 0.0; angle <= 360.0 && !magnetFound; angle += 0.1) {
    encoder.setAngle(angle);
    
    QByteArray request = QByteArray::fromHex("01 03 0052 0001");
    QByteArray response = motor.processRequest(request);
    uint16_t ai1Level = (response[3] << 8) | response[4];
    
    if (ai1Level == 0) {
        qDebug() << "Homing magnet found at" << angle << "degrees";
        magnetFound = true;
    }
}

QVERIFY(magnetFound);
QCOMPARE(motor.getMagnetPassCount(0), 1);
```

## 技术细节

### 检测算法

```cpp
void MockMotorDevice::updateMagnetDetection(double currentAngle) {
    bool anyMagnetDetected = false;

    for (int i = 0; i < m_magnetPositions.size(); ++i) {
        double magnetPos = m_magnetPositions[i];
        double angleDiff = qAbs(currentAngle - magnetPos);

        // 处理 0°/360° 边界
        if (angleDiff > 180.0) {
            angleDiff = 360.0 - angleDiff;
        }

        // 检测窗口判断
        if (angleDiff <= m_magnetDetectionWindow) {
            if (!m_magnetStates[i].detected) {
                // 首次进入检测窗口
                m_magnetStates[i].detected = true;
                m_magnetStates[i].passCount++;
            }
            anyMagnetDetected = true;
        } else {
            // 离开检测窗口
            if (m_magnetStates[i].detected) {
                m_magnetStates[i].detected = false;
            }
        }
    }

    // 更新 AI1 电平：任意磁铁检测到 = LOW，否则 = HIGH
    m_ai1InputLevel = !anyMagnetDetected;
}
```

### 性能优化

- **角度变化检测**：仅在角度变化 > 0.01° 时更新，避免冗余计算
- **边界处理**：高效的角度差计算，正确处理 0°/360° 跨越
- **状态缓存**：使用 `detected` 标志避免重复触发

## 与现有系统集成

### 兼容性

- **向后兼容**：禁用自动检测时，保持原有手动控制行为
- **接口一致**：通过 Modbus 寄存器读取 AI1，与真实硬件一致
- **测试框架**：与 `Tests::Mocks::MockMotorDevice` 功能对齐

### 集成点

1. **GearboxTestEngine**：归零流程中的磁铁检测
2. **TestExecutionViewModel**：测试执行中的位置验证
3. **SimulationContext**：与编码器角度模拟联动

## 未来改进方向

1. **信号触发**：添加 `magnetDetected(int magnetIndex)` 信号
2. **检测延迟**：模拟真实硬件的检测延迟（如 10ms）
3. **噪声模拟**：添加检测抖动和误触发模拟
4. **多圈支持**：支持虚拟多圈编码器的磁铁检测

## 总结

本次增强显著提升了 `MockMotorDevice` 的自动化能力：

✅ **减少手动干预**：自动根据编码器角度更新 AI1  
✅ **提高测试可靠性**：精确的磁铁检测模拟  
✅ **增强可配置性**：灵活的磁铁位置和检测窗口  
✅ **完善测试覆盖**：15 个测试用例验证所有功能  
✅ **保持兼容性**：不影响现有代码和测试  

这为高级场景模拟（如归零、多位置检测、往复运动）提供了坚实的基础。
