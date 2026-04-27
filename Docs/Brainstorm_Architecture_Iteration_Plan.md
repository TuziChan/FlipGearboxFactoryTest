# FlipGearboxFactoryTest — 架构分析与迭代方案头脑风暴报告

**任务ID**: `277dbc65-fa34-4b93-8af8-c78097cf44b7`  
**生成日期**: 2026-04-23  
**生成角色**: 头脑风暴 (Brainstorm)  
**基线版本**: v1.0-production-ready (2026-04-20)  
**Qt版本**: 6.11.0 (llvm-mingw-64)  

---

## 目录

1. [当前架构全景图](#1-当前架构全景图)
2. [代码质量与健康度评估](#2-代码质量与健康度评估)
3. [需求映射与Gap分析](#3-需求映射与gap分析)
4. [方案A：渐进式优化路线（保守稳健）](#4-方案a渐进式优化路线保守稳健)
5. [方案B：Qt6.11现代化路线（技术领先）](#5-方案bqt611现代化路线技术领先)
6. [方案C：AI团队协作增强路线（流程革新）](#6-方案cai团队协作增强路线流程革新)
7. [方案D：测试完备性路线（质量优先）](#7-方案d测试完备性路线质量优先)
8. [综合优先级矩阵与推荐路线](#8-综合优先级矩阵与推荐路线)
9. [详细实施路线图（四阶段）](#9-详细实施路线图四阶段)
10. [风险对冲与兜底策略](#10-风险对冲与兜底策略)

---

## 1. 当前架构全景图

### 1.1 分层架构

```
┌─────────────────────────────────────────────────────────────┐
│  UI Layer (QML)                                             │
│  ├── 60+ 可复用组件 (AppButton/AppCard/AppTheme...)         │
│  ├── 页面: TestExecutionPage | DiagnosticsPage | ...        │
│  └── ChartPainter (C++自定义渲染)                           │
├─────────────────────────────────────────────────────────────┤
│  ViewModel Layer (C++)                                      │
│  ├── TestExecutionViewModel  (核心状态总线)                  │
│  ├── DiagnosticsViewModel                                    │
│  ├── HistoryViewModel                                        │
│  └── RecipeViewModel                                         │
├─────────────────────────────────────────────────────────────┤
│  Domain Layer (C++)                                         │
│  ├── GearboxTestEngine  (状态机, ~1400行)                    │
│  ├── RecipeValidator                                         │
│  └── 数据模型: TestRecipe/TestResults/TestRunState/...      │
├─────────────────────────────────────────────────────────────┤
│  Infrastructure Layer (C++)                                 │
│  ├─ Bus: ModbusRtuBusController + ModbusFrame               │
│  ├─ Devices: 4类硬件接口 + 真实驱动实现                      │
│  ├─ Simulation: SimulationContext + Mock/Simulated设备       │
│  │             (Mock*Device + Simulated*Device 双轨制)      │
│  ├─ Acquisition: AcquisitionScheduler + TelemetryBuffer     │
│  ├─ Config: StationConfig/ConfigLoader/RuntimeManager        │
│  ├─ Services: HistoryService + RecipeService                 │
│  ├─ Reporting: JsonReportWriter + ReportGenerator            │
│  ├─ Logging: LogManager (轮转+清理)                         │
│  └─ Monitoring: PerformanceMonitor                           │
└─────────────────────────────────────────────────────────────┘
```

### 1.2 核心数据流

```
硬件设备 ←Modbus RTU→ BusController → Device接口 → TestEngine(ViewModel)
                                                        ↓
SimulationContext ← tick → Simulated*Device ────────┘
                     ↑
            Mock*Device (测试专用)
```

### 1.3 双模式运行架构

| 模式 | 入口 | 运行时 | 用途 |
|------|------|--------|------|
| **真实模式** | 默认启动 | StationRuntimeFactory创建真实设备 | 产线生产 |
| **仿真/Mock模式** | `--mock`参数 | StationRuntimeFactory创建模拟设备 | 开发调试/CI测试 |

**关键洞察**: 项目已经建立了非常完整的**仿真双轨制**，这是工业上位机项目中极为先进的设计。SimulationContext提供了确定性的物理模拟，Mock层提供了故障注入能力。

---

## 2. 代码质量与健康度评估

### 2.1 规模统计

| 模块 | 文件数 | 代码行数(估算) | 健康度 |
|------|--------|----------------|--------|
| Domain | 9 | ~2,500 | ⭐⭐⭐⭐⭐ |
| ViewModel | 8 | ~1,500 | ⭐⭐⭐⭐☆ |
| Infrastructure(总线+设备) | 16 | ~3,000 | ⭐⭐⭐⭐⭐ |
| Infrastructure(仿真) | 30 | ~4,100 | ⭐⭐⭐⭐⭐ |
| UI/QML组件 | 60+ | ~15,000+ | ⭐⭐⭐☆☆ |
| 测试代码 | 30+ | ~8,000+ | ⭐⭐⭐⭐☆ |

### 2.2 已解决的P0问题 ✅

- Modbus RTU 0x04/0x05功能码完整实现
- 并发安全加固（QMutex + QMutexLocker + 无锁快照）
- 错误处理机制（分级重试、资源清理、降级策略）
- 边界约束保护（RecipeValidator、缓冲区上限、类型溢出保护）

### 2.3 现存技术债务 ⚠️

| 债务项 | 严重程度 | 影响范围 | 说明 |
|--------|----------|----------|------|
| **QML绑定冗余** | 中 | UI层 | TestExecutionPage中大量`Connections`手动赋值，未利用Qt6绑定引擎优化 |
| **QVariantMap类型擦除** | 中 | ViewModel→UI | 结果数据用`QVariantMap/QVariantList`传递，丢失编译时类型安全 |
| **测试环境DLL依赖** | 中 | CI/CD | Git Bash下测试运行失败，阻碍自动化验证 |
| **GearboxTestEngine单类过大** | 低 | Domain | ~1400行，含20+个子状态处理函数，可考虑按Phase拆分 |
| **缺失builder/tester角色** | 低 | 团队 | 交付计划中已识别，但尚未正式集成到SpectrAI |

### 2.4 架构优势 💪

1. **设备抽象彻底**: Domain层仅依赖`I*Device`接口，与总线解耦
2. **配置驱动**: `station.json` + `recipes/*.json` 驱动硬件拓扑和测试流程
3. **确定性仿真**: SimulationContext的tick驱动物理模型，测试100%可复现
4. **实时性保障**: 33ms控制周期，互斥锁开销<1μs
5. **单向依赖**: UI→VM→Domain→Infra，无反向依赖

---

## 3. 需求映射与Gap分析

### 3.1 需求→现状映射

| 用户需求 | 当前状态 | Gap | 优先级 |
|----------|----------|-----|--------|
| **自动化开发** | 已有CMake+CTest+run_tests.bat | 缺少CI流水线、自动构建触发 | P1 |
| **AI团队监控** | 无 | 完全缺失：无法实时监控各Agent执行状态 | P0 |
| **自动头脑风暴** | 无 | 完全缺失：无自动触发机制 | P1 |
| **Qt6新特性应用** | 已用`pragma ComponentBehavior: Bound` | 未用QML StateMachine/Inline Components/TableView | P1 |
| **多角色迭代完善** | 7角色已定义 | 缺builder/tester角色；缺角色间自动协调 | P1 |
| **Bug修复** | P0已修复 | 需建立持续缺陷跟踪机制 | P1 |
| **性能优化** | 33ms达标 | 内存、启动时间、QML渲染可进一步优化 | P2 |
| **自动化测试** | 11+套件，AutoTestFramework | 测试运行环境依赖；缺UI自动化测试 | P1 |
| **Mock测试** | Mock/Simulated双轨制 | 场景覆盖可扩展（故障注入、边界条件） | P2 |
| **硬件模拟** | 完整仿真层 | 物理模型可更真实（温度、摩擦非线性） | P2 |
| **测试执行页完整测试** | 手动QML Smoke Test | 缺QML TestCase自动化、缺截图对比 | P1 |

### 3.2 关键Gap详解

#### Gap-1: AI团队执行监控 (P0)

**现状**: 各Agent独立工作，Leader通过任务系统分配，但无**实时可视化面板**监控：
- 各Agent当前状态（空闲/执行中/阻塞）
- 任务队列和完成进度
- 代码变更实时流转（Dev→Reviewer→Builder→Tester）

**影响**: Leader无法实时掌握团队负载均衡，可能出现某角色瓶颈而其他角色空闲。

#### Gap-2: Qt6.11特性利用不足 (P1)

**现状**: 仅使用了基础Qt6特性，未充分利用6.5+的高级特性。

**具体缺失**:
- `QtQml.StateMachine`: TestExecutionPage状态转换仍用字符串匹配
- `Inline Components`: 重复的metric display模式未抽象
- `TableView`: 结果展示用ListView手动布局
- `qmltc` (QML Type Compiler): 未启用编译优化

#### Gap-3: 自动化测试闭环不完整 (P1)

**现状**: 测试编译成功，但Git Bash环境运行失败（DLL依赖）。

**阻塞点**:
- 无法在AI Agent环境中自动执行`ctest`
- 缺少自动截图/UI回归测试
- 测试失败无法自动创建修复任务

---

## 4. 方案A：渐进式优化路线（保守稳健）

> **核心理念**: 不引入新技术栈，在现有架构上填坑补漏，风险最低。

### 4.1 目标

3周内将项目从"可上线"提升到"可稳定迭代"。

### 4.2 关键动作

| 序号 | 动作 | 负责人 | 工期 | 产出 |
|------|------|--------|------|------|
| A1 | 修复测试DLL依赖，创建`scripts/run_ci_tests.ps1` | infra-dev | 2天 | CI可运行测试 |
| A2 | UI绑定优化：减少Connections，改用property binding | ui-dev | 3天 | 代码-50行，性能↑ |
| A3 | 扩展Mock场景：覆盖通信超时/ CRC错误/设备离线 | protocol-dev | 3天 | 3个新Mock测试类 |
| A4 | GearboxTestEngine按Phase拆分为策略类 | domain-dev | 5天 | 单类<500行 |
| A5 | 添加日志持久化文件输出 | infra-dev | 2天 | 日志文件+轮转 |

### 4.3 优缺点

| ✅ 优点 | ❌ 缺点 |
|---------|---------|
| 技术风险极低 | 无法解决AI监控Gap |
| 所有成员熟悉技术栈 | 未利用Qt6.11优势 |
| 短期内可见改进 | 长期竞争力不足 |

---

## 5. 方案B：Qt6.11现代化路线（技术领先）

> **核心理念**: 充分利用Qt6.11最新特性，提升代码质量和运行时性能。

### 5.1 关键技术升级

#### B1. QML State Machine重构 (高优先级)

**当前痛点**: `TestExecutionPage.qml`中`currentPhaseIndex`通过字符串匹配更新：
```qml
// 当前代码（痛点）
function updateCurrentPhaseIndex() {
    if (phaseTitle.includes("Homing")) currentPhaseIndex = 0
    else if (phaseTitle.includes("Idle")) currentPhaseIndex = 1
    // ... 字符串匹配，易出错
}
```

**Qt6方案**:
```qml
import QtQml.StateMachine

StateMachine {
    id: testStateMachine
    initialState: idleState
    
    State {
        id: idleState
        SignalTransition {
            targetState: homingState
            signal: viewModel.runningChanged
            guard: viewModel.running
        }
    }
    State {
        id: homingState
        onEntered: { currentPhaseIndex = 0; stepModel.setProperty(0, "state", "run") }
        SignalTransition {
            targetState: idleTestState
            signal: viewModel.currentPhaseChanged
            guard: viewModel.currentPhase === "IdleRun"
        }
    }
    // ... 其他状态
}
```

**收益**:
- 消除字符串匹配，编译时检查状态转换
- 状态进入/退出动作自动化
- 可视化状态图（可通过qmllint辅助分析）

#### B2. Inline Components消除重复代码

**当前痛点**: `metricColor/metricValue/metricUnit`三个函数重复处理遥测数据显示。

**Qt6方案**:
```qml
// TestExecutionPage.qml 内定义
component MetricDisplay: RowLayout {
    required property string label
    required property real value
    required property string unit
    required property color valueColor
    // 统一显示逻辑
}

// 使用
MetricDisplay { label: "转速"; value: speedValue; unit: "RPM"; valueColor: theme.textPrimary }
```

**收益**: 减少80+行重复代码，类型安全。

#### B3. TableView替代ListView模拟表格

**当前痛点**: `angleModel`/`loadModel`/`idleModel`用ListView+手动RowLayout模拟表格。

**Qt6方案**: `TableView` + `TableModel` 原生支持列宽调整、排序、虚拟化。

#### B4. qmltc编译器启用

**操作**: 在CMakeLists.txt中启用`qt6_quick_compiler`或`qmltc`。

**收益**: QML预编译为C++，启动速度提升20-30%，运行时性能提升。

### 5.2 实施计划

| 阶段 | 内容 | 工期 |
|------|------|------|
| B-Phase1 | Property Binding优化（快速见效） | 2天 |
| B-Phase2 | Inline Components重构 | 3天 |
| B-Phase3 | QML State Machine迁移 | 4天 |
| B-Phase4 | TableView替换 | 3天 |
| B-Phase5 | qmltc启用+性能验证 | 2天 |

### 5.3 优缺点

| ✅ 优点 | ❌ 缺点 |
|---------|---------|
| 充分利用Qt6.11投资 | 需要ui-dev学习State Machine新API |
| 性能可量化提升 | 重构期间可能引入回归 |
| 代码可维护性显著改善 | 与方案A相比，工期增加~30% |
| 为后续AI监控提供现代化UI基础 | |

---

## 6. 方案C：AI团队协作增强路线（流程革新）

> **核心理念**: 构建"元层"基础设施，让AI团队自组织、自监控、自优化。

### 6.1 创意概念：TeamOps Dashboard

在现有上位机中嵌入一个**"开发运维面板"**，将AI团队的协作状态可视化。

```
┌─────────────────────────────────────────────────────────────┐
│  [生产系统] FlipGearboxFactoryTest 上位机                     │
│                                                             │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐           │
│  │ 测试执行 │ │ 设备诊断 │ │ 历史记录 │ │ 配方管理 │  + 新标签 │
│  └─────────┘ └─────────┘ └─────────┘ └─────────┘           │
│                                                             │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  🆕 TeamOps 面板 (QML + C++ ViewModel)              │   │
│  │  ┌────────────┐ ┌────────────┐ ┌────────────┐      │   │
│  │  │ 角色状态   │ │ 任务流水线 │ │ 代码健康度 │      │   │
│  │  │ 🟢domain   │ │ Dev→Review│ │ 覆盖率75% │      │   │
│  │  │ 🟡ui-dev   │ │ →Build→Test│ │ 编译✅    │      │   │
│  │  │ 🔴reviewer │ │  3 task    │ │ 测试⚠️   │      │   │
│  │  └────────────┘ └────────────┘ └────────────┘      │   │
│  └─────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

### 6.2 架构设计：TeamOps子系统

```
src/teamops/
├── TeamMonitorService.h/cpp      # 轮询各角色状态
├── TaskPipelineModel.h/cpp       # 任务流水线数据模型
├── CodeHealthMetrics.h/cpp       # 代码健康度采集
├── AutoBrainstormTrigger.h/cpp   # 自动头脑风暴触发器
└── TeamOpsViewModel.h/cpp        # QML绑定
```

#### C1. 角色状态实时监控

**机制**:
- 利用已有的`team_get_tasks` MCP工具，由`TeamMonitorService`定时轮询
- 将任务状态映射为QML可绑定的`TeamRoleStatus`对象
- 状态: `Idle`/`Executing`/`Blocked`/`ReviewPending`

**QML展示**:
```qml
TeamRoleCard {
    roleName: "protocol-dev"
    status: teamOpsViewModel.roleStatus("protocol-dev")
    currentTask: teamOpsViewModel.currentTask("protocol-dev")
    completedToday: teamOpsViewModel.completedCount("protocol-dev")
}
```

#### C2. 任务流水线可视化

**机制**:
- 跟踪每个代码变更的流转：Dev提交 → Reviewer审查 → Builder构建 → Tester测试
- 利用Git状态 + CTest结果 + 文件系统监控

**价值**: Leader一眼看出瓶颈环节（如Tester队列堆积）。

#### C3. 自动头脑风暴触发器

**触发条件**:
1. **架构变更前**: 当任务涉及跨层修改（Domain+VM+UI），自动触发头脑风暴生成方案
2. **Bug复发时**: 同一模块7天内出现3次修复，自动触发根因分析+改进方案
3. **技术债务告警**: 当某文件复杂度超过阈值（如GearboxTestEngine.cpp >1500行），自动触发重构建议

**实现**:
```cpp
class AutoBrainstormTrigger : public QObject {
    // 监听文件系统变更、Git日志、CTest结果
    // 条件满足时，调用LLM生成结构化建议
    // 输出到 Docs/brainstorm/auto/YYYY-MM-DD-trigger.md
};
```

#### C4. 代码健康度仪表板

**指标**:
- 测试覆盖率趋势（按模块）
- 编译警告数量趋势
- 代码复杂度（cyclomatic complexity）
- 文档覆盖率

**采集方式**:
- 编译时通过CMake Custom Command生成`metrics.json`
- `CodeHealthMetrics`读取并计算趋势

### 6.3 优缺点

| ✅ 优点 | ❌ 缺点 |
|---------|---------|
| 根本性解决"AI团队监控"需求 | 引入全新子系统，增加架构复杂度 |
| 自动化头脑风暴，减少Leader负担 | 需要infra-dev投入较多工期 |
| 代码健康度可视化，预防技术债务 | 与生产系统混布，需考虑隔离 |
| 可复用于其他Qt项目 | 初期ROI不明确 |

---

## 7. 方案D：测试完备性路线（质量优先）

> **核心理念**: 构建工业级的测试金字塔，达到可无人值守的发布标准。

### 7.1 测试金字塔现状 vs 目标

```
         /\
        /  \    E2E UI测试 (当前: 0% → 目标: 15%)
       /----\   ──────────────────────────────────
      /      \  集成测试 (当前: 30% → 目标: 35%)
     /--------\ ──────────────────────────────────
    /          \单元测试 (当前: 70% → 目标: 50%)
   /────────────\─────────────────────────────────
```

### 7.2 关键动作

#### D1. QML自动化测试框架

**技术选型**: Qt Test框架的`QQuickTest` + `QTest::mouseClick`

**实施**:
```cpp
// tests/ui/TestExecutionPageInteractiveTests.cpp
class TestExecutionPageInteractiveTests : public QObject {
    Q_OBJECT
private slots:
    void test_startButton_triggersTest() {
        QQuickWindow* window = createTestWindow("TestExecutionPage.qml");
        QQuickItem* startBtn = findItemByObjectName(window, "startTestButton");
        QTest::mouseClick(window, Qt::LeftButton, {}, startBtn->mapToScene({}).toPoint());
        
        QSignalSpy spy(testViewModel, &TestExecutionViewModel::runningChanged);
        QVERIFY(spy.wait(1000));
        QVERIFY(testViewModel->running());
    }
};
```

**增强**: 集成`grabWindow()`截图对比，实现视觉回归测试。

#### D2. Mock场景扩展矩阵

| 场景 | 覆盖 | 测试文件 |
|------|------|----------|
| 正常流程 | ✅ | GearboxSimulationIntegrationTests |
| 通信CRC错误 | ⚠️ | 需新增: `MockCommunicationFaultTests` |
| 设备超时离线 | ⚠️ | 需新增: `MockDeviceOfflineTests` |
| 电机堵转 | ❌ | 需新增: `MockMotorStallTests` |
| 编码器丢脉冲 | ❌ | 需新增: `MockEncoderGlitchTests` |
| 制动电源过流 | ❌ | 需新增: `MockBrakeOvercurrentTests` |
| 配方参数越界 | ✅ | BoundaryProtectionTests |
| 长时间运行 | ✅ | LongRunningStabilityTest |

#### D3. 性能基准测试

```cpp
// tests/performance/TestEnginePerformanceTests.cpp
void test_33msCycleConsistency() {
    QVector<qint64> cycleTimes;
    for (int i = 0; i < 1000; ++i) {
        QElapsedTimer timer;
        timer.start();
        testEngine->onCycleTick();
        cycleTimes.append(timer.nsecsElapsed());
    }
    
    // 95%分位 < 33ms * 0.3 = 10ms (留出70%裕量给通信)
    auto p95 = percentile(cycleTimes, 0.95);
    QVERIFY(p95 < 10'000'000); // 10ms in ns
}
```

#### D4. 硬件在环(HIL)测试接口

为真实硬件测试预留标准化接口：
```cpp
class IHilTestFixture {
public:
    virtual bool connectHardware() = 0;
    virtual bool runCalibrationSequence() = 0;
    virtual bool verifySafetyInterlocks() = 0;
};
```

### 7.3 优缺点

| ✅ 优点 | ❌ 缺点 |
|---------|---------|
| 质量置信度最高 | 测试编写和维护成本高 |
| 支持无人值守发布 | 工期长（4-6周） |
| 回归防护最强 | 部分测试仍需真实硬件 |
| 工业客户认可度最高 | |

---

## 8. 综合优先级矩阵与推荐路线

### 8.1 四象限评估

```
高价值 ^
       │
   C   │   B+D
  AI   │  Qt6现代化+测试完备
 监控   │   (推荐主路线)
       │
───────┼──────────→ 低风险
       │
   不   │   A
  做   │  渐进优化
       │  (保底路线)
       │
       └──────────────────→ 高价值
```

### 8.2 推荐路线：**B-C-D融合路线**

> 以**方案B（Qt6现代化）**为技术基线，**方案C（AI监控）**为流程增强，**方案D（测试完备）**为质量底线，按四阶段滚动实施。

**为什么不是纯A？**
- 项目已完成P0修复，处于"上线就绪"状态，继续渐进优化ROI低
- 用户需求明确提到"Qt6最新特性"和"AI团队监控"，这是差异化价值点
- 团队有7个AI Agent，手动协调效率低，自动化监控是刚需

**为什么不是纯D？**
- 测试完备性重要，但单独实施无法解决"迭代效率"问题
- 需要与现代化结合（Qt6 StateMachine使UI测试更容易）

### 8.3 优先级矩阵

| 优先级 | 需求 | 方案 | 负责角色 | 预计工期 | 阻塞项 |
|--------|------|------|----------|----------|--------|
| **P0** | AI团队监控面板 | C1+C2 | infra-dev + ui-dev | 5天 | 无 |
| **P0** | 自动头脑风暴触发 | C3 | infra-dev | 3天 | C1完成 |
| **P1** | Qt6 StateMachine重构 | B1 | ui-dev | 4天 | 无 |
| **P1** | UI绑定优化 | B2 | ui-dev | 2天 | 可与B1并行 |
| **P1** | 修复测试CI环境 | D1(部分) | infra-dev | 2天 | 无 |
| **P1** | Mock场景扩展 | D2 | protocol-dev | 4天 | 无 |
| **P2** | TableView替换 | B3 | ui-dev | 3天 | B1完成 |
| **P2** | qmltc编译器 | B4 | infra-dev | 2天 | B3完成 |
| **P2** | QML交互测试 | D1(完整) | ui-dev | 5天 | B1完成 |
| **P2** | 代码健康度仪表板 | C4 | infra-dev | 4天 | C1完成 |
| **P3** | GearboxTestEngine拆分 | A4 | domain-dev | 5天 | 无紧急需求 |
| **P3** | HIL测试接口 | D4 | protocol-dev | 5天 | 需硬件 |

---

## 9. 详细实施路线图（四阶段）

### Phase 1: 监控基线（Week 1）

**目标**: 让Leader能"看见"团队。

**任务列表**:
1. **infra-dev**: 实现`TeamMonitorService`，轮询任务状态，暴露QML属性
2. **ui-dev**: 设计`TeamOpsPanel.qml`最小可用版本（角色状态卡片+任务计数器）
3. **infra-dev**: 创建`scripts/run_ci_tests.ps1`，解决DLL依赖问题
4. **protocol-dev**: 实现3个新Mock故障场景（CRC错误、超时离线、电机堵转）

**验收标准**:
- [ ] TeamOps面板显示所有7个角色的实时状态
- [ ] `scripts/run_ci_tests.ps1`在PowerShell中100%通过
- [ ] 新增Mock测试编译通过

---

### Phase 2: UI现代化（Week 2）

**目标**: 测试执行页技术升级。

**任务列表**:
1. **ui-dev**: `TestExecutionPage.qml` Property Binding优化，删除50%+ Connections代码
2. **ui-dev**: Inline Components重构（MetricDisplay等）
3. **ui-dev**: 引入`QtQml.StateMachine`，迁移测试阶段状态管理
4. **domain-dev**: 配合State Machine，在GearboxTestEngine中增加`currentPhaseEnum`信号（替代字符串）

**验收标准**:
- [ ] `qmllint`零警告
- [ ] TestExecutionPage代码行数减少20%+
- [ ] 状态转换逻辑从字符串匹配改为枚举/状态机

---

### Phase 3: 质量增强（Week 3-4）

**目标**: 测试覆盖率和自动化程度达到工业标准。

**任务列表**:
1. **ui-dev**: QML交互测试框架（`TestExecutionPageInteractiveTests`）
2. **protocol-dev**: 完整Mock场景矩阵（6个故障场景全部实现）
3. **infra-dev**: 代码健康度采集脚本（CMake Custom Command集成）
4. **infra-dev**: TableView替换结果展示表格
5. **reviewer**: 全量代码审查，确保Phase 1-2无回归

**验收标准**:
- [ ] QML交互测试至少覆盖：开始测试、停止测试、切换页面
- [ ] Mock测试覆盖6个故障场景
- [ ] 代码健康度仪表板显示覆盖率、复杂度、警告数

---

### Phase 4: 智能迭代（Week 5+）

**目标**: 系统能自我优化。

**任务列表**:
1. **infra-dev**: 实现`AutoBrainstormTrigger`
   - 监听Git变更、编译结果、测试失败
   - 条件满足时自动生成头脑风暴报告
2. **infra-dev**: 启用`qmltc`，性能基准测试
3. **domain-dev**: 按需拆分GearboxTestEngine（非紧急）
4. **全团队**: 根据TeamOps面板数据，优化任务分配策略

**验收标准**:
- [ ] 自动头脑风暴成功触发至少3次并生成有价值建议
- [ ] qmltc启用后启动时间减少15%+

---

## 10. 风险对冲与兜底策略

### 10.1 主要风险

| 风险 | 概率 | 影响 | 对冲策略 |
|------|------|------|----------|
| Qt6 StateMachine与现有逻辑冲突 | 中 | 高 | **并行保留旧逻辑**: 新逻辑通过feature flag切换，验证通过后再删除旧代码 |
| TeamOps面板引入性能开销 | 低 | 中 | **异步轮询**: 监控服务运行在独立QThread，不阻塞UI |
| QML交互测试不稳定 | 中 | 中 | **截图对比容差**: 允许像素级差异<1%，使用模糊比对 |
| 多任务并行导致集成冲突 | 高 | 中 | **每日集成点**: 每天17:00固定合并，冲突当日解决 |
| AI自动头脑风暴建议质量低 | 中 | 低 | **人工审核门**: 自动建议需Leader确认后才生成任务 |

### 10.2 兜底方案

如果Phase 1-2出现严重阻塞：
1. **立即回退到方案A**：只修复测试环境+UI绑定优化，跳过StateMachine和TeamOps
2. **保留已完成代码**：使用Git分支隔离，不污染主分支
3. **重新评估**：由Leader召集全角色复盘，识别阻塞根因

---

## 附录A：头脑风暴触发建议清单

以下场景建议自动触发头脑风暴：

| # | 触发条件 | 建议生成内容 |
|---|----------|--------------|
| 1 | 跨层修改任务（涉及≥2个分层） | 架构影响分析+接口变更清单 |
| 2 | 同一文件7天内修改≥3次 | 重构建议+复杂度降低方案 |
| 3 | 测试覆盖率下降>5% | 测试补充策略+风险分析 |
| 4 | 编译警告新增>10条 | 清理方案+规范强化建议 |
| 5 | CTest失败持续>2次 | 根因假设+调试策略+修复方案 |
| 6 | 新需求涉及硬件协议 | 协议选型分析+兼容性评估 |
| 7 | 性能指标退化>10% | 瓶颈假设+Profiling方案+优化思路 |

---

## 附录B：推荐新增角色

基于`team-config-analysis.md`的建议，正式推荐新增两个角色：

| 角色ID | 角色名称 | 核心职责 | 加入阶段 |
|--------|----------|----------|----------|
| `builder` | 构建工程师 | CMake优化、CI配置、构建脚本、qmltc集成 | Phase 1 |
| `tester` | 测试工程师 | 测试策略、Mock设计、覆盖率分析、回归测试 | Phase 3 |

---

*报告结束。本报告由头脑风暴角色基于对项目代码、文档和架构的全面分析生成，供Leader和全团队决策参考。*
