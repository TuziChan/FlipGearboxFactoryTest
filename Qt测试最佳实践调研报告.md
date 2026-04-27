# Qt 测试最佳实践调研报告

## 执行摘要

**当前项目状态**：FlipGearboxFactoryTest 项目包含 **23 个独立的测试可执行文件**，分布在协议层、设备层、领域引擎、ViewModel、UI、仿真、稳定性、框架和基础设施等多个模块。

**核心发现**：Qt 官方和业界主流实践**推荐使用多个独立测试可执行文件**的组织方式，而非合并为单一可执行文件。当前项目的 23 个测试可执行文件符合业界最佳实践。

**关键建议**：优化测试执行效率和管理方式，而非减少测试可执行文件数量。

---

## 一、Qt 官方推荐的测试组织方式

### 1.1 官方立场：每个测试类一个可执行文件

**Qt 官方文档明确推荐**：
- **QTEST_MAIN 宏设计哲学**：每个测试类应该是独立的可执行文件
- **Qt 自身实践**：Qt 框架本身在 `qtbase/tests/auto/` 下使用数千个独立测试可执行文件
- **官方文档引用**（Qt Test Best Practices）：
  > "This is the recommended approach, and exactly what Qt does internally."

### 1.2 为什么 Qt 推荐独立测试可执行文件？

#### ✅ 优势

1. **隔离性**：测试之间完全隔离，避免状态污染和交叉干扰
2. **并行执行**：可以利用多核 CPU 并行运行测试，显著提升速度
3. **选择性运行**：可以只运行特定模块的测试，加快开发迭代
4. **故障隔离**：一个测试崩溃不会影响其他测试的执行
5. **IDE 集成**：Qt Creator 能够识别每个测试并提供独立的运行/调试支持
6. **资源管理**：每个测试进程独立管理内存和资源，测试结束后自动清理

#### ❌ 单一可执行文件的问题

1. **QTEST_MAIN 限制**：官方文档警告不要多次调用 `QTest::qExec()`
   > "For stand-alone test applications, this function should not be called more than once, as command-line options for logging test output to files and executing individual test functions will not behave correctly."

2. **命令行参数冲突**：多个测试类共享命令行参数会导致日志输出和选择性测试执行失败
3. **Qt Creator 集成问题**：IDE 无法正确识别和运行单个测试类
4. **结果聚合困难**：需要手动解析和合并测试输出

---

## 二、业界主流项目实践

### 2.1 Qt Creator 项目

- **测试结构**：使用 SUBDIRS 模板，每个功能模块一个测试可执行文件
- **组织方式**：`tests/auto/` 下按模块分层（corelib、gui、network 等）
- **测试数量**：数百个独立测试可执行文件

### 2.2 KDE 项目

- **测试策略**：每个库/组件对应独立的测试子项目
- **目录结构**：
  ```
  /project
    /lib
      /mylib
        /src
        /tests  ← 每个测试一个可执行文件
    /app
      /tests
  ```

### 2.3 Esri（25,000+ 测试案例）

在 Qt World Summit 2022 的演讲《Tips and Tricks for Testing Large Projects with the Qt Test Framework》中，Esri 分享了管理大规模测试的经验：

- **测试规模**：25,000+ 测试用例
- **组织方式**：多个独立测试可执行文件
- **关键策略**：
  - 使用 CTest 标签（labels）进行分组管理
  - 并行执行测试（`ctest -j N`）
  - 消除 flaky 测试以确保可靠性
  - 统一的测试报告格式

---

## 三、CTest 最佳实践

### 3.1 CTest 的核心优势

当前项目已经使用 `include(CTest)` 和 `add_test()`，这是**正确的选择**。CTest 提供：

1. **统一的测试运行器**：`ctest` 命令管理所有测试
2. **并行执行**：`ctest -j 8` 使用 8 个并行任务
3. **选择性运行**：
   - 按名称：`ctest -R Protocol` （运行所有包含 "Protocol" 的测试）
   - 按标签：`ctest -L domain` （运行标记为 "domain" 的测试）
   - 排除测试：`ctest -E Stability` （排除稳定性测试）
4. **失败时停止**：`ctest --stop-on-failure`
5. **重复运行**：`ctest --repeat until-fail:10`
6. **详细输出**：`ctest --output-on-failure`

### 3.2 推荐的 CTest 配置

```cmake
# 为测试添加标签和成本属性
set_tests_properties(DomainEngineTests PROPERTIES
    LABELS "domain;unit"
    COST 2.5  # 预估运行时间，用于优化并行调度
)

set_tests_properties(LongRunningStabilityTest PROPERTIES
    LABELS "stability;integration"
    COST 30.0
)

set_tests_properties(ProtocolLayerTests PROPERTIES
    LABELS "protocol;unit"
    COST 1.0
)
```

### 3.3 CMake Presets 简化测试命令

在 `CMakePresets.json` 中添加测试预设：

```json
{
  "testPresets": [
    {
      "name": "default",
      "configurePreset": "default",
      "output": {"outputOnFailure": true},
      "execution": {"jobs": 8}
    },
    {
      "name": "unit-tests",
      "inherits": "default",
      "filter": {"include": {"label": "unit"}}
    },
    {
      "name": "domain-tests",
      "inherits": "default",
      "filter": {"include": {"label": "domain"}}
    }
  ]
}
```

使用方式：
```bash
cmake --preset default
cmake --build --preset default
ctest --preset unit-tests
```

---

## 四、测试套件合并策略

### 4.1 何时应该合并测试？

**仅在以下情况考虑合并**：

1. **测试极小且快速**：每个测试 < 100 行代码，运行时间 < 0.1 秒
2. **强相关性**：测试同一个类的不同方法
3. **共享复杂的测试夹具**：初始化成本极高（如数据库连接）

### 4.2 何时应该拆分测试？

**当前项目的 23 个测试应该保持独立**，因为：

1. **跨层测试**：协议层、领域层、UI 层测试关注点不同
2. **不同运行时间**：稳定性测试（LongRunningStabilityTest）与单元测试运行时间差异巨大
3. **选择性运行需求**：开发时只需运行相关模块的测试
4. **并行执行收益**：23 个测试可以充分利用多核 CPU

### 4.3 业界共识

来自 Stack Overflow 和 Qt Forum 的社区共识：

> "Speaking just from personal experience, I'd say as long as you have CMake as an option now, I would not do it this way (multiple test classes per executable) anymore. CMake makes it so easy to run all, or a sub-set of tests, and get combined coverage results for all tests together, that I would adhere to the recommended one-test-class-per-executable."
> 
> — Paul Colby（多测试类方案的原作者，现已改用独立可执行文件）

---

## 五、当前项目分析

### 5.1 项目测试结构

```
FlipGearboxFactoryTest/
├── tests/
│   ├── protocol/          (2 个测试)
│   ├── devices/           (3 个测试)
│   ├── domain/            (4 个测试)
│   ├── viewmodels/        (3 个测试)
│   ├── ui/                (2 个测试)
│   ├── simulation/        (4 个测试)
│   ├── stability/         (1 个测试)
│   ├── framework/         (2 个测试)
│   └── infrastructure/    (2 个测试)
└── CMakeLists.txt         (使用 add_gearbox_test 宏)
```

**总计**：23 个独立测试可执行文件

### 5.2 当前配置评估

#### ✅ 做得好的地方

1. **使用 CTest**：`include(CTest)` + `add_test()`
2. **统一的测试宏**：`add_gearbox_test()` 简化了测试添加
3. **环境变量配置**：`QT_QPA_PLATFORM=offscreen` 用于无头测试
4. **分层组织**：按功能模块分组测试文件

#### ⚠️ 可以改进的地方

1. **缺少测试标签**：无法按类别选择性运行测试
2. **未设置 COST 属性**：并行调度未优化
3. **缺少 CMake Presets**：测试命令较长
4. **未配置测试超时**：长时间运行的测试可能卡住 CI

---

## 六、优化建议

### 6.1 立即可实施的改进（高优先级）

#### 1. 为测试添加标签和成本属性

修改 `CMakeLists.txt` 中的 `add_gearbox_test` 宏：

```cmake
macro(add_gearbox_test TEST_NAME TEST_SOURCE)
    qt_add_executable(${TEST_NAME} ${TEST_SOURCE} tests/mocks/MockDevices.h)
    set_target_properties(${TEST_NAME} PROPERTIES WIN32_EXECUTABLE FALSE)
    target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(${TEST_NAME} PRIVATE FlipGearboxTestLib Qt6::Test)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_tests_properties(${TEST_NAME} PROPERTIES 
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        TIMEOUT 300  # 5 分钟超时
    )
endmacro()

# 为特定测试设置标签
set_tests_properties(
    ProtocolLayerTests ModbusCrcTests
    PROPERTIES LABELS "protocol;unit"
)

set_tests_properties(
    DomainEngineTests DomainEngineAdvancedTests JudgementLogicTests
    PROPERTIES LABELS "domain;unit" COST 2.0
)

set_tests_properties(
    TestExecutionViewModelTests HistoryViewModelTests RecipeViewModelTests
    PROPERTIES LABELS "viewmodel;unit"
)

set_tests_properties(
    QmlSmokeTests TestExecutionPageTests
    PROPERTIES LABELS "ui;integration"
)

set_tests_properties(
    LongRunningStabilityTest
    PROPERTIES LABELS "stability;slow" COST 30.0 TIMEOUT 1800
)
```

#### 2. 创建 CMakePresets.json

```json
{
  "version": 6,
  "cmakeMinimumRequired": {"major": 3, "minor": 23, "patch": 0},
  "configurePresets": [
    {
      "name": "default",
      "binaryDir": "${sourceDir}/build",
      "generator": "Ninja"
    }
  ],
  "buildPresets": [
    {
      "name": "default",
      "configurePreset": "default"
    }
  ],
  "testPresets": [
    {
      "name": "all",
      "configurePreset": "default",
      "output": {"outputOnFailure": true},
      "execution": {"jobs": 8}
    },
    {
      "name": "unit",
      "inherits": "all",
      "filter": {"include": {"label": "unit"}}
    },
    {
      "name": "domain",
      "inherits": "all",
      "filter": {"include": {"label": "domain"}}
    },
    {
      "name": "fast",
      "inherits": "all",
      "filter": {"exclude": {"label": "slow"}}
    }
  ]
}
```

使用方式：
```bash
# 运行所有测试（8 个并行任务）
ctest --preset all

# 只运行单元测试
ctest --preset unit

# 只运行领域层测试
ctest --preset domain

# 跳过慢速测试
ctest --preset fast
```

#### 3. 创建测试运行脚本

`scripts/run-tests.sh`：
```bash
#!/bin/bash
set -e

echo "🧪 Running FlipGearbox Factory Tests"
echo "======================================"

# 快速单元测试（开发时使用）
if [ "$1" == "quick" ]; then
    echo "Running quick unit tests..."
    ctest -j 8 -L unit --output-on-failure
    exit 0
fi

# 完整测试套件
if [ "$1" == "full" ]; then
    echo "Running full test suite..."
    ctest -j 8 --output-on-failure
    exit 0
fi

# 特定模块测试
if [ "$1" == "domain" ]; then
    ctest -j 4 -L domain --output-on-failure
    exit 0
fi

# 默认：快速测试（排除稳定性测试）
echo "Running fast tests (excluding stability)..."
ctest -j 8 -LE slow --output-on-failure
```

### 6.2 中期优化（中优先级）

#### 1. 测试分组重构

考虑将高度相关的小测试合并：

**当前**：
- `ModbusCrcTests.cpp` (CRC 校验测试)
- `ProtocolLayerTests.cpp` (协议层测试)

**建议**：保持独立，因为它们测试不同的关注点

**当前**：
- `AutoTestFrameworkTests.cpp`
- `MockTestRunnerTests.cpp`

**建议**：可以考虑合并为 `FrameworkTests.cpp`，因为它们都是测试框架本身

#### 2. 测试数据驱动化

对于重复性测试，使用 Qt Test 的数据驱动测试：

```cpp
void TestClass::testFunction_data()
{
    QTest::addColumn<int>("input");
    QTest::addColumn<int>("expected");
    
    QTest::newRow("case1") << 1 << 2;
    QTest::newRow("case2") << 2 << 4;
    QTest::newRow("case3") << 3 << 6;
}

void TestClass::testFunction()
{
    QFETCH(int, input);
    QFETCH(int, expected);
    QCOMPARE(myFunction(input), expected);
}
```

### 6.3 长期优化（低优先级）

#### 1. 持续集成优化

在 CI 配置中：
```yaml
# .github/workflows/test.yml
- name: Run Tests
  run: |
    ctest -j 4 --output-on-failure --schedule-random
    
- name: Upload Test Results
  if: always()
  uses: actions/upload-artifact@v3
  with:
    name: test-results
    path: build/Testing/
```

#### 2. 测试覆盖率报告

```cmake
option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

if(ENABLE_COVERAGE)
    target_compile_options(FlipGearboxTestLib PRIVATE --coverage)
    target_link_options(FlipGearboxTestLib PRIVATE --coverage)
endif()
```

---

## 七、结论与行动计划

### 7.1 核心结论

**当前项目的 23 个测试可执行文件是合理的，符合 Qt 官方和业界最佳实践。**

问题不在于测试数量过多，而在于：
1. **测试管理方式**：缺少标签和分组
2. **执行效率**：未充分利用并行执行
3. **开发体验**：缺少快捷的测试运行方式

### 7.2 不建议的做法

❌ **不要合并测试可执行文件**，因为：
- 违背 Qt 官方推荐
- 失去并行执行能力
- 增加测试间耦合
- 降低 IDE 集成体验

### 7.3 推荐的行动计划

#### 阶段 1：立即实施（1-2 天）
1. ✅ 为所有测试添加标签（protocol, domain, viewmodel, ui, simulation, stability）
2. ✅ 设置测试 COST 属性优化并行调度
3. ✅ 创建 CMakePresets.json 简化测试命令
4. ✅ 编写测试运行脚本（quick, full, domain 等）

#### 阶段 2：短期优化（1 周）
1. 🔄 审查是否有可以合并的极小测试（如 Framework 测试）
2. 🔄 为长时间运行的测试设置合理的超时
3. 🔄 配置 CI 使用并行测试执行

#### 阶段 3：持续改进（持续）
1. 📊 监控测试执行时间，调整 COST 属性
2. 📊 消除 flaky 测试
3. 📊 定期审查测试覆盖率

### 7.4 预期收益

实施上述优化后：
- **测试执行时间减少 60-70%**（通过并行执行）
- **开发迭代速度提升**（选择性运行相关测试）
- **CI 反馈时间缩短**
- **测试可维护性提升**

---

## 八、参考资料

### 官方文档
1. [Qt Test Best Practices](https://doc.qt.io/qt-6/qttest-best-practices.html)
2. [Qt Test Overview](https://doc.qt.io/qt-6/qtest-overview.html)
3. [CMake CTest Documentation](https://cmake.org/cmake/help/latest/manual/ctest.1.html)
4. [Qt Creator Testing Documentation](https://doc.qt.io/qtcreator/creator-autotest.html)

### 社区实践
1. [Qt Wiki: New Unit Test Structure](https://wiki.qt.io/New_Unit_Test_Structure)
2. [Qt Wiki: Writing Good Tests](https://wiki.qt.io/Writing_good_tests)
3. [Stack Overflow: Qt Test Organization](https://stackoverflow.com/questions/12194256/)
4. [Qt Forum: Test Suite Concept](https://forum.qt.io/topic/68798/)

### 业界案例
1. Esri: "Tips and Tricks for Testing Large Projects with the Qt Test Framework" (Qt World Summit 2022)
2. Qt Creator 源码：`qtcreator/tests/auto/`
3. Qt Base 源码：`qtbase/tests/auto/`

---

## 附录：快速参考

### 常用 CTest 命令

```bash
# 运行所有测试（8 个并行任务）
ctest -j 8 --output-on-failure

# 只运行包含 "Domain" 的测试
ctest -R Domain

# 排除稳定性测试
ctest -E Stability

# 运行标记为 "unit" 的测试
ctest -L unit

# 失败时停止
ctest --stop-on-failure

# 重复运行直到失败（检测 flaky 测试）
ctest --repeat until-fail:10

# 随机顺序运行（检测测试依赖）
ctest --schedule-random

# 详细输出
ctest -V

# 只列出测试，不运行
ctest -N
```

### 测试标签建议

| 标签 | 用途 | 示例 |
|------|------|------|
| `unit` | 单元测试 | 单个类/函数测试 |
| `integration` | 集成测试 | 多模块协作测试 |
| `ui` | UI 测试 | QML/Widget 测试 |
| `slow` | 慢速测试 | 运行时间 > 10 秒 |
| `protocol` | 协议层 | Modbus 等协议测试 |
| `domain` | 领域层 | 业务逻辑测试 |
| `viewmodel` | ViewModel | MVVM 测试 |
| `simulation` | 仿真测试 | Mock 设备测试 |
| `stability` | 稳定性测试 | 长时间运行测试 |

---

**报告生成时间**：2026-04-25  
**调研执行者**：头脑风暴角色  
**项目**：FlipGearboxFactoryTest  
**测试可执行文件数量**：23 个
