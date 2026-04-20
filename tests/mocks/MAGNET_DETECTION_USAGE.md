# Magnet Detection Mock Usage Guide

## Overview

Enhanced MockMotorDevice and MockEncoderDevice now support realistic magnet detection simulation for angle positioning tests.

## Key Features

### 1. MockEncoderDevice - Angle Simulation
- **Auto-increment angle**: Simulates continuous rotation
- **Configurable speed**: Set degrees per tick
- **Forward/Reverse**: Bidirectional rotation support
- **Boundary handling**: Automatic 0-360° wrapping

### 2. MockMotorDevice - Magnet Detection
- **Multiple magnets**: Configure any number of magnet positions
- **State tracking**: Tracks how many times each magnet has been passed
- **Linked encoder**: Automatically reads angle from encoder
- **Detection window**: Configurable tolerance around magnet position (default 0.5°)
- **AI1 signal**: Generates falling edge (high->low) when magnet detected

## Basic Usage

### Setup

```cpp
// Create mock devices
MockMotorDevice* motor = new MockMotorDevice();
MockEncoderDevice* encoder = new MockEncoderDevice();

// Configure magnet positions (3°, 49°, 113°)
motor->setMagnetPositions({3.0, 49.0, 113.0});

// Link encoder angle to motor for automatic detection
motor->linkEncoderAngle(&encoder->mockAngleDeg);

// Enable magnet detection
motor->setMagnetDetectionEnabled(true);
```

### Manual Angle Control

```cpp
// Set encoder angle manually
encoder->setAngle(3.0);

// Read AI1 level (will be LOW when at magnet)
bool level;
motor->readAI1Level(level);
// level == false (magnet detected)

// Move away from magnet
encoder->setAngle(10.0);
motor->readAI1Level(level);
// level == true (no magnet)
```

### Automatic Angle Simulation

```cpp
// Start simulation: 0° to 120° at 0.5°/tick, forward direction
encoder->startAngleSimulation(0.0, 120.0, 0.5, true);

// In your test loop (e.g., 33ms timer)
while (encoder->enableAngleSimulation) {
    double angle;
    encoder->readAngle(angle);  // Auto-increments angle
    
    bool level;
    motor->readAI1Level(level);  // Auto-detects magnets
    
    // Your test logic here
}
```

## Advanced Features

### Pass Count Tracking

```cpp
// Check how many times each magnet has been passed
int count = motor->getMagnetPassCount(0);  // First magnet (3°)
qDebug() << "Magnet 0 passed" << count << "times";
```

### Reset Detection State

```cpp
// Reset all magnet pass counts to zero
motor->resetMagnetStates();
```

### Custom Detection Window

```cpp
// Set detection window to 1.0° (default is 0.5°)
motor->magnetDetectionWindowDeg = 1.0;
```

### Disable Detection

```cpp
// Temporarily disable magnet detection
motor->setMagnetDetectionEnabled(false);

// Re-enable later
motor->setMagnetDetectionEnabled(true);
```

## Complete Example: Angle Test Sequence

```cpp
// Setup
MockMotorDevice* motor = new MockMotorDevice();
MockEncoderDevice* encoder = new MockEncoderDevice();

motor->setMagnetPositions({3.0, 49.0, 113.0});
motor->linkEncoderAngle(&encoder->mockAngleDeg);
motor->setMagnetDetectionEnabled(true);

// Simulate angle test sequence
QVector<double> targetAngles = {3.0, 49.0, 3.0, 113.0, 0.0};
bool lastLevel = true;

for (double targetAngle : targetAngles) {
    // Simulate movement to target
    double currentAngle = encoder->mockAngleDeg;
    double step = (targetAngle > currentAngle) ? 0.5 : -0.5;
    
    while (qAbs(encoder->mockAngleDeg - targetAngle) > 0.1) {
        encoder->mockAngleDeg += step;
        
        bool level;
        motor->readAI1Level(level);
        
        // Detect falling edge
        if (lastLevel && !level) {
            qDebug() << "Magnet detected at" << encoder->mockAngleDeg << "°";
        }
        
        lastLevel = level;
    }
}

// Verify results
QCOMPARE(motor->getMagnetPassCount(0), 2);  // 3° passed twice
QCOMPARE(motor->getMagnetPassCount(1), 1);  // 49° passed once
QCOMPARE(motor->getMagnetPassCount(2), 1);  // 113° passed once
```

## Edge Cases Handled

### 1. Boundary Crossing (0°/360°)
```cpp
motor->setMagnetPositions({358.0});
encoder->setAngle(359.0);
motor->readAI1Level(level);
// Correctly detects magnet at 358° even when at 359°
```

### 2. Multiple Passes Through Same Magnet
```cpp
// First pass
encoder->setAngle(3.0);
motor->readAI1Level(level);
QCOMPARE(motor->getMagnetPassCount(0), 1);

// Move away and return
encoder->setAngle(10.0);
motor->readAI1Level(level);
encoder->setAngle(3.0);
motor->readAI1Level(level);
QCOMPARE(motor->getMagnetPassCount(0), 2);  // Incremented
```

### 3. Reverse Direction
```cpp
// Forward: 0° -> 50° (detects 3° then 49°)
// Reverse: 50° -> 0° (detects 49° then 3°)
// Both directions work correctly
```

## Integration with GearboxTestEngine

The mock devices integrate seamlessly with the test engine:

```cpp
GearboxTestEngine* engine = new GearboxTestEngine();
MockMotorDevice* motor = new MockMotorDevice();
MockEncoderDevice* encoder = new MockEncoderDevice();

// Setup magnet detection
motor->setMagnetPositions({3.0, 49.0, 113.0});
motor->linkEncoderAngle(&encoder->mockAngleDeg);
motor->setMagnetDetectionEnabled(true);

// Setup angle simulation
encoder->startAngleSimulation(0.0, 120.0, 0.5, true);

// Set devices
engine->setDevices(motor, torque, encoder, brake);

// Start test - engine will automatically detect magnets
engine->startTest("TEST-001");
```

## Debugging Tips

### Enable Debug Output
The mock automatically logs magnet detections:
```
MockMotor: Magnet 0 detected at angle 3.00° (pass #1)
MockMotor: Magnet 1 detected at angle 49.00° (pass #1)
```

### Verify Detection Logic
```cpp
// Check if magnet was detected
bool wasDetected = (motor->getMagnetPassCount(0) > 0);

// Check detection angle
// (stored internally in magnetStates[i].detectionAngle)
```

## Test Files

- `tests/MagnetDetectionMockTests.cpp` - Unit tests for mock devices
- `tests/AngleTestMagnetIntegrationTests.cpp` - Integration tests with full sequence

## Configuration Reference

### MockMotorDevice Properties
- `magnetPositionsDeg` - QVector of magnet angles (default: {3.0, 49.0, 113.0})
- `magnetDetectionWindowDeg` - Detection tolerance (default: 0.5°)
- `enableMagnetDetection` - Enable/disable detection (default: false)
- `linkedEncoderAngle` - Pointer to encoder angle for auto-detection

### MockEncoderDevice Properties
- `mockAngleDeg` - Current angle (0-360°)
- `enableAngleSimulation` - Auto-increment enabled
- `simulationSpeedDegPerTick` - Degrees per tick (default: 0.5)
- `simulationForward` - Direction (true=forward, false=reverse)
