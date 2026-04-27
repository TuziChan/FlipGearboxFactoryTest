# ViewModel 物理检查结果集成 - 实现文档

**任务ID**: 98626f21-8c45-4b8f-a4e8-f0eb15c4ec2d  
**完成日期**: 2026-04-25  
**负责人**: ViewModel开发

---

## 实现概述

为 `TestExecutionViewModel` 添加了物理规律检查结果展示功能，使 QML UI 层能够实时显示 Mock 数据的物理验证状态。

---

## 新增 Q_PROPERTY

### 1. physicsViolations (QVariantList)
**用途**: 存储当前检测到的所有物理规律违规记录

**数据结构**:
```qml
[
    {
        "passed": false,
        "ruleName": "R1_SpeedConsistency",
        "message": "Speed mismatch: Torque=500.0 RPM, Encoder=520.0 RPM, Error=3.85%",
        "actualValue": 500.0,
        "expectedValue": 520.0,
        "threshold": 0.05,
        "errorPercent": 0.0385
    },
    ...
]
```

### 2. physicsViolationStats (QVariantMap)
**用途**: 提供违规统计信息

**数据结构**:
```qml
{
    "totalViolations": 3,
    "warningCount": 2,      // errorPercent <= 20%
    "criticalCount": 1,     // errorPercent > 20%
    "lastCheckTime": QDateTime
}
```

---

## 信号

### physicsValidationChanged()
**触发时机**:
- 检测到新的物理规律违规
- 违规统计数据更新
- 调用 `clearPhysicsValidation()` 清空数据

---

## 核心方法实现

### updatePhysicsValidation(const TelemetrySnapshot& current)
**职责**: 
- 调用 `PhysicsValidator::validateAll()` 执行物理规律检查
- 将验证结果转换为 QML 友好的 QVariantList/QVariantMap
- 根据 `errorPercent` 分类违规严重程度（警告/严重）
- 更新成员变量并发射信号

**调用位置**: `updateFromState()` 中，每次接收到新的遥测数据时

**性能考虑**:
- 仅在有前一次遥测快照时才执行验证（避免首次调用时无效比较）
- 验证逻辑在 `PhysicsValidator` 中已优化（跳过离线设备、零值数据）

### clearPhysicsValidation()
**职责**: 
- 清空所有物理验证数据
- 重置 `m_lastTelemetry`

**调用位置**: `resetTest()` 中

### toVariantMap(const ValidationResult& result)
**职责**: 将 `PhysicsValidator::ValidationResult` 转换为 QVariantMap

---

## 集成点

### 1. 构造函数
初始化新增成员变量：
```cpp
, m_physicsViolations()
, m_physicsViolationStats()
, m_lastTelemetry()
```

### 2. updateFromState()
在更新遥测数据后立即调用物理验证：
```cpp
// Update physics validation with current telemetry
updatePhysicsValidation(state.currentTelemetry);
```

### 3. resetTest()
清空物理验证数据：
```cpp
clearPhysicsValidation();
```

并在信号发射列表中添加：
```cpp
emit physicsValidationChanged();
```

---

## 严重程度分类规则

| 错误百分比 | 分类 | 说明 |
|-----------|------|------|
| ≤ 20% | Warning | 轻微偏差，可能是传感器噪声 |
| > 20% | Critical | 严重偏差，Mock 数据模型可能有问题 |

---

## QML 使用示例

### 显示违规列表
```qml
ListView {
    model: testExecutionViewModel.physicsViolations
    delegate: Rectangle {
        Text {
            text: modelData.ruleName + ": " + modelData.message
            color: modelData.errorPercent > 0.20 ? "red" : "orange"
        }
    }
}
```

### 显示统计信息
```qml
Row {
    Text { text: "总违规: " + testExecutionViewModel.physicsViolationStats.totalViolations }
    Text { text: "警告: " + testExecutionViewModel.physicsViolationStats.warningCount }
    Text { text: "严重: " + testExecutionViewModel.physicsViolationStats.criticalCount }
}
```

---

## 验证的物理规律

根据 `PhysicsValidator` 实现，当前验证以下规律：

1. **R1_SpeedConsistency**: 扭矩传感器与编码器转速一致性（阈值 5%）
2. **R2_AccelerationLimit**: 加速度限制（≤ 600 RPM/s）
3. **R4_PowerConservation**: 功率守恒 P = T × ω（阈值 10%）
4. **R7_BrakePower**: 制动功率 P = V × I（阈值 15%）

---

## 依赖关系

### 头文件
- `TestExecutionViewModel.h`: 添加前向声明 `Infrastructure::Validation::PhysicsValidator`
- `TestExecutionViewModel.cpp`: 包含 `#include "../infrastructure/validation/PhysicsValidator.h"`

### 数据流
```
GearboxTestEngine (Domain)
    ↓ stateChanged(TestRunState)
TestExecutionViewModel
    ↓ updateFromState()
    ↓ updatePhysicsValidation()
PhysicsValidator::validateAll()
    ↓ ValidationResult[]
QML UI (physicsViolations, physicsViolationStats)
```

---

## 测试建议

### 单元测试
1. 验证 `updatePhysicsValidation()` 正确分类违规严重程度
2. 验证 `clearPhysicsValidation()` 清空所有数据
3. 验证首次调用时跳过验证（无前一次快照）

### 集成测试
1. 运行 Mock 测试，观察 `physicsViolations` 是否实时更新
2. 验证 `physicsValidationChanged` 信号正确触发
3. 检查 QML UI 能否正确显示违规信息

---

## 已知限制

1. **不阻塞测试流程**: 物理验证仅记录违规，不会中断测试执行
2. **仅 Mock 模式有效**: 真实硬件模式下，物理规律验证可能因传感器噪声产生误报
3. **内存占用**: 违规记录会累积，长时间运行可能占用较多内存（建议定期清理或限制记录数量）

---

## 后续优化方向

1. **违规记录限制**: 添加最大记录数限制（如保留最近 100 条）
2. **可配置阈值**: 允许通过配置文件调整验证阈值
3. **违规趋势分析**: 统计违规频率，识别系统性问题
4. **日志持久化**: 将违规记录写入文件，便于离线分析

---

**实现状态**: ✅ 已完成  
**代码审查**: 待审查  
**集成测试**: 待测试
