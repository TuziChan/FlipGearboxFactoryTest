# Domain Engine Advanced Tests

## 概述

`DomainEngineAdvancedTests.cpp` 包含 GearboxTestEngine 的高级场景测试用例，补充了基础测试中缺失的复杂场景验证。共 17 个测试用例。

## 测试覆盖

### 1. 锁止检测状态机转换测试（5 个测试）

验证锁止检测状态机的完整转换流程：`Idle → WindowCheck → HoldCheck → Locked`

- **testLockDetectionStateMachineIdleToWindowCheck**
  - 验证当转速降至阈值以下时，状态机从 Idle 进入 WindowCheck
  - 测试条件：转速从 10.0 RPM 降至 2.0 RPM（阈值 5.0 RPM）

- **testLockDetectionWindowCheckToHoldCheck**
  - 验证在 WindowCheck 阶段角度稳定后，转换到 HoldCheck
  - 测试条件：角度变化 < 0.5° 持续 100ms

- **testLockDetectionHoldCheckToLocked**
  - 验证在 HoldCheck 阶段持续稳定后，确认锁止
  - 测试条件：转速 < 5.0 RPM，角度变化 < 0.5°，持续 100ms

- **testLockDetectionResetOnSpeedIncrease**
  - 验证转速超过阈值时，状态机重置到 Idle
  - 测试场景：锁止检测过程中转速突然升高

- **testLockDetectionResetOnAngleDrift**
  - 验证角度漂移超过阈值时，状态机重置
  - 测试场景：角度从 45.0° 漂移到 46.0°（阈值 0.5°）

### 2. 角度定位序列测试（3 个测试）

验证角度定位阶段的 5 个位置测量完整性。

- **testAnglePositioningFiveMeasurements**
  - 验证完整测量序列：P1(first) → P2 → P1(return) → P3 → Zero
  - 使用自动磁铁检测模拟真实运动

- **testAnglePositioningSequenceOrder**
  - 验证测量顺序的正确性
  - 确认结果数组中位置名称的顺序

- **testAnglePositioningTimeout**
  - 验证角度定位超时处理
  - 测试条件：100ms 超时，不触发磁铁事件

### 3. 超时处理测试（3 个测试）

验证 LoadTest 和 ReturnToZero 阶段的超时处理。

- **testLoadTestPhaseTimeout**
  - 验证负载测试阶段总超时（200ms）
  - 测试条件：保持高转速，阻止锁止检测

- **testLoadTestRampTimeout**
  - 验证制动斜坡超时（200ms）
  - 测试条件：斜坡完成但未达到锁止

- **testReturnToZeroPhaseTimeout**
  - 验证归零阶段超时（100ms）
  - 测试条件：编码器保持在 180°，远离零点

### 4. 磁铁检测失败恢复测试（3 个测试）

验证磁铁检测失败场景的处理和恢复。

- **testMagnetDetectionTimeout**
  - 验证 Homing 阶段磁铁检测超时
  - 测试条件：100ms 超时，AI1 保持高电平

- **testMagnetDetectionRecoveryAfterFalsePositive**
  - 验证误触发磁铁事件后的恢复
  - 测试场景：在错误位置（100°）触发磁铁事件

- **testEncoderZeroReachTimeout**
  - 验证磁铁检测后编码器归零超时
  - 测试条件：磁铁已检测，但编码器保持在 180°

### 5. 锁止检测超时测试（3 个测试）

验证锁止条件无法满足时的超时处理。

- **testLockDetectionNeverAchieved**
  - 验证整个斜坡过程中锁止条件从未满足
  - 测试条件：转速保持 50.0 RPM（高于阈值）

- **testLockDetectionIntermittentSpeed**
  - 验证间歇性转速波动的锁止检测
  - 测试场景：转速在阈值上下波动 10 次

- **testLockDetectionAngleOscillation**
  - 验证角度振荡时的锁止检测
  - 测试场景：角度在 45.0° ± 0.3° 范围内振荡

## 测试辅助函数

### advanceToPhase(TestPhase targetPhase)

辅助函数，用于将测试引擎推进到指定阶段。

- 自动处理各阶段的进度条件
- Homing：触发磁铁事件，设置编码器归零
- AnglePositioning：触发 4 次磁铁事件
- LoadRampAndLock：设置低转速和稳定角度
- 最多尝试 100 个周期（3.3 秒）

## 运行测试

### 编译

```bash
cmake --build build --target DomainEngineAdvancedTests
```

### 执行

```bash
# Windows
build\DomainEngineAdvancedTests.exe

# Linux/Mac
./build/DomainEngineAdvancedTests
```

### CTest 集成

```bash
cd build
ctest -R DomainEngineAdvancedTests -V
```

## 测试配方参数

测试使用 `makeTestRecipe()` 创建的配方，关键参数：

- **锁止检测**
  - lockSpeedThresholdRpm: 2.0
  - lockAngleWindowMs: 50
  - lockAngleDeltaDeg: 0.5
  - lockHoldMs: 100

- **超时设置**
  - homeTimeoutMs: 2000
  - idleTimeoutMs: 5000
  - angleTimeoutMs: 2000
  - loadTimeoutMs: 5000
  - returnZeroTimeoutMs: 2000

- **判定限值**
  - 空载电流：0.5 ~ 3.0 A（平均），0.6 ~ 4.0 A（最大）
  - 空载转速：50 ~ 200 RPM（平均），60 ~ 250 RPM（最大）
  - 负载电流：1.0 ~ 4.0 A
  - 负载扭矩：10.0 ~ 50.0 Nm

## 依赖

- Qt6::Core
- Qt6::Test
- MockDevices（tests/mocks/MockDevices.h）
- GearboxTestEngine（src/domain/GearboxTestEngine.h）

## 注意事项

1. **时序敏感**：测试使用 `QTest::qWait()` 模拟时间流逝，实际执行时间可能因系统负载而变化
2. **状态机内部**：锁止检测状态机是引擎内部实现，测试通过观察外部行为验证
3. **QSKIP 使用**：部分测试在无法到达目标阶段时会跳过，这是正常行为
4. **Mock 设备**：所有测试使用 Mock 设备，不依赖真实硬件

## 测试覆盖率

这些测试补充了 `DomainEngineTests.cpp` 中的基础测试，共同覆盖：

- ✅ 状态机所有阶段的正常流程
- ✅ 状态机所有阶段的超时处理
- ✅ 锁止检测状态机的完整转换
- ✅ 角度定位的 5 个位置测量
- ✅ 磁铁检测失败和恢复
- ✅ 通信失败处理
- ✅ 紧急停止
- ✅ 判定逻辑（空载/角度/负载）

## 维护指南

### 添加新测试

1. 在 `private slots:` 区域添加新测试函数
2. 使用 `init()` 和 `cleanup()` 自动设置/清理
3. 使用 `advanceToPhase()` 快速到达目标阶段
4. 使用 `QVERIFY`/`QCOMPARE` 断言验证结果

### 修改测试配方

修改 `makeTestRecipe()` 函数中的参数，所有测试将自动使用新配方。

### 调试失败测试

1. 检查 qDebug 输出，了解失败原因
2. 使用 `QSKIP` 临时跳过不相关测试
3. 调整 `QTest::qWait()` 时间，适应不同系统性能
4. 验证 Mock 设备状态是否符合预期
