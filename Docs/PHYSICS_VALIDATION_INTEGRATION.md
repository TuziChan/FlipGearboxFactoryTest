# 物理规律运行时监控工具 - 集成文档

**创建日期**: 2026-04-25  
**任务**: 阶段3 - 运行时监控工具开发  
**状态**: ✅ 已完成

---

## 概述

本文档描述运行时物理规律监控工具的实现，用于在测试执行页实时监控 Mock 设备的物理规律符合性。

---

## 实现的组件

### 1. PhysicsValidator 类

**文件**: 
- `src/infrastructure/validation/PhysicsValidator.h`
- `src/infrastructure/validation/PhysicsValidator.cpp`

**功能**: 静态验证方法，检查物理规律

**验证规则**:

| 规则 | 描述 | 阈值 |
|------|------|------|
| R1_SpeedConsistency | 转速一致性（扭矩传感器 vs 编码器） | 5% |
| R2_AccelerationLimit | 角加速度限制 | 600 RPM/s |
| R4_PowerConservation | 功率守恒 (P = T × ω) | 10% |
| R7_BrakePower | 制动功率 (P = V × I) | 15% |

**接口**:
```cpp
struct ValidationResult {
    bool passed;
    QString ruleName;
    QString message;
    double actualValue;
    double expectedValue;
    double threshold;
    double errorPercent;
};

struct ValidationConfig {
    double speedConsistencyThreshold = 0.05;      // 5%
    double powerConservationThreshold = 0.10;     // 10%
    double maxAccelerationRpmPerS = 600.0;        // 600 RPM/s
    double brakePowerThreshold = 0.15;            // 15%
};

static ValidationResult checkSpeedConsistency(
    const TelemetrySnapshot& snapshot,
    const ValidationConfig& config = ValidationConfig());

static ValidationResult checkAccelerationLimit(
    const TelemetrySnapshot& current,
    const TelemetrySnapshot& previous,
    const ValidationConfig& config = ValidationConfig());

static ValidationResult checkPowerConservation(
    const TelemetrySnapshot& snapshot,
    const ValidationConfig& config = ValidationConfig());

static ValidationResult checkBrakePower(
    const TelemetrySnapshot& snapshot,
    const ValidationConfig& config = ValidationConfig());

static QVector<ValidationResult> validateAll(
    const TelemetrySnapshot& current,
    const TelemetrySnapshot& previous,
    const ValidationConfig& config = ValidationConfig());
```

**智能跳过逻辑**:
- 设备离线时跳过验证
- 转速/功率接近零时跳过（避免除零和无意义比较）
- 时间戳无效时跳过加速度检查

---

### 2. PhysicsViolationLogger 类

**文件**:
- `src/infrastructure/validation/PhysicsViolationLogger.h`
- `src/infrastructure/validation/PhysicsViolationLogger.cpp`

**功能**: 违规事件日志记录器（JSON Lines 格式）

**日志格式**:
```json
{
  "timestamp": "2026-04-25T10:30:45.123",
  "rule": "R4_PowerConservation",
  "passed": false,
  "message": "Power mismatch: Measured=130.5 W, Calculated=130.9 W, Error=0.31%",
  "actual_value": 130.5,
  "expected_value": 130.9,
  "threshold": 0.10,
  "error_percent": 0.0031,
  "telemetry": {
    "motor_current_a": 2.5,
    "dyn_speed_rpm": 500.0,
    "dyn_torque_nm": 2.5,
    "dyn_power_w": 130.5,
    "encoder_angle_deg": 45.0,
    "encoder_velocity_rpm": 500.0,
    "brake_current_a": 0.0,
    "brake_voltage_v": 0.0,
    "brake_power_w": 0.0
  }
}
```

**日志文件路径**: `logs/physics_violations_<serialNumber>_<timestamp>.jsonl`

**线程安全**: 使用 QMutex 保护并发写入

---

### 3. GearboxTestEngine 集成

**修改文件**: 
- `src/domain/GearboxTestEngine.h`
- `src/domain/GearboxTestEngine.cpp`

**集成点**: `GearboxTestEngine::onCycleTick()`

**执行流程**:
```cpp
void GearboxTestEngine::onCycleTick() {
    // 1. 紧急停止检查
    if (m_emergencyStopRequested.load()) return;

    // 2. 采集遥测数据
    if (!acquireTelemetry(m_state.currentTelemetry)) {
        failTest(...);
        return;
    }

    // 3. 物理规律验证（新增）
    if (m_physicsValidationEnabled && m_lastSnapshot.timestamp.isValid()) {
        auto results = PhysicsValidator::validateAll(
            m_state.currentTelemetry, 
            m_lastSnapshot, 
            m_validationConfig);

        for (const auto& result : results) {
            if (!result.passed) {
                qWarning() << "[PhysicsValidation]" << result.ruleName
                          << "FAILED:" << result.message;

                // 记录违规日志
                if (m_violationLogger && m_violationLogger->isActive()) {
                    m_violationLogger->logViolation(result, m_state.currentTelemetry);
                }
            }
        }
    }

    // 4. 更新上一次快照
    m_lastSnapshot = m_state.currentTelemetry;

    // 5. 状态机逻辑...
}
```

**新增成员变量**:
```cpp
private:
    Infrastructure::Validation::PhysicsValidator::ValidationConfig m_validationConfig;
    Infrastructure::Validation::PhysicsViolationLogger* m_violationLogger;
    TelemetrySnapshot m_lastSnapshot;
    bool m_physicsValidationEnabled;
```

**新增配置方法**:
```cpp
void setPhysicsValidationEnabled(bool enabled);
void setValidationConfig(const ValidationConfig& config);
```

**生命周期管理**:
- `startTest()`: 启动日志会话
- `completeTest()`: 关闭日志会话
- `failTest()`: 关闭日志会话

---

## 性能考虑

### 开销分析

| 操作 | 耗时估算 |
|------|---------|
| 4 个验证规则计算 | < 0.1 ms |
| JSON 序列化 + 文件写入 | < 0.5 ms（仅违规时） |
| **总开销** | **< 0.6 ms/tick** |

**结论**: 在 33ms 周期（30Hz）下，开销 < 2%，符合要求（< 1ms/tick）。

### 优化措施

1. **智能跳过**: 设备离线或数据无效时不计算
2. **延迟写入**: 仅违规时写入日志（正常情况无 I/O）
3. **静态方法**: 无对象创建开销
4. **线程安全**: QMutex 仅在写入时锁定

---

## 使用示例

### 基本用法（默认配置）

```cpp
GearboxTestEngine engine;
engine.setDevices(motor, torque, encoder, brake);
engine.setRecipe(recipe);

// 物理验证默认启用，无需额外配置
engine.startTest("SN12345");
```

### 自定义阈值

```cpp
PhysicsValidator::ValidationConfig config;
config.speedConsistencyThreshold = 0.03;      // 更严格：3%
config.powerConservationThreshold = 0.15;     // 更宽松：15%
config.maxAccelerationRpmPerS = 800.0;        // 更高加速度

engine.setValidationConfig(config);
engine.startTest("SN12345");
```

### 禁用验证（生产环境）

```cpp
engine.setPhysicsValidationEnabled(false);
engine.startTest("SN12345");
```

---

## 日志分析

### 查看违规日志

```bash
# 查看最新日志
cat logs/physics_violations_SN12345_20260425_103045.jsonl

# 统计违规次数
grep -c '"passed": false' logs/physics_violations_*.jsonl

# 按规则分类
jq -r '.rule' logs/physics_violations_*.jsonl | sort | uniq -c
```

### 离线分析脚本（Python）

```python
import json
import pandas as pd

# 加载日志
violations = []
with open('logs/physics_violations_SN12345_20260425_103045.jsonl') as f:
    for line in f:
        violations.append(json.loads(line))

df = pd.DataFrame(violations)

# 统计违规率
print(df['rule'].value_counts())

# 绘制误差分布
import matplotlib.pyplot as plt
df.boxplot(column='error_percent', by='rule')
plt.show()
```

---

## 测试验证

### 单元测试（待实现）

**文件**: `tests/validation/PhysicsValidatorTests.cpp`

**测试用例**:
```cpp
TEST(PhysicsValidator, SpeedConsistency_Pass) {
    TelemetrySnapshot snapshot;
    snapshot.dynSpeedRpm = 500.0;
    snapshot.encoderVelocityRpm = 505.0;  // 1% 误差
    snapshot.torqueOnline = true;
    snapshot.encoderOnline = true;

    auto result = PhysicsValidator::checkSpeedConsistency(snapshot);
    ASSERT_TRUE(result.passed);
}

TEST(PhysicsValidator, SpeedConsistency_Fail) {
    TelemetrySnapshot snapshot;
    snapshot.dynSpeedRpm = 500.0;
    snapshot.encoderVelocityRpm = 550.0;  // 10% 误差
    snapshot.torqueOnline = true;
    snapshot.encoderOnline = true;

    auto result = PhysicsValidator::checkSpeedConsistency(snapshot);
    ASSERT_FALSE(result.passed);
    ASSERT_GT(result.errorPercent, 0.05);
}

TEST(PhysicsValidator, PowerConservation) {
    TelemetrySnapshot snapshot;
    snapshot.dynTorqueNm = 2.5;
    snapshot.dynSpeedRpm = 500.0;
    snapshot.dynPowerW = 130.9;  // P = 2.5 × (500×2π/60) ≈ 130.9 W
    snapshot.torqueOnline = true;

    auto result = PhysicsValidator::checkPowerConservation(snapshot);
    ASSERT_TRUE(result.passed);
}

TEST(PhysicsValidator, AccelerationLimit) {
    TelemetrySnapshot prev, curr;
    prev.encoderVelocityRpm = 0.0;
    prev.timestamp = QDateTime::currentDateTime();
    prev.encoderOnline = true;

    curr.encoderVelocityRpm = 500.0;
    curr.timestamp = prev.timestamp.addMSecs(1000);  // 1 秒后
    curr.encoderOnline = true;

    auto result = PhysicsValidator::checkAccelerationLimit(curr, prev);
    ASSERT_TRUE(result.passed);  // 500 RPM/s < 600 RPM/s
}
```

### 集成测试

**运行完整测试**:
```bash
cd build
ctest -R GearboxSimulationIntegrationTests -V
```

**预期结果**:
- 测试执行页正常运行
- 控制台输出物理验证警告（如有违规）
- `logs/` 目录生成 `.jsonl` 文件

---

## CMakeLists.txt 更新

已添加到 `FlipGearboxInfra` 库：

```cmake
# Infrastructure - Validation
src/infrastructure/validation/PhysicsValidator.h
src/infrastructure/validation/PhysicsValidator.cpp
src/infrastructure/validation/PhysicsViolationLogger.h
src/infrastructure/validation/PhysicsViolationLogger.cpp
```

---

## 下一步工作

### T3.2: GearboxTestEngine 集成（已完成）
- ✅ 在 `onCycleTick()` 中插入验证检查点
- ✅ 生命周期管理（启动/关闭日志会话）
- ✅ 配置接口（启用/禁用、阈值调整）

### T3.3: 违规日志系统（已完成）
- ✅ JSON Lines 格式输出
- ✅ 线程安全写入
- ✅ 文件路径管理

### 待完成（后续任务）
- [ ] 单元测试（`tests/validation/PhysicsValidatorTests.cpp`）
- [ ] 集成测试验证（运行测试执行页，检查日志）
- [ ] Python 离线分析脚本（`scripts/analyze_physics.py`）
- [ ] 性能基准测试（验证 < 1ms/tick）

---

## 验收标准

- [x] PhysicsValidator 实现完成
- [x] PhysicsViolationLogger 实现完成
- [x] GearboxTestEngine 集成完成
- [x] CMakeLists.txt 更新
- [ ] 编译通过（需要 Qt Creator 构建）
- [ ] 运行完整测试无崩溃
- [ ] 违规日志文件生成正常

---

## 参考文档

- [MOCK_VALIDATION_TASK_BREAKDOWN.md](./MOCK_VALIDATION_TASK_BREAKDOWN.md) - 任务分解
- [MOCK_PHYSICS_VALIDATION_PLAN.md](./MOCK_PHYSICS_VALIDATION_PLAN.md) - 验证计划
- `src/infrastructure/simulation/SimulationContext.h` - 物理模拟实现

---

**文档状态**: ✅ 已完成  
**实现状态**: ✅ 代码已完成，待编译验证
