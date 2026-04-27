# Mock测试仿真数据物理规律检查方案

**文档版本**: v1.0  
**创建日期**: 2026-04-25  
**负责角色**: 头脑风暴 (Brainstorming)  
**目标**: 确保测试执行页Mock仿真数据符合真实物理规律，避免虚假通过测试

---

## 一、物理模型架构分析

### 1.1 核心物理引擎
**SimulationContext** (`src/infrastructure/simulation/SimulationContext.h`)
- **物理时钟**: 10ms/tick，模拟实时物理演进
- **状态变量**:
  - 电机方向 (Forward/Reverse/Stopped)
  - 电机占空比 (0-100%)
  - 制动电流/电压 (A/V)
  - 编码器角度 (度)
  - 当前转速/目标转速 (RPM)

### 1.2 设备仿真层
| 设备 | 实现类 | 物理量输出 |
|------|--------|-----------|
| 电机驱动 | SimulatedMotorDevice | 电流(A)、AI1电平(磁铁检测) |
| 扭矩传感器 | SimulatedTorqueDevice | 扭矩(N·m)、转速(RPM)、功率(W) |
| 编码器 | SimulatedEncoderDevice | 角度(°)、累计角度(°)、角速度(RPM) |
| 制动电源 | SimulatedBrakeDevice | 电压(V)、电流(A)、功率(W) |

---

## 二、物理规律维度清单

### 2.1 运动学规律

#### ✅ R1: 角速度-转速一致性
**物理公式**: `编码器角速度(RPM) ≈ 扭矩传感器转速(RPM)`

**代码实现**:
```cpp
// SimulationContext.h:103-106
double encoderAngularVelocityRpm() const {
    return (m_motorDirection == Forward) ? m_currentSpeedRpm : 
           (m_motorDirection == Reverse) ? -m_currentSpeedRpm : 0.0;
}

// SimulatedTorqueDevice.cpp:35-42
bool readSpeed(double& speedRpm) {
    speedRpm = std::abs(m_context->encoderAngularVelocityRpm());
    return true;
}
```

**检查方法**: 
- 采样两个设备的转速值
- 计算相对误差: `|encoder_rpm - torque_rpm| / max(encoder_rpm, torque_rpm)`
- **判据**: 误差 < 5%

**潜在问题**: 
- ❌ 扭矩传感器使用 `std::abs()` 丢失方向信息
- ✅ 编码器保留符号（正转为正，反转为负）

---

#### ✅ R2: 角度积分连续性
**物理公式**: `Δ角度 = 转速(RPM) × 6 × Δt(s)`

**代码实现**:
```cpp
// SimulationContext.h:130-139
double degreesPerSecond = m_currentSpeedRpm * 6.0;
double degreesPerTick = degreesPerSecond * 0.01; // 10ms per tick

if (m_motorDirection == Forward) {
    m_encoderAngleDeg += degreesPerTick;
} else if (m_motorDirection == Reverse) {
    m_encoderAngleDeg -= degreesPerTick;
}
```

**检查方法**:
- 记录连续N个tick的角度值: `[θ₀, θ₁, θ₂, ..., θₙ]`
- 计算数值积分: `Σ(转速ᵢ × 6 × 0.01)`
- 对比实际角度变化: `θₙ - θ₀`
- **判据**: 积分误差 < 1°

**边界条件**:
- 跨越0°/360°边界时需归一化处理

---

#### ✅ R3: 加速度物理限制
**物理公式**: `加速度 = Δ转速 / Δt ≤ 500 RPM/s`

**代码实现**:
```cpp
// SimulationContext.h:119-127
const double accelRatePerTick = 5.0; // 5 RPM per 10ms = 500 RPM/s

if (m_currentSpeedRpm < effectiveTargetSpeed) {
    m_currentSpeedRpm = std::min(m_currentSpeedRpm + accelRatePerTick, effectiveTargetSpeed);
} else if (m_currentSpeedRpm > effectiveTargetSpeed) {
    double decelRate = brakeLoad > 0.0 ? accelRatePerTick * 3.0 : accelRatePerTick;
    m_currentSpeedRpm = std::max(m_currentSpeedRpm - decelRate, effectiveTargetSpeed);
}
```

**检查方法**:
- 计算相邻采样点加速度: `a = (v₂ - v₁) / 0.01`
- **判据**: 
  - 加速: `a ≤ 500 RPM/s`
  - 减速(无制动): `a ≥ -500 RPM/s`
  - 减速(有制动): `a ≥ -1500 RPM/s` (允许3倍减速率)

**异常场景**:
- ⚠️ 急停时可能出现瞬时大加速度（需特殊标记）

---

### 2.2 动力学规律

#### ✅ R4: 功率守恒定律
**物理公式**: `P(W) = T(N·m) × ω(rad/s) = T × RPM × 2π/60`

**代码实现**:
```cpp
// SimulatedTorqueDevice.cpp:54-55
powerW = torque * speed * 2.0 * M_PI / 60.0;
```

**检查方法**:
- 读取扭矩、转速、功率三个值
- 计算理论功率: `P_theory = T × RPM × 0.10472`
- 计算偏差率: `|P_actual - P_theory| / P_theory`
- **判据**: 偏差 < 10%

**噪声影响**:
- 扭矩传感器添加 ±0.05 N·m 高斯噪声
- 在低功率区间(<10W)相对误差会放大

---

#### ✅ R5: 扭矩组成模型
**物理公式**: `T_total = T_friction + T_motor + T_brake`

**代码实现**:
```cpp
// SimulatedTorqueDevice.cpp:84-111
double baseTorque = 0.3; // 摩擦扭矩
double motorTorque = (dutyCycle/100.0) * 2.0 - (speed/1000.0) * 0.5; // 电机特性
double brakeTorque = m_context->brakeCurrent() * 0.85; // 制动扭矩
double totalTorque = baseTorque + motorTorque + brakeTorque;
```

**检查方法**:
- **空载工况** (制动关闭):
  - 扭矩应为 `0.3 + 电机扭矩`
  - 电机扭矩随占空比线性增长
  - **判据**: 空载扭矩 < 3 N·m
  
- **负载工况** (制动开启):
  - 扭矩主导项为 `制动电流 × 0.85`
  - **判据**: 制动扭矩单调递增（电流↑ → 扭矩↑）

**物理合理性**:
- ✅ 摩擦扭矩恒定 (0.3 N·m)
- ✅ 电机扭矩随速度衰减 (反电动势效应)
- ✅ 制动扭矩与电流成正比 (0.85 N·m/A)

---

#### ✅ R6: 电机电流-负载关系
**物理公式**: `I_motor = I_base + k₁×duty + k₂×brake_load - k₃×speed`

**代码实现**:
```cpp
// SimulatedMotorDevice.cpp:64-76
double baseCurrent = 0.5;
double dutyCycleCurrent = (dutyCycle / 100.0) * 1.5;
double loadCurrent = brakeLoad * 0.3;
double speedReduction = (speed / 1000.0) * 0.2;
currentA = baseCurrent + dutyCycleCurrent + loadCurrent - speedReduction;
```

**检查方法**:
- **空载测试** (制动关闭):
  - 电流应 < 3A
  - **判据**: `I < 0.5 + 1.5×(duty/100) + 0.2`
  
- **负载测试** (制动开启):
  - 电流随制动电流增加
  - **判据**: `ΔI_motor / ΔI_brake ≈ 0.3`

**物理意义**:
- 基础电流 0.5A: 电机空转损耗
- 占空比系数 1.5: 最大额外电流
- 负载系数 0.3: 制动每增加1A，电机电流增加0.3A
- 速度系数 -0.2: 反电动势降低电流

---

### 2.3 电气特性

#### ✅ R7: 制动功率计算
**物理公式**: `P_brake = U × I`

**代码实现**:
```cpp
// MockBrakeDevice.cpp:105
actualPowerW = actualVoltageV * actualCurrentA;
```

**检查方法**:
- 读取制动电压、电流、功率
- 计算理论功率: `P_theory = U × I`
- **判据**: `|P_actual - P_theory| < 0.5W`

---

#### ✅ R8: CC/CV模式行为
**代码实现**:
```cpp
// MockBrakeDevice.cpp:101-111
if (state->mode == 0) { // CC模式
    actualCurrentA = state->setCurrentA; // 电流恒定
    actualVoltageV = state->setVoltageV; // 电压跟随负载
} else { // CV模式
    actualVoltageV = state->setVoltageV; // 电压恒定
    actualCurrentA = state->setCurrentA; // 电流跟随负载
}
```

**检查方法**:
- **CC模式**: 
  - 设定电流后，实际电流应稳定在设定值 ±0.1A
  - 电压随负载变化
  
- **CV模式**:
  - 设定电压后，实际电压应稳定在设定值 ±0.5V
  - 电流随负载变化

**判据**: 控制量偏差 < 5%

---

### 2.4 磁铁检测物理

#### ✅ R9: 磁铁检测窗口
**物理公式**: `|当前角度 - 磁铁位置| ≤ 检测窗口`

**代码实现**:
```cpp
// MockMotorDevice.cpp:172-189
bool isAngleInWindow(double angle, double targetAngle, double window) const {
    angle = normalize(angle);
    targetAngle = normalize(targetAngle);
    double diff = qAbs(angle - targetAngle);
    if (diff > 180.0) diff = 360.0 - diff; // 最短路径
    return diff <= window;
}
```

**检查方法**:
- 设定磁铁位置: [3°, 49°, 113°]
- 检测窗口: ±2°
- 遍历角度 0-360°，验证检测逻辑
- **判据**: 
  - 在窗口内: AI1 = LOW
  - 在窗口外: AI1 = HIGH

**边界测试**:
- 角度 = 1° → 距离磁铁①(3°) = 2° → 应检测到
- 角度 = 0° → 距离磁铁①(3°) = 3° → 不应检测到
- 角度 = 358° → 距离磁铁①(3°) = 5° → 不应检测到

---

#### ✅ R10: 边缘触发计数
**代码实现**:
```cpp
// MockMotorDevice.cpp:157-159
if (inWindow && !m_magnetLastState[i]) {
    m_magnetPassCounts[i]++; // 仅在上升沿计数
}
```

**检查方法**:
- 模拟电机正转一圈 (0° → 360°)
- 记录每个磁铁的通过次数
- **判据**: 每个磁铁计数 = 1

**异常场景**:
- ❌ 在磁铁区域内抖动 → 不应重复计数
- ✅ 离开后再次进入 → 应再次计数

---

### 2.5 时间域特性

#### ✅ R11: 制动减速时序
**物理模型**: 制动电流上升 → 扭矩增加 → 转速下降

**代码实现**:
```cpp
// SimulationContext.h:113-115
double brakeLoad = m_brakeOutputEnabled ? m_brakeCurrentA : 0.0;
double speedReduction = brakeLoad * 100.0; // 每安培降低100 RPM
double effectiveTargetSpeed = std::max(0.0, m_targetSpeedRpm - speedReduction);
```

**检查方法**:
- 记录时间序列: `[(t, I_brake, speed), ...]`
- 验证因果关系:
  - `I_brake(t)` 增加 → `speed(t+Δt)` 下降
  - 延迟时间 Δt < 100ms
- **判据**: 相关系数 < -0.8 (负相关)

---

#### ✅ R12: 锁止判定逻辑
**物理条件**: 
1. 转速 < 5 RPM
2. 角度稳定 (100ms内变化 < 5°)

**检查方法**:
- 在负载测试阶段监控状态机
- 记录锁止判定时刻的物理量
- **判据**:
  - 判定时转速 < 5 RPM
  - 前100ms角度标准差 < 2°

---

## 三、检查策略设计

### 3.1 四层检查架构

```
┌─────────────────────────────────────────────────────────┐
│ L4: UI数据呈现检查 (ViewModel + UI开发)                  │
│ - 图表数据无瞬时跳变                                     │
│ - 物理量显示精度合理                                     │
└─────────────────────────────────────────────────────────┘
                          ↑
┌─────────────────────────────────────────────────────────┐
│ L3: 状态机时序检查 (领域引擎开发)                        │
│ - 测试阶段转换合理                                       │
│ - 锁止/磁铁检测逻辑正确                                  │
└─────────────────────────────────────────────────────────┘
                          ↑
┌─────────────────────────────────────────────────────────┐
│ L2: 跨设备耦合检查 (协议与设备开发)                      │
│ - 编码器转速 ≈ 扭矩传感器转速                            │
│ - 功率 = 扭矩 × 角速度                                   │
└─────────────────────────────────────────────────────────┘
                          ↑
┌─────────────────────────────────────────────────────────┐
│ L1: 单设备自洽性检查 (基础设施/服务开发)                 │
│ - 制动功率 = 电压 × 电流                                 │
│ - 角度积分 = 转速 × 时间                                 │
└─────────────────────────────────────────────────────────┘
```

### 3.2 检查点优先级

| 优先级 | 检查点 | 影响范围 | 检测难度 |
|--------|--------|---------|---------|
| 🔴 P0 | R4: 功率守恒 | 全局 | 低 |
| 🔴 P0 | R1: 角速度一致性 | 全局 | 低 |
| 🟠 P1 | R5: 扭矩组成模型 | 负载测试 | 中 |
| 🟠 P1 | R9: 磁铁检测窗口 | 找零阶段 | 中 |
| 🟡 P2 | R3: 加速度限制 | 动态过程 | 高 |
| 🟡 P2 | R11: 制动减速时序 | 负载测试 | 高 |

---

## 四、实施方案

### 4.1 阶段1: 静态代码审查 (1天)
**负责人**: 代码审查 + 头脑风暴

**任务**:
1. 审查 `SimulationContext::updatePhysics()` 物理公式正确性
2. 验证各设备 `calculate*()` 方法的数学模型
3. 检查边界条件处理 (0°/360°, 除零保护)

**交付物**: 代码审查报告 (标注潜在物理错误)

---

### 4.2 阶段2: 单元测试增强 (2天)
**负责人**: 基础设施/服务开发 + 协议与设备开发

**任务**:
1. 为每个Mock设备编写物理规律单元测试
   - `MockTorqueDevice_PowerConservation_Test`
   - `MockMotorDevice_CurrentLoadRelation_Test`
   - `MockEncoderDevice_AngleIntegration_Test`
   
2. 实现跨设备集成测试
   - `CrossDevice_SpeedConsistency_Test`
   - `CrossDevice_BrakingDynamics_Test`

**示例测试代码**:
```cpp
TEST(MockTorqueDevice, PowerConservationLaw) {
    SimulationContext ctx;
    SimulatedTorqueDevice device(&ctx);
    
    ctx.setMotorDirection(Forward);
    ctx.setMotorDutyCycle(50.0);
    
    for (int i = 0; i < 100; i++) {
        double torque, speed, power;
        device.readAll(torque, speed, power);
        
        double theoreticalPower = torque * speed * 2 * M_PI / 60.0;
        double error = std::abs(power - theoreticalPower) / theoreticalPower;
        
        ASSERT_LT(error, 0.10) << "Power conservation violated at tick " << i;
    }
}
```

**交付物**: 
- 新增单元测试 (覆盖R1-R10)
- 测试报告 (通过率、失败案例分析)

---

### 4.3 阶段3: 运行时监控 (2天)
**负责人**: 领域引擎开发 + ViewModel开发

**任务**:
1. 在 `GearboxTestEngine::onCycleTick()` 中插入物理规律断言
   ```cpp
   void GearboxTestEngine::onCycleTick() {
       TelemetrySnapshot snapshot;
       acquireTelemetry(snapshot);
       
       // 物理规律检查
       PhysicsValidator::checkPowerConservation(snapshot);
       PhysicsValidator::checkAccelerationLimit(snapshot, m_lastSnapshot);
       
       m_lastSnapshot = snapshot;
   }
   ```

2. 实现 `PhysicsValidator` 工具类
   - 输入: 遥测快照
   - 输出: 违规报告 (JSON格式)

**交付物**:
- `PhysicsValidator.h/cpp` 实现
- 运行时日志示例

---

### 4.4 阶段4: 离线数据分析 (1天)
**负责人**: 头脑风暴 + 代码审查

**任务**:
1. 运行完整测试流程，导出遥测数据 (CSV格式)
2. 使用Python/Pandas分析:
   ```python
   import pandas as pd
   import matplotlib.pyplot as plt
   
   df = pd.read_csv('telemetry.csv')
   
   # 检查功率守恒
   df['power_theoretical'] = df['torque'] * df['speed'] * 2 * 3.14159 / 60
   df['power_error'] = abs(df['power'] - df['power_theoretical']) / df['power_theoretical']
   
   violations = df[df['power_error'] > 0.10]
   print(f"功率守恒违规: {len(violations)} 个采样点")
   ```

3. 生成可视化报告:
   - 功率守恒偏差分布图
   - 加速度时间序列图
   - 磁铁检测事件标记图

**交付物**:
- Jupyter Notebook 分析脚本
- 物理规律符合性报告 (PDF)

---

## 五、关键检查点执行清单

### ✅ 检查点矩阵

| 规律ID | 检查点名称 | 检查方法 | 判据 | 负责角色 | 优先级 |
|--------|-----------|---------|------|---------|--------|
| R1 | 角速度-转速一致性 | 采样对比 | 误差<5% | 协议与设备开发 | P0 |
| R2 | 角度积分连续性 | 数值积分验证 | 误差<1° | 基础设施开发 | P1 |
| R3 | 加速度物理限制 | 微分计算 | ≤500 RPM/s | 领域引擎开发 | P2 |
| R4 | 功率守恒定律 | 公式验证 | 偏差<10% | 协议与设备开发 | P0 |
| R5 | 扭矩组成模型 | 分量分析 | 单调性检验 | 基础设施开发 | P1 |
| R6 | 电机电流-负载关系 | 相关性分析 | 系数验证 | 协议与设备开发 | P1 |
| R7 | 制动功率计算 | 乘积验证 | 误差<0.5W | 基础设施开发 | P0 |
| R8 | CC/CV模式行为 | 控制量稳定性 | 偏差<5% | 基础设施开发 | P1 |
| R9 | 磁铁检测窗口 | 几何计算 | ±2°窗口 | 协议与设备开发 | P1 |
| R10 | 边缘触发计数 | 状态机验证 | 单次计数 | 协议与设备开发 | P1 |
| R11 | 制动减速时序 | 因果关系检验 | 相关系数<-0.8 | 领域引擎开发 | P2 |
| R12 | 锁止判定逻辑 | 条件验证 | 速度+角度双重判据 | 领域引擎开发 | P1 |

---

## 六、已知问题与改进建议

### 🐛 问题1: 扭矩传感器转速丢失方向信息
**位置**: `SimulatedTorqueDevice.cpp:41`
```cpp
speedRpm = std::abs(m_context->encoderAngularVelocityRpm()); // ❌ 丢失符号
```

**影响**: 无法区分正转/反转，可能导致功率计算错误

**建议**: 保留符号，或单独提供方向标志位

---

### 🐛 问题2: 磁铁检测简化模型
**位置**: `SimulatedMotorDevice.cpp:94-98`
```cpp
if (m_context->tickCount() < m_ai1TransitionTick) {
    level = true; // 固定延迟后触发
}
```

**影响**: 未考虑实际角度位置，与Mock设备的角度检测逻辑不一致

**建议**: 统一使用基于角度的磁铁检测模型

---

### 🐛 问题3: 制动模式简化
**位置**: `MockBrakeDevice.cpp:101-111`
```cpp
actualVoltageV = state->setVoltageV; // 假设电压立即达到设定值
```

**影响**: 未模拟电源响应延迟和负载特性

**建议**: 添加一阶惯性环节模拟真实电源动态

---

## 七、成功标准

### 验收条件
1. ✅ 所有P0优先级检查点通过率 > 95%
2. ✅ 所有P1优先级检查点通过率 > 90%
3. ✅ 完整测试流程无物理规律违规报警
4. ✅ 离线分析报告显示物理量符合性 > 90%

### 交付物清单
- [x] 本分析方案文档
- [ ] 物理规律单元测试代码 (阶段2)
- [ ] PhysicsValidator 运行时监控工具 (阶段3)
- [ ] 遥测数据分析脚本 (阶段4)
- [ ] 物理规律符合性报告 (阶段4)

---

**文档状态**: ✅ 已完成  
**下一步行动**: 等待Leader分配具体实施任务给各角色成员
