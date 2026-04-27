# FlipGearboxFactoryTest

**Gearbox Factory Test System** - Industrial automation testing platform based on Qt6

## Overview

FlipGearboxFactoryTest is an automated quality inspection system for gearbox manufacturing, providing:

- **Multi-device coordination**: Motor drive, torque sensor, encoder, brake power supply
- **Modbus RTU protocol**: RS485 industrial communication
- **Complete test workflow**: Homing, idle, angle positioning, load testing
- **Real-time data acquisition**: High-frequency telemetry and visualization
- **Mock mode simulation**: Hardware-free development and testing

## Technology Stack

- **UI Framework**: Qt 6.11 (QML + Qt Quick)
- **Language**: C++20
- **Build System**: CMake 3.16+
- **Communication**: Modbus RTU (RS485)
- **Architecture**: MVVM (Model-View-ViewModel)

## Quick Start

### Prerequisites

- Qt 6.11+ with Qt Quick and Qt SerialPort
- CMake 3.16+
- MinGW 13.1.0+ (Windows) or GCC/Clang (Linux)
- C++20 compatible compiler

### Build

```bash
# Configure
cmake -S . -B build -G "MinGW Makefiles"

# Build
cmake --build build

# Run
.\build\appFlipGearboxFactoryTest.exe
```

### Run Tests

```bash
# Run all tests
ctest --test-dir build --output-on-failure

# Run specific test
.\build\DomainEngineTests.exe
```

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                      UI Layer (QML)                     │
│  TestExecutionPage │ RecipePage │ DiagnosticsPage      │
└────────────────────────┬────────────────────────────────┘
                         │
┌────────────────────────┴────────────────────────────────┐
│                  ViewModel Layer (C++)                  │
│  TestExecutionViewModel │ RecipeViewModel │ ...         │
└────────────────────────┬────────────────────────────────┘
                         │
┌────────────────────────┴────────────────────────────────┐
│                   Domain Layer (C++)                    │
│  GearboxTestEngine (33ms cycle, state machine)          │
│  TestRecipe │ TestResults │ TelemetrySnapshot           │
└────────────────────────┬────────────────────────────────┘
                         │
┌────────────────────────┴────────────────────────────────┐
│              Infrastructure Layer (C++)                 │
│  ┌──────────────────────────────────────────────────┐  │
│  │ Device Implementations                           │  │
│  │  AqmdMotorDriveDevice │ Dyn200TorqueSensorDevice │  │
│  │  SingleTurnEncoderDevice │ BrakePowerSupplyDevice│  │
│  └──────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────┐  │
│  │ Bus Communication                                │  │
│  │  ModbusRtuBusController │ ModbusFrame            │  │
│  └──────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────┐  │
│  │ Configuration & Services                         │  │
│  │  StationRuntime │ ConfigLoader │ RecipeService   │  │
│  └──────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

## Supported Devices

| Device | Model | Protocol | Function |
|--------|-------|----------|----------|
| Motor Drive | AQMD3610NS-A2 | Modbus RTU | Speed control, magnet detection |
| Torque Sensor | DYN200 | Modbus RTU | Torque, speed, power measurement |
| Encoder | Single-turn absolute | Modbus RTU | Angle positioning |
| Brake Power | Dual-channel DC supply | Modbus RTU | Load simulation |

## Test Workflow

1. **Homing Phase**: Detect magnet position and set encoder zero
2. **Idle Phase**: Stabilize at low speed, verify baseline torque
3. **Angle Phase**: Position to target angle, verify accuracy
4. **Load Phase**: Apply brake load, measure torque and lock detection
5. **Return Phase**: Return to home position

## Project Status

**Current Progress**: ~70% Complete

✅ **Completed**:
- Infrastructure layer (bus, devices, config)
- Domain layer (test engine, state machine)
- ViewModel layer (MVVM binding)
- Mock/simulation framework
- Comprehensive test suite

🚧 **In Progress**:
- Hardware validation
- Report generation
- Logging system

See [IMPLEMENTATION_PROGRESS.md](IMPLEMENTATION_PROGRESS.md) for detailed status.

## Documentation

- **[User Manual](Docs/USER_MANUAL.md)** - Operation guide
- **[Deployment Guide](Docs/DEPLOYMENT_GUIDE.md)** - Installation and setup
- **[Troubleshooting](Docs/TROUBLESHOOTING.md)** - Common issues
- **[Mock Mode Guide](Docs/MOCK_MODE_GUIDE.md)** - Simulation usage
- **[Device Register Reference](Docs/Device_Register_Reference_Guide.md)** - Hardware specs

## Configuration

### Station Configuration

Edit `config/station.json`:

```json
{
  "stationId": "STATION-001",
  "devices": {
    "motor": {
      "type": "AQMD3610NS-A2",
      "slaveId": 1,
      "serialPort": "COM3"
    },
    "torqueSensor": {
      "type": "DYN200",
      "slaveId": 2,
      "serialPort": "COM4"
    }
  }
}
```

### Recipe Configuration

Edit `config/recipes/GBX-42A.json`:

```json
{
  "recipeId": "GBX-42A",
  "homingSpeed": 50,
  "idleSpeed": 100,
  "targetAngle": 180.0,
  "loadTorque": 5.0,
  "lockThreshold": 0.5
}
```

## Development

### Code Structure

```
src/
├── domain/              # Business logic (test engine, recipes)
├── infrastructure/      # Technical implementation
│   ├── bus/            # Modbus RTU communication
│   ├── devices/        # Device drivers
│   ├── config/         # Configuration system
│   ├── simulation/     # Mock devices
│   └── services/       # Application services
├── viewmodels/         # MVVM view models
└── ui/                 # QML components and pages
```

### Running in Mock Mode

The application automatically uses mock devices when real hardware is unavailable:

```cpp
// In main.cpp
auto runtime = StationRuntimeFactory::create(config);
// Falls back to simulation if serial ports unavailable
```

### Adding a New Device

1. Define interface in `src/infrastructure/devices/IYourDevice.h`
2. Implement driver in `src/infrastructure/devices/YourDevice.cpp`
3. Add mock in `src/infrastructure/simulation/MockYourDevice.cpp`
4. Register in `StationRuntimeFactory`

## Testing

### Test Categories

- **Unit Tests**: Domain logic, protocol parsing
- **Integration Tests**: Device communication, state machine
- **UI Tests**: QML component rendering
- **Simulation Tests**: Mock device behavior

### Test Coverage

```bash
# Run all tests with coverage
cmake --build build --target run-all-tests

# Run specific test suite
.\build\DomainEngineTests.exe
.\build\ProtocolLayerTests.exe
.\build\SimulationRuntimeTests.exe
```

## Troubleshooting

### Serial Port Access

**Windows**: Ensure COM ports are not in use by other applications

**Linux**: Add user to `dialout` group:
```bash
sudo usermod -a -G dialout $USER
```

### Build Issues

**Qt not found**: Set `CMAKE_PREFIX_PATH`:
```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=F:/Qt/6.11.0/mingw_64
```

**MinGW linker errors**: Ensure MinGW bin directory is in PATH

See [TROUBLESHOOTING.md](Docs/TROUBLESHOOTING.md) for more solutions.

## License

[To be determined]

## Contributing

Contributions are welcome! Please ensure:
- Code follows C++20 standards
- All tests pass before submitting PR
- QML components follow AppTheme conventions
- Device implementations include mock counterparts

---

**Project Version**: 0.1  
**Last Updated**: 2026-04-24
