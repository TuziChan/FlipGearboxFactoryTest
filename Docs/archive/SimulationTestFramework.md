# Simulation Test Framework

## Overview

The Simulation Test Framework provides a complete testing environment for the gearbox test system without requiring physical hardware. It enables fast, deterministic, and reliable tests through:

- **Deterministic time control**: Advance simulation by exact tick counts or milliseconds
- **Event triggering**: Simulate magnet detection, angle arrival, and other events
- **State inspection**: Verify motor speed, encoder angle, brake load, etc.
- **Scenario helpers**: Quick setup for common test scenarios
- **Full integration**: Works seamlessly with GearboxTestEngine

## Architecture

```
SimulationTestRuntime
├── SimulationContext (shared state)
├── SimulationTestHelper (control & verification)
├── Simulated Devices
│   ├── SimulatedMotorDevice
│   ├── SimulatedTorqueDevice
│   ├── SimulatedEncoderDevice
│   └── SimulatedBrakeDevice
└── GearboxTestEngine
```

## Quick Start

### Basic Test Setup

```cpp
#include "src/infrastructure/simulation/SimulationTestRuntime.h"

class MyTest : public QObject {
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

    void testMotorControl() {
        // Start motor
        m_helper->setMotorForward(50.0);
        
        // Advance time
        m_helper->advanceMs(200);
        
        // Verify speed
        QVERIFY(m_helper->motorSpeedRpm() > 0.0);
    }
};
```

## Core APIs

### Time Control

```cpp
// Advance by ticks (1 tick = 10ms)
m_helper->advanceTicks(10);

// Advance by milliseconds
m_helper->advanceMs(100);

// Get current tick
uint64_t tick = m_helper->currentTick();
```

### Motor Control

```cpp
// Start motor forward
m_helper->setMotorForward(50.0); // 50% duty cycle

// Start motor reverse
m_helper->setMotorReverse(30.0);

// Stop motor
m_helper->stopMotor();

// Check motor state
bool running = m_helper->isMotorRunning();
bool forward = m_helper->isMotorForward();
double speed = m_helper->motorSpeedRpm();
double duty = m_helper->motorDutyCycle();
```

### Encoder Control

```cpp
// Get current angle
double angle = m_helper->encoderAngle();

// Set encoder zero
m_helper->setEncoderZero(angle);

// Wait for specific angle
bool reached = m_helper->waitForEncoderAngle(
    90.0,    // target angle
    5.0,     // tolerance
    2000     // timeout ms
);
```

### Brake Control

```cpp
// Set brake current
m_helper->setBrakeCurrent(2.5);

// Enable/disable brake
m_helper->setBrakeEnabled(true);

// Check brake state
bool enabled = m_helper->isBrakeEnabled();
double current = m_helper->brakeCurrent();
```

### Event Triggers

```cpp
// Trigger magnet detection manually
m_helper->triggerMagnetDetection();

// Setup automatic magnet detection
m_helper->setMagnetDetectionCallback([](double angle) {
    return angle >= 44.0 && angle <= 46.0; // Magnet at 45°
});

// Listen for magnet detection
QSignalSpy spy(m_helper, &SimulationTestHelper::magnetDetected);
```

### Scenario Helpers

```cpp
// Setup idle run scenario
m_helper->setupIdleRunScenario(
    50.0,   // duty cycle %
    750.0   // target speed RPM
);

// Setup load test scenario
m_helper->setupLoadTestScenario(
    50.0,   // motor duty cycle %
    2.5     // brake current A
);

// Setup homing scenario
m_helper->setupHomingScenario(45.0); // magnet at 45°
```

### Advanced Control

```cpp
// Run until condition met or timeout
bool success = m_helper->runUntil(
    []() { return m_helper->motorSpeedRpm() > 500.0; },
    2000,  // timeout ms
    10     // check interval ms
);

// Reset simulation
m_runtime->reset();
```

## Integration with Test Engine

```cpp
void testFullTestSequence() {
    // Setup recipe
    TestRecipe recipe;
    recipe.name = "SimTest";
    recipe.homeDutyCycle = 20.0;
    recipe.homeTimeoutMs = 1000;
    
    m_runtime->testEngine()->setRecipe(recipe);
    
    // Start test
    QVERIFY(m_runtime->startTest("SN-001"));
    
    // Simulate magnet detection for homing
    m_helper->setMagnetDetectionCallback([](double angle) {
        return angle < 5.0; // Magnet near zero
    });
    
    // Run simulation
    m_helper->advanceMs(500);
    
    // Check test state
    auto state = m_runtime->currentState();
    QCOMPARE(state.phase, TestPhase::PrepareAndHome);
}
```

## Testing Patterns

### Pattern 1: State Verification

```cpp
void testMotorAcceleration() {
    m_helper->setMotorForward(100.0);
    
    QVector<double> speeds;
    for (int i = 0; i < 10; i++) {
        m_helper->advanceMs(50);
        speeds.append(m_helper->motorSpeedRpm());
    }
    
    // Verify monotonic increase
    for (int i = 1; i < speeds.size(); i++) {
        QVERIFY(speeds[i] >= speeds[i-1]);
    }
}
```

### Pattern 2: Event-Driven Testing

```cpp
void testMagnetDetection() {
    QSignalSpy magnetSpy(m_helper, &SimulationTestHelper::magnetDetected);
    
    m_helper->setMagnetDetectionCallback([](double angle) {
        return angle >= 44.0 && angle <= 46.0;
    });
    
    m_helper->setMotorForward(50.0);
    m_helper->advanceMs(1000);
    
    QVERIFY(magnetSpy.count() > 0);
}
```

### Pattern 3: Timeout Testing

```cpp
void testHomingTimeout() {
    TestRecipe recipe;
    recipe.homeTimeoutMs = 100; // Very short timeout
    m_runtime->testEngine()->setRecipe(recipe);
    
    // Don't trigger magnet detection
    m_runtime->startTest("SN-TIMEOUT");
    
    // Run past timeout
    m_helper->advanceMs(200);
    
    auto state = m_runtime->currentState();
    QCOMPARE(state.phase, TestPhase::Failed);
}
```

### Pattern 4: Load Testing

```cpp
void testBrakeLoadEffect() {
    // Measure speed without brake
    m_helper->setMotorForward(50.0);
    m_helper->advanceMs(300);
    double speedNoBrake = m_helper->motorSpeedRpm();
    
    // Apply brake and measure again
    m_helper->setBrakeCurrent(2.0);
    m_helper->setBrakeEnabled(true);
    m_helper->advanceMs(300);
    double speedWithBrake = m_helper->motorSpeedRpm();
    
    // Verify brake reduces speed
    QVERIFY(speedWithBrake < speedNoBrake);
}
```

## Physics Simulation

The simulation includes realistic physics:

- **Motor acceleration**: ~500 RPM/s (5 RPM per 10ms tick)
- **Brake load effect**: Each amp reduces speed by ~100 RPM
- **Friction deceleration**: 2 RPM per tick when stopped
- **Speed range**: 0-1500 RPM at 100% duty cycle
- **Encoder tracking**: Angle updated based on motor speed and direction

## Performance

Simulation runs much faster than real-time:

```cpp
void testPerformance() {
    QElapsedTimer timer;
    timer.start();
    
    m_helper->setMotorForward(50.0);
    m_helper->advanceTicks(1000); // Simulate 10 seconds
    
    qint64 elapsed = timer.elapsed();
    qDebug() << "Simulated 10s in" << elapsed << "ms";
    // Typical: 10-50ms (200-1000x speedup)
}
```

## Best Practices

1. **Always initialize runtime**: Call `m_runtime->initialize()` in `init()`
2. **Reset between tests**: Call `m_runtime->reset()` in `cleanup()`
3. **Use deterministic time**: Prefer `advanceTicks()` over `QTest::qWait()`
4. **Setup scenarios**: Use scenario helpers for common test setups
5. **Verify state**: Check motor/encoder/brake state after operations
6. **Test timeouts**: Verify timeout handling with short timeout values
7. **Use signals**: Connect to `magnetDetected` and `angleReached` for events

## Troubleshooting

### Motor not accelerating
- Check duty cycle is > 0
- Verify motor direction is not Stopped
- Advance time to allow acceleration

### Encoder angle not changing
- Verify motor is running
- Advance sufficient time for rotation
- Check motor speed is > 0

### Magnet not detected
- Verify callback is set correctly
- Check angle range in callback
- Ensure motor is moving through magnet position

### Test timing issues
- Use `advanceTicks()` instead of `QTest::qWait()`
- Increase timeout values if needed
- Check simulation tick rate (1 tick = 10ms)

## Examples

See `tests/simulation/SimulationFrameworkExampleTests.cpp` for comprehensive examples covering:
- Basic framework usage
- Motor control
- Encoder tracking
- Brake control
- Scenario setup
- Integration with test engine
- Signal handling
- Performance testing

## API Reference

### SimulationTestRuntime

Main entry point for simulation testing.

**Methods:**
- `bool initialize()` - Initialize all devices
- `void reset()` - Reset to initial state
- `SimulationTestHelper* helper()` - Get test helper
- `GearboxTestEngine* testEngine()` - Get test engine
- `SimulatedMotorDevice* motor()` - Get motor device
- `SimulatedTorqueDevice* torque()` - Get torque device
- `SimulatedEncoderDevice* encoder()` - Get encoder device
- `SimulatedBrakeDevice* brake()` - Get brake device

### SimulationTestHelper

Control and verification interface.

**Time Control:**
- `void advanceTicks(int ticks)`
- `void advanceMs(int ms)`
- `uint64_t currentTick()`

**Motor Control:**
- `void setMotorForward(double dutyCyclePercent)`
- `void setMotorReverse(double dutyCyclePercent)`
- `void stopMotor()`
- `double motorSpeedRpm()`
- `bool isMotorRunning()`

**Encoder Control:**
- `double encoderAngle()`
- `void setEncoderZero(double offsetDeg)`
- `bool waitForEncoderAngle(double targetDeg, double toleranceDeg, int timeoutMs)`

**Brake Control:**
- `void setBrakeCurrent(double currentA)`
- `void setBrakeEnabled(bool enabled)`
- `double brakeCurrent()`
- `bool isBrakeEnabled()`

**Scenarios:**
- `void setupIdleRunScenario(double dutyCyclePercent, double targetSpeedRpm)`
- `void setupLoadTestScenario(double motorDutyCycle, double brakeCurrentA)`
- `void setupHomingScenario(double magnetAngleDeg)`

**Advanced:**
- `bool runUntil(std::function<bool()> condition, int timeoutMs, int tickIntervalMs = 10)`
- `void setMagnetDetectionCallback(std::function<bool(double)> callback)`

**Signals:**
- `void magnetDetected(double angleDeg)`
- `void angleReached(double angleDeg)`
