# Simulation Test Framework Integration Guide

## New Files Added

### Source Files (src/infrastructure/simulation/)
1. `SimulationTestHelper.h` - Test helper API for controlling simulation
2. `SimulationTestHelper.cpp` - Implementation of test helper
3. `SimulationTestRuntime.h` - Complete test runtime environment
4. `SimulationTestRuntime.cpp` - Implementation of test runtime

### Test Files (tests/simulation/)
1. `SimulationFrameworkExampleTests.cpp` - Comprehensive example tests

### Documentation (docs/)
1. `SimulationTestFramework.md` - Complete usage documentation

## Build System Integration

### For CMake Projects

Add to your CMakeLists.txt:

```cmake
# Simulation test framework sources
set(SIMULATION_TEST_FRAMEWORK_SOURCES
    src/infrastructure/simulation/SimulationTestHelper.cpp
    src/infrastructure/simulation/SimulationTestRuntime.cpp
)

set(SIMULATION_TEST_FRAMEWORK_HEADERS
    src/infrastructure/simulation/SimulationTestHelper.h
    src/infrastructure/simulation/SimulationTestRuntime.h
)

# Add to main executable or library
target_sources(YourTarget PRIVATE
    ${SIMULATION_TEST_FRAMEWORK_SOURCES}
    ${SIMULATION_TEST_FRAMEWORK_HEADERS}
)

# Add example test executable
add_executable(SimulationFrameworkExampleTests
    tests/simulation/SimulationFrameworkExampleTests.cpp
    ${SIMULATION_TEST_FRAMEWORK_SOURCES}
    # Add other required sources
)

target_link_libraries(SimulationFrameworkExampleTests
    Qt6::Core
    Qt6::Test
    # Add other required libraries
)

add_test(NAME SimulationFrameworkExampleTests 
         COMMAND SimulationFrameworkExampleTests)
```

### For qmake Projects

Add to your .pro file:

```qmake
# Simulation test framework
HEADERS += \
    src/infrastructure/simulation/SimulationTestHelper.h \
    src/infrastructure/simulation/SimulationTestRuntime.h

SOURCES += \
    src/infrastructure/simulation/SimulationTestHelper.cpp \
    src/infrastructure/simulation/SimulationTestRuntime.cpp

# Test configuration
CONFIG += testcase
QT += testlib

# Example test
SOURCES += tests/simulation/SimulationFrameworkExampleTests.cpp
```

### For Qt Creator

1. Right-click on project in Project Explorer
2. Select "Add Existing Files..."
3. Add all new .h and .cpp files
4. Ensure files are in correct folders:
   - Source files in `src/infrastructure/simulation/`
   - Test files in `tests/simulation/`

## Dependencies

The simulation test framework requires:

- Qt6 Core
- Qt6 Test (for test executables)
- Existing simulation infrastructure:
  - `SimulationContext.h`
  - `SimulatedMotorDevice.h/cpp`
  - `SimulatedTorqueDevice.h/cpp`
  - `SimulatedEncoderDevice.h/cpp`
  - `SimulatedBrakeDevice.h/cpp`
- Domain layer:
  - `GearboxTestEngine.h/cpp`
  - `TestRecipe.h`
  - `TestRunState.h`
  - `TestResults.h`

## Compilation Verification

To verify the framework compiles correctly:

```bash
# CMake
mkdir build && cd build
cmake ..
cmake --build . --target SimulationFrameworkExampleTests

# qmake
qmake
make

# Run tests
./SimulationFrameworkExampleTests
```

## Usage in Your Tests

### Include Headers

```cpp
#include "src/infrastructure/simulation/SimulationTestRuntime.h"
#include "src/infrastructure/simulation/SimulationTestHelper.h"
```

### Basic Test Structure

```cpp
class MySimulationTest : public QObject {
    Q_OBJECT
private:
    std::unique_ptr<SimulationTestRuntime> m_runtime;
    SimulationTestHelper* m_helper;

private slots:
    void init() {
        m_runtime = std::make_unique<SimulationTestRuntime>();
        QVERIFY(m_runtime->initialize());
        m_helper = m_runtime->helper();
    }

    void cleanup() {
        m_runtime->reset();
    }

    void testMyFeature() {
        // Your test code here
    }
};

QTEST_MAIN(MySimulationTest)
#include "MySimulationTest.moc"
```

## Integration Checklist

- [ ] Add source files to build system
- [ ] Add test files to test configuration
- [ ] Verify all dependencies are available
- [ ] Compile and run example tests
- [ ] Review documentation in `docs/SimulationTestFramework.md`
- [ ] Write your first simulation test
- [ ] Verify tests run in CI/CD pipeline

## Troubleshooting

### Compilation Errors

**Error: Cannot find SimulationContext.h**
- Ensure `src/infrastructure/simulation/SimulationContext.h` exists
- Check include paths in build configuration

**Error: Undefined reference to SimulatedMotorDevice**
- Add all simulated device source files to build
- Check linking configuration

**Error: Cannot find GearboxTestEngine**
- Add domain layer sources to build
- Verify domain headers are accessible

### Runtime Errors

**Test crashes on initialization**
- Verify Qt Test module is linked
- Check QObject parent relationships
- Ensure all devices are properly initialized

**Simulation doesn't advance**
- Call `m_helper->advanceTicks()` or `advanceMs()`
- Verify simulation context is shared correctly

## Next Steps

1. Review `docs/SimulationTestFramework.md` for complete API documentation
2. Run `SimulationFrameworkExampleTests` to see framework in action
3. Write simulation tests for your features
4. Replace `QTest::qWait()` with deterministic `advanceMs()` in existing tests
5. Add scenario helpers for your specific test cases

## Support

For questions or issues:
1. Check documentation in `docs/SimulationTestFramework.md`
2. Review examples in `tests/simulation/SimulationFrameworkExampleTests.cpp`
3. Examine existing simulation device implementations
4. Contact the simulation runtime development team
