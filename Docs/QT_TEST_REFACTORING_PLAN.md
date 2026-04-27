# Qt 测试框架重构方案

## 📊 现状诊断

### 当前问题
- **23 个独立测试可执行文件**，每个 3-16MB
- **26 个独立测试入口**（QTEST_MAIN）
- **11,479 行测试代码**分散在 33 个文件中
- **构建产物占用 ~70MB**（23 个 exe + 15MB 共享库）
- **编译时间长**：每个测试都链接完整的 FlipGearboxTestLib（16MB 静态库）
- **维护成本高**：修改业务代码后需重新编译所有测试

### 已有改进
✅ 已创建 `FlipGearboxTestLib` 静态库（避免重复编译源码）  
✅ 已按层次组织测试文件（protocol/devices/domain/viewmodels/ui/simulation/infrastructure）

---

## 🎯 业界推荐的 Qt 测试最佳实践

### 1. 测试组织模式

#### ❌ 反模式：每个测试类一个可执行文件
```cmake
add_gearbox_test(TestA tests/TestA.cpp)  # 生成 TestA.exe
add_gearbox_test(TestB tests/TestB.cpp)  # 生成 TestB.exe
# 问题：链接开销大、构建慢、难以管理
```

#### ✅ 推荐模式：分层测试套件（Test Suite per Layer）
```cmake
# 一个层次一个测试套件
add_test_suite(ProtocolTests 
    tests/protocol/ModbusCrcTests.cpp
    tests/protocol/FrameParsingTests.cpp
)  # 生成 ProtocolTests.exe

add_test_suite(DomainTests
    tests/domain/JudgementLogicTests.cpp
    tests/domain/EngineTests.cpp
)  # 生成 DomainTests.exe
```

**优势**：
- 减少链接次数（7 个套件 vs 23 个独立测试）
- 更快的增量构建（修改单层代码只重新链接该层测试）
- 更清晰的测试报告结构
- 符合 Qt 官方文档推荐（[Qt Test Tutorial](https://doc.qt.io/qt-6/qtest-tutorial.html)）

---

### 2. 共享库拆分策略

#### 当前问题
`FlipGearboxTestLib` 包含所有业务代码（domain + infrastructure + viewmodels + ui），导致：
- 修改任何层的代码 → 所有测试都需重新链接
- 测试可执行文件体积大（每个都链接完整的 16MB 库）

#### 推荐方案：按依赖层次拆分库

```
┌─────────────────────────────────────┐
│  FlipGearboxUI (QML + ViewModels)  │  ← UI 层测试链接
├─────────────────────────────────────┤
│  FlipGearboxDomain (Engine + Logic)│  ← 领域层测试链接
├─────────────────────────────────────┤
│  FlipGearboxInfra (Devices + Bus)  │  ← 基础设施测试链接
└─────────────────────────────────────┘
```

**CMake 配置示例**：
```cmake
# 基础设施层库（最底层，无依赖）
qt_add_library(FlipGearboxInfra STATIC
    src/infrastructure/bus/*.cpp
    src/infrastructure/devices/*.cpp
    src/infrastructure/simulation/*.cpp
)

# 领域层库（依赖基础设施）
qt_add_library(FlipGearboxDomain STATIC
    src/domain/*.cpp
)
target_link_libraries(FlipGearboxDomain PUBLIC FlipGearboxInfra)

# UI 层库（依赖领域层）
qt_add_library(FlipGearboxUI STATIC
    src/viewmodels/*.cpp
    src/ui/*.cpp
)
target_link_libraries(FlipGearboxUI PUBLIC FlipGearboxDomain)
```

**测试链接优化**：
```cmake
# 协议层测试只链接基础设施库
target_link_libraries(ProtocolTests PRIVATE FlipGearboxInfra Qt6::Test)

# 领域层测试链接领域库（自动传递基础设施依赖）
target_link_libraries(DomainTests PRIVATE FlipGearboxDomain Qt6::Test)

# UI 层测试链接 UI 库（自动传递所有依赖）
target_link_libraries(UITests PRIVATE FlipGearboxUI Qt6::Test Qt6::Qml)
```

**收益**：
- 修改基础设施代码 → 只重新链接 ProtocolTests + DomainTests + UITests
- 修改领域代码 → 只重新链接 DomainTests + UITests
- 修改 UI 代码 → 只重新链接 UITests
- 测试可执行文件体积减小 50-70%

---

### 3. CMake 配置优化

#### 3.1 使用 `qt_add_test` 替代手动宏

**当前方式**：
```cmake
macro(add_gearbox_test TEST_NAME TEST_SOURCE)
    qt_add_executable(${TEST_NAME} ${TEST_SOURCE})
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endmacro()
```

**推荐方式**（Qt 6.2+）：
```cmake
function(add_test_suite SUITE_NAME)
    qt_add_executable(${SUITE_NAME} ${ARGN})
    target_link_libraries(${SUITE_NAME} PRIVATE Qt6::Test)
    
    # 自动发现测试用例（无需手动 add_test）
    qt_discover_tests(${SUITE_NAME}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endfunction()
```

**优势**：
- 自动发现每个测试类中的所有测试方法
- CTest 报告更细粒度（显示每个测试方法的结果）
- 支持并行测试执行

#### 3.2 启用并行构建和测试

```cmake
# CMakeLists.txt 顶部
set(CMAKE_UNITY_BUILD ON)  # 启用 Unity Build 加速编译
set(CMAKE_UNITY_BUILD_BATCH_SIZE 16)

# 并行测试执行
enable_testing()
include(CTest)
set(CTEST_PARALLEL_LEVEL 4)  # 同时运行 4 个测试套件
```

#### 3.3 条件编译测试（可选）

```cmake
option(BUILD_TESTING "Build test suites" ON)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
```

**用途**：生产构建时跳过测试编译，节省 CI 时间。

---

### 4. 测试套件分层方案

#### 推荐结构（7 个测试套件）

| 测试套件 | 包含测试 | 链接库 | 预计体积 |
|---------|---------|--------|---------|
| **ProtocolTests** | ModbusCrcTests, ProtocolLayerTests | FlipGearboxInfra | ~500KB |
| **DeviceTests** | BrakePowerTests, AngleTestTests, BoundaryTests | FlipGearboxInfra | ~1.5MB |
| **DomainTests** | JudgementLogicTests, EngineTests, EngineAdvancedTests, ExecutionVerification | FlipGearboxDomain | ~2MB |
| **ViewModelTests** | TestExecutionVM, HistoryVM, RecipeVM | FlipGearboxUI | ~3MB |
| **UITests** | QmlSmokeTests, TestExecutionPageTests | FlipGearboxUI + Qt6::Qml | ~4MB |
| **SimulationTests** | SimulationRuntimeTests, MockMotorTests, HarnessTests, IntegrationTests | FlipGearboxInfra | ~2MB |
| **IntegrationTests** | LongRunningStabilityTest, PerformanceMonitorTests, ReportGeneratorTests | FlipGearboxUI | ~3MB |

**总体积**：~16MB（vs 当前 70MB，减少 77%）

#### CMake 实现示例

```cmake
# tests/CMakeLists.txt

# 协议层测试套件
add_test_suite(ProtocolTests
    protocol/ModbusCrcTests.cpp
    ProtocolLayerTests.cpp
)
target_link_libraries(ProtocolTests PRIVATE FlipGearboxInfra Qt6::Test)

# 设备层测试套件
add_test_suite(DeviceTests
    devices/BrakePowerConstantVoltageTest.cpp
    AngleTestMagnetIntegrationTests.cpp
    BoundaryProtectionTests.cpp
)
target_link_libraries(DeviceTests PRIVATE FlipGearboxInfra Qt6::Test)

# 领域层测试套件
add_test_suite(DomainTests
    domain/JudgementLogicTests.cpp
    DomainEngineTests.cpp
    DomainEngineAdvancedTests.cpp
    TestExecutionVerification.cpp
)
target_link_libraries(DomainTests PRIVATE FlipGearboxDomain Qt6::Test)

# ViewModel 层测试套件
add_test_suite(ViewModelTests
    viewmodels/TestExecutionViewModelTests.cpp
    viewmodels/HistoryViewModelTests.cpp
    viewmodels/RecipeViewModelTests.cpp
)
target_link_libraries(ViewModelTests PRIVATE FlipGearboxUI Qt6::Test)

# UI 层测试套件（需要 QML 引擎）
add_test_suite(UITests
    ui/QmlSmokeTests.cpp
    ui/TestExecutionPageTests.cpp
)
target_link_libraries(UITests PRIVATE FlipGearboxUI Qt6::Test Qt6::Qml Qt6::Quick)

# 仿真测试套件
add_test_suite(SimulationTests
    simulation/SimulationRuntimeTests.cpp
    simulation/MockMotorMagnetDetectionTests.cpp
    simulation/GearboxSimulationIntegrationTests.cpp
    simulation/HardwareSimulationHarnessTests.cpp
)
target_link_libraries(SimulationTests PRIVATE FlipGearboxInfra Qt6::Test)

# 集成测试套件（长时间运行）
add_test_suite(IntegrationTests
    stability/LongRunningStabilityTest.cpp
    infrastructure/PerformanceMonitorTests.cpp
    infrastructure/TestReportGeneratorTests.cpp
    framework/AutoTestFrameworkTests.cpp
    framework/MockTestRunnerTests.cpp
)
target_link_libraries(IntegrationTests PRIVATE FlipGearboxUI Qt6::Test)
set_tests_properties(IntegrationTests PROPERTIES TIMEOUT 300)  # 5 分钟超时
```

---

### 5. Build 目录处理建议

#### 当前问题
- `build/` 目录包含 23 个测试 exe + 大量中间文件
- 难以区分主程序和测试程序

#### 推荐方案：分离输出目录

```cmake
# 主程序输出到 build/bin/
set_target_properties(appFlipGearboxFactoryTest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 测试程序输出到 build/tests/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/tests)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/tests)

# 或者使用 CTest 的默认行为（测试不生成 exe，直接运行）
```

#### .gitignore 优化

```gitignore
# 忽略整个 build 目录
/build/

# 但保留测试报告（可选）
!/build/test-reports/
```

---

## 🚀 实施路线图

### 阶段 1：拆分共享库（1-2 小时）
1. 创建 `FlipGearboxInfra` 库（基础设施层）
2. 创建 `FlipGearboxDomain` 库（领域层）
3. 创建 `FlipGearboxUI` 库（UI 层）
4. 验证主程序编译通过

### 阶段 2：重构测试套件（2-3 小时）
1. 创建 `tests/CMakeLists.txt`
2. 实现 `add_test_suite()` 函数
3. 按层次合并测试文件到 7 个套件
4. 验证所有测试通过

### 阶段 3：优化构建配置（1 小时）
1. 启用 Unity Build
2. 配置并行测试
3. 分离输出目录
4. 更新 CI/CD 脚本

### 阶段 4：清理遗留代码（30 分钟）
1. 删除旧的 `add_gearbox_test` 宏
2. 删除独立测试的 QTEST_MAIN 入口
3. 更新文档

---

## 📈 预期收益

| 指标 | 当前 | 优化后 | 改善 |
|-----|------|--------|------|
| 测试可执行文件数量 | 23 个 | 7 个 | -70% |
| 构建产物总大小 | ~70MB | ~16MB | -77% |
| 全量构建时间 | ~8 分钟 | ~3 分钟 | -62% |
| 增量构建时间（修改单层） | ~2 分钟 | ~30 秒 | -75% |
| CTest 报告粒度 | 23 个测试 | ~150 个测试方法 | +550% |
| 维护复杂度 | 高（23 个 CMake 目标） | 低（7 个套件） | -70% |

---

## 🔗 参考资料

- [Qt Test Tutorial](https://doc.qt.io/qt-6/qtest-tutorial.html)
- [CMake CTest Documentation](https://cmake.org/cmake/help/latest/manual/ctest.1.html)
- [Qt Test Best Practices (Qt Wiki)](https://wiki.qt.io/Writing_Unit_Tests)
- [Google Test FAQ - Test Organization](https://google.github.io/googletest/faq.html#should-i-use-the-constructordestructor-of-the-test-fixture-or-setupteardown)

---

## ⚠️ 风险与注意事项

### 1. 测试隔离性
**风险**：多个测试类在同一进程中运行，可能相互影响（全局状态污染）。

**缓解措施**：
- 每个测试类使用独立的 `initTestCase()` / `cleanupTestCase()`
- 避免使用全局变量和单例
- 使用 Qt Test 的 `QTEST_APPLESS_MAIN` 避免 QApplication 冲突

### 2. 长时间运行的测试
**风险**：`LongRunningStabilityTest` 可能拖慢整个套件。

**缓解措施**：
- 单独放入 `IntegrationTests` 套件
- 设置 `TIMEOUT` 属性
- CI 中使用 `ctest -L quick` 只运行快速测试

### 3. QML 测试的特殊性
**风险**：QML 测试需要 QGuiApplication，与其他测试冲突。

**缓解措施**：
- QML 测试单独放入 `UITests` 套件
- 使用 `QTEST_MAIN` 而非 `QTEST_APPLESS_MAIN`
- 设置 `QT_QPA_PLATFORM=offscreen`

---

## 📝 后续优化方向

1. **引入 Catch2/Google Test**（可选）
   - Qt Test 功能有限（无参数化测试、无 BDD 风格）
   - Catch2 提供更好的断言语法和报告
   - 可与 Qt Test 共存（不同套件使用不同框架）

2. **测试覆盖率分析**
   - 集成 gcov/lcov 或 OpenCppCoverage
   - 生成 HTML 覆盖率报告
   - 设置覆盖率门槛（如 80%）

3. **性能基准测试**
   - 使用 `QBENCHMARK` 宏
   - 跟踪关键路径性能回归
   - 集成到 CI 中自动检测

4. **Mock 框架**
   - 当前使用手写 Mock（如 `MockDevices.h`）
   - 可引入 Google Mock 或 FakeIt 简化 Mock 编写
   - 提高测试可读性和维护性

---

**文档版本**：v1.0  
**创建日期**：2026-04-25  
**作者**：FlipGearbox Dev Team - 头脑风暴角色  
**状态**：待审查
