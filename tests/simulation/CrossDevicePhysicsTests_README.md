# 跨设备集成测试 - 物理规律验证

## 测试文件
	ests/simulation/CrossDevicePhysicsTests.cpp

## 测试目标
验证协议层和设备层的物理规律一致性，确保Mock仿真数据符合真实物理规律。

## 测试覆盖的物理规律

### R1: 角速度-转速一致性
- **物理公式**: 编码器角速度(RPM) ≈ 扭矩传感器转速(RPM)
- **测试用例**:
  - 	est_R1_SpeedConsistency_ForwardRotation: 正转时速度一致性
  - 	est_R1_SpeedConsistency_ReverseRotation: 反转时速度一致性
- **判据**: 相对误差 < 5%

### R4: 功率守恒定律
- **物理公式**: P(W) = T(N·m) × ω(rad/s) = T × RPM × 2π/60
- **测试用例**: 	est_R4_PowerConservation
- **判据**: 功率计算误差 < 10%

### R6: 电机电流-负载关系
- **物理公式**: I_motor = I_base + k₁×duty + k₂×brake_load - k₃×speed
- **测试用例**:
  - 	est_R6_MotorCurrentLoadRelation: 电流随制动负载增加
  - 	est_R6_IdleCurrentLimit: 空载电流限制 < 3A
- **判据**: 
  - 电流单调递增
  - 负载系数 ΔI_motor/ΔI_brake ≈ 0.3 (±0.1)

### R11: 制动减速动力学
- **物理模型**: 制动电流上升 → 扭矩增加 → 转速下降
- **测试用例**: 	est_R11_BrakingDecelerationDynamics
- **判据**: 
  - 相关系数 < -0.7 (强负相关)
  - 速度降低 > 200 RPM

### 综合场景测试
- **测试用例**: 	est_CrossDevice_CompleteBrakingScenario
- **验证内容**:
  - 多设备协同工作
  - 加速阶段物理量正确
  - 制动阶段物理规律保持
  - 所有设备数据一致性

## 构建和运行

### 方法1: 使用CMake/Ninja
\\\ash
cd build
ninja CrossDevicePhysicsTests
./CrossDevicePhysicsTests
\\\

### 方法2: 使用Qt Creator
1. 打开项目 FlipGearboxFactoryTest.pro 或 CMakeLists.txt
2. 在项目树中找到 CrossDevicePhysicsTests
3. 右键 → Run Test

### 方法3: 使用CTest
\\\ash
cd build
ctest -R CrossDevicePhysicsTests -V
\\\

## 测试输出示例

\\\
********* Start testing of CrossDevicePhysicsTests *********
Config: Using QtTest library 6.x.x

PASS   : CrossDevicePhysicsTests::initTestCase()
INFO   : CrossDevicePhysicsTests::test_R1_SpeedConsistency_ForwardRotation() R1 Speed Consistency Test:
INFO   : CrossDevicePhysicsTests::test_R1_SpeedConsistency_ForwardRotation()   Valid samples: 100
INFO   : CrossDevicePhysicsTests::test_R1_SpeedConsistency_ForwardRotation()   Average error: 0.15 %
INFO   : CrossDevicePhysicsTests::test_R1_SpeedConsistency_ForwardRotation()   Maximum error: 0.82 %
PASS   : CrossDevicePhysicsTests::test_R1_SpeedConsistency_ForwardRotation()

INFO   : CrossDevicePhysicsTests::test_R4_PowerConservation() R4 Power Conservation:
INFO   : CrossDevicePhysicsTests::test_R4_PowerConservation()   Torque: 1.234 N·m
INFO   : CrossDevicePhysicsTests::test_R4_PowerConservation()   Speed: 850.5 RPM
INFO   : CrossDevicePhysicsTests::test_R4_PowerConservation()   Actual power: 109.87 W
INFO   : CrossDevicePhysicsTests::test_R4_PowerConservation()   Theoretical power: 109.92 W
INFO   : CrossDevicePhysicsTests::test_R4_PowerConservation()   Relative error: 0.05 %
PASS   : CrossDevicePhysicsTests::test_R4_PowerConservation()

...

Totals: 8 passed, 0 failed, 0 skipped, 0 blacklisted
********* Finished testing of CrossDevicePhysicsTests *********
\\\

## 失败诊断

### 速度不一致 (R1失败)
- **原因**: SimulatedTorqueDevice 和 SimulatedEncoderDevice 使用不同的速度计算
- **检查**: 确认两者都从 SimulationContext::encoderAngularVelocityRpm() 读取

### 功率守恒违规 (R4失败)
- **原因**: 功率计算公式错误或噪声过大
- **检查**: 验证 P = T × RPM × 2π/60 实现正确

### 电流负载系数异常 (R6失败)
- **原因**: SimulatedMotorDevice 中的负载系数不是 0.3
- **检查**: 查看 loadCurrent = brakeLoad * 0.3 是否正确

### 制动相关性弱 (R11失败)
- **原因**: 制动效果不明显或延迟过大
- **检查**: 验证 speedReduction = brakeLoad × 100 的系数

## 相关文档
- [MOCK_PHYSICS_VALIDATION_PLAN.md](../../Docs/MOCK_PHYSICS_VALIDATION_PLAN.md)
- [MOCK_VALIDATION_TASK_BREAKDOWN.md](../../Docs/MOCK_VALIDATION_TASK_BREAKDOWN.md)

## 维护说明
- 测试阈值可根据实际硬件特性调整
- 新增物理规律检查时，参考现有测试用例结构
- 保持测试独立性，每个测试用例应能单独运行
