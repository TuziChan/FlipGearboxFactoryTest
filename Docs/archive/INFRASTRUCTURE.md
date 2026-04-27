# FlipGearboxFactoryTest — Infrastructure & Automation Toolchain

## 新增基础设施组件

### 1. 统一硬件模拟器外壳 (HardwareSimulationHarness)
- **路径**: `src/infrastructure/simulation/HardwareSimulationHarness.h/.cpp`
- **职责**: 将 SimulatedMotorDevice、SimulatedTorqueDevice、SimulatedEncoderDevice、SimulatedBrakeDevice 以及 SimulatedBusController 封装为单一管理单元
- **特性**:
  - 生命周期管理 (`initialize` / `shutdown`)
  - 统一状态控制 (`setMotorState`, `setBrakeOutput`, `setEncoderZeroOffset`)
  - 故障注入 API (`injectBusFault`, `injectSensorFault`, `clearFaults`)
  - 磁铁位置配置 (支持翻面齿轮箱角度测试)
  - QML 友好属性 (`currentEncoderAngle`, `currentMotorSpeedRpm`, `currentTorqueNm`)
  - 统计信息收集

### 2. 场景定义与执行器 (SimulationScenario + ScenarioExecutor)
- **路径**: `src/infrastructure/simulation/SimulationScenario.h/.cpp`
- **职责**: 以声明式 JSON 定义测试场景，支持时间线驱动的故障注入和动作编排
- **特性**:
  - `InitialState` — 初始物理状态
  - `FaultEvent` / `ActionEvent` — 基于 tick 的调度
  - `PassCriteria` — 通过准则
  - JSON 序列化/反序列化（支持从文件加载场景）
  - 逐步执行模式（用于调试）

### 3. 统一测试编排器 (TestOrchestrator)
- **路径**: `src/infrastructure/testing/TestOrchestrator.h/.cpp`
- **职责**: 自动发现、执行并聚合所有测试可执行文件的结果
- **特性**:
  - 自动发现 `*Tests.exe` / `*Test.exe`
  - 支持过滤器运行特定测试子集
  - 按类别运行
  - 多种报告格式：JSON、HTML、JUnit XML、Markdown
  - CTest 集成

### 4. AI 团队执行监控 (TeamExecutionMonitor)
- **路径**: `src/infrastructure/testing/TeamExecutionMonitor.h/.cpp`
- **职责**: 结构化记录 AI 团队协作事件，输出 JSONL
- **特性**:
  - 任务分配/完成日志
  - 代码变更追踪（作者、文件、行数）
  - 构建/测试事件
  - 错误与指标记录
  - 决策记录（含备选方案）
  - 会话摘要与报告生成
  - 线程安全（QMutex 保护文件 I/O）

### 5. 构建流水线运行器 (BuildPipelineRunner)
- **路径**: `src/infrastructure/testing/BuildPipelineRunner.h/.cpp`
- **职责**: 自动化 CMake 配置 → 编译 → 测试 → 报告 全流程
- **特性**:
  - 自动检测 Qt CMake 目录和生成器
  - 支持 Clean Build
  - 环境变量自动注入（QTDIR、PATH）
  - 分段计时与详细输出捕获
  - JSON 流水线报告

### 6. 统一测试运行器可执行文件 (UnifiedTestRunner)
- **路径**: `tests/framework/UnifiedTestRunner.cpp`
- **职责**: 命令行工具，封装 TestOrchestrator 供 CI/CD 调用
- **命令行选项**:
  - `--build-dir` / `-b`
  - `--output-dir` / `-o`
  - `--filter` / `-f`
  - `--format` (json,html,junit,md)
  - `--ctest`
  - `--qt-path`

### 7. 硬件模拟器单元测试 (HardwareSimulationHarnessTests)
- **路径**: `tests/simulation/HardwareSimulationHarnessTests.cpp`
- **覆盖**:
  - Harness 生命周期
  - 设备访问器
  - 电机/制动器/编码器状态控制
  - 故障注入（总线和传感器）
  - 磁铁位置配置
  - 场景执行（含故障注入场景）
  - JSON 序列化往返测试

### 8. 自动化构建脚本
- **PowerShell**: `scripts/Build-And-Test.ps1`
- **Batch**: `scripts/Build-And-Test.bat`
- **功能**:
  - 参数化 Qt 路径、BuildType、Clean、SkipTests
  - 自动生成器检测（Ninja > MinGW Makefiles）
  - 环境变量自动配置
  - 四阶段流水线：Configure → Build → Test → Report
  - 详细的 JSON 流水线报告

## CMakeLists.txt 增强

- 新增源文件到主目标
- 新增 `UnifiedTestRunner` 可执行目标
- 新增 `HardwareSimulationHarnessTests` 可执行目标
- 新增 `run-all-tests` 自定义目标（依赖所有测试目标，通过 CTest 运行）
- 所有新增测试已注册到 CTest

## 与现有代码的集成点

| 组件 | 集成方式 |
|------|----------|
| HardwareSimulationHarness | 被 StationRuntimeFactory 在 mockMode 下使用（可选替换原有分散创建） |
| SimulationScenario | 可由 QML 或 C++ 测试代码加载 JSON 场景文件 |
| TestOrchestrator | 可被 main() 在自动化测试模式下调用 |
| TeamExecutionMonitor | 在 main() 中注册，记录团队事件到 `logs/team_execution_<session>.jsonl` |
| BuildPipelineRunner | 可被 CI 脚本或自动化工具调用 |

## 后续建议

1. **CI/CD 集成**: 将 `scripts/Build-And-Test.ps1` 接入 GitHub Actions / GitLab CI
2. **场景库**: 在 `scenarios/` 目录中积累标准测试场景 JSON 文件
3. **监控仪表板**: 解析 `team_execution_*.jsonl` 生成实时团队执行看板
4. **性能基线**: 利用 TestOrchestrator 的 Markdown 报告建立每次构建的性能基线对比
