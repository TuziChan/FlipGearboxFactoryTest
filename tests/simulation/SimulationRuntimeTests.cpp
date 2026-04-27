#include <QCoreApplication>
#include <QtTest>
#include <QSignalSpy>

#include "src/infrastructure/config/StationRuntimeFactory.h"
#include "src/infrastructure/config/StationConfig.h"
#include "src/infrastructure/simulation/SimulationContext.h"
#include "src/infrastructure/simulation/SimulatedMotorDevice.h"
#include "src/infrastructure/simulation/SimulatedTorqueDevice.h"
#include "src/infrastructure/simulation/SimulatedEncoderDevice.h"
#include "src/infrastructure/simulation/SimulatedBrakeDevice.h"
#include "src/domain/TestRecipe.h"

using namespace Infrastructure;
using namespace Infrastructure::Config;
using namespace Infrastructure::Simulation;
using namespace Domain;

/**
 * @brief Tests for simulation runtime components
 * 
 * This test suite covers:
 * - StationRuntimeFactory with mockMode=true
 * - StationRuntime initialize/shutdown lifecycle
 * - SimulationContext physics simulation
 * - All simulated device implementations
 * 
 * Tests emphasize deterministic behavior and avoid flaky timing issues.
 */
class SimulationRuntimeTests : public QObject {
    Q_OBJECT

private:
    StationConfig makeTestConfig() {
        StationConfig config;
        config.stationName = "SimTest";
        config.brakeChannel = 1;
        
        config.aqmdConfig.enabled = true;
        config.aqmdConfig.portName = "SIM_AQMD";
        config.aqmdConfig.baudRate = 9600;
        config.aqmdConfig.slaveId = 1;
        config.aqmdConfig.timeout = 1000;
        config.aqmdConfig.parity = "None";
        config.aqmdConfig.stopBits = 1;
        config.aqmdConfig.pollIntervalUs = 10000;
        
        config.dyn200Config.enabled = true;
        config.dyn200Config.portName = "SIM_DYN200";
        config.dyn200Config.baudRate = 9600;
        config.dyn200Config.slaveId = 1;
        config.dyn200Config.timeout = 1000;
        config.dyn200Config.parity = "None";
        config.dyn200Config.stopBits = 1;
        config.dyn200Config.communicationMode = 0;
        config.dyn200Config.pollIntervalUs = 10000;
        
        config.encoderConfig.enabled = true;
        config.encoderConfig.portName = "SIM_ENCODER";
        config.encoderConfig.baudRate = 9600;
        config.encoderConfig.slaveId = 1;
        config.encoderConfig.timeout = 1000;
        config.encoderConfig.parity = "None";
        config.encoderConfig.stopBits = 1;
        config.encoderConfig.encoderResolution = 4096;
        config.encoderConfig.communicationMode = 0;
        config.encoderConfig.pollIntervalUs = 10000;
        
        config.brakeConfig.enabled = true;
        config.brakeConfig.portName = "SIM_BRAKE";
        config.brakeConfig.baudRate = 9600;
        config.brakeConfig.slaveId = 1;
        config.brakeConfig.timeout = 1000;
        config.brakeConfig.parity = "None";
        config.brakeConfig.stopBits = 1;
        config.brakeConfig.pollIntervalUs = 10000;
        
        return config;
    }

private slots:
    // ========== StationRuntimeFactory Tests ==========
    
    void testFactoryCreatesMockRuntime() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        
        QVERIFY(runtime != nullptr);
        QVERIFY(runtime->motor() != nullptr);
        QVERIFY(runtime->torque() != nullptr);
        QVERIFY(runtime->encoder() != nullptr);
        QVERIFY(runtime->brake() != nullptr);
        QVERIFY(runtime->testEngine() != nullptr);
        QVERIFY(runtime->acquisitionScheduler() != nullptr);
        QCOMPARE(runtime->brakeChannel(), 1);
        QVERIFY(!runtime->isInitialized());
    }
    
    void testFactoryRespectsDisabledDevices() {
        StationConfig config = makeTestConfig();
        config.aqmdConfig.enabled = false;
        config.dyn200Config.enabled = false;
        
        auto runtime = StationRuntimeFactory::create(config, true);
        
        QVERIFY(runtime != nullptr);
        QVERIFY(runtime->motor() == nullptr);
        QVERIFY(runtime->torque() == nullptr);
        QVERIFY(runtime->encoder() != nullptr);
        QVERIFY(runtime->brake() != nullptr);
    }
    
    // ========== StationRuntime Lifecycle Tests ==========
    
    void testRuntimeInitializeSuccess() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        
        QVERIFY(!runtime->isInitialized());
        bool result = runtime->initialize();
        QVERIFY(result);
        QVERIFY(runtime->isInitialized());
        QVERIFY(runtime->lastError().isEmpty());
    }
    
    void testRuntimeShutdown() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        
        runtime->initialize();
        QVERIFY(runtime->isInitialized());
        
        runtime->shutdown();
        QVERIFY(!runtime->isInitialized());
    }
    
    void testRuntimeDoubleInitialize() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        
        QVERIFY(runtime->initialize());
        QVERIFY(runtime->isInitialized());
        
        // Second initialize should succeed (idempotent)
        QVERIFY(runtime->initialize());
        QVERIFY(runtime->isInitialized());
    }
    
    // ========== SimulationContext Tests ==========
    
    void testSimulationContextInitialState() {
        SimulationContext ctx;
        
        QCOMPARE(ctx.tickCount(), 0ULL);
        QCOMPARE(ctx.motorDirection(), SimulationContext::MotorDirection::Stopped);
        QCOMPARE(ctx.motorDutyCycle(), 0.0);
        QCOMPARE(ctx.brakeCurrent(), 0.0);
        QCOMPARE(ctx.brakeVoltage(), 0.0);
        QVERIFY(!ctx.brakeOutputEnabled());
        QCOMPARE(ctx.encoderAngleDeg(), 0.0);
        QCOMPARE(ctx.encoderAngularVelocityRpm(), 0.0);
    }
    
    void testSimulationContextReset() {
        SimulationContext ctx;
        
        ctx.setMotorDirection(SimulationContext::MotorDirection::Forward);
        ctx.setMotorDutyCycle(50.0);
        ctx.setBrakeCurrent(2.0);
        ctx.advanceTick();
        ctx.advanceTick();
        
        QVERIFY(ctx.tickCount() > 0);
        
        ctx.reset();
        
        QCOMPARE(ctx.tickCount(), 0ULL);
        QCOMPARE(ctx.motorDirection(), SimulationContext::MotorDirection::Stopped);
        QCOMPARE(ctx.motorDutyCycle(), 0.0);
        QCOMPARE(ctx.brakeCurrent(), 0.0);
    }
    
    void testSimulationContextTickAdvancement() {
        SimulationContext ctx;
        
        QCOMPARE(ctx.tickCount(), 0ULL);
        ctx.advanceTick();
        QCOMPARE(ctx.tickCount(), 1ULL);
        ctx.advanceTick();
        QCOMPARE(ctx.tickCount(), 2ULL);
    }
    
    void testSimulationContextMotorAcceleration() {
        SimulationContext ctx;
        
        // Start motor at 50% duty cycle forward
        ctx.setMotorDirection(SimulationContext::MotorDirection::Forward);
        ctx.setMotorDutyCycle(50.0);
        
        // Initially speed should be zero
        QCOMPARE(ctx.encoderAngularVelocityRpm(), 0.0);
        
        // After several ticks, speed should increase (deterministic acceleration)
        for (int i = 0; i < 100; i++) {
            ctx.advanceTick();
        }
        
        // Speed should be positive and approaching target (50% * 15 = 750 RPM)
        double speed = ctx.encoderAngularVelocityRpm();
        QVERIFY(speed > 0.0);
        QVERIFY(speed <= 750.0);
    }
    
    void testSimulationContextMotorDeceleration() {
        SimulationContext ctx;
        
        // Accelerate to full speed
        ctx.setMotorDirection(SimulationContext::MotorDirection::Forward);
        ctx.setMotorDutyCycle(100.0);
        for (int i = 0; i < 300; i++) {
            ctx.advanceTick();
        }
        
        double initialSpeed = ctx.encoderAngularVelocityRpm();
        QVERIFY(initialSpeed > 1000.0);
        
        // Reduce duty cycle
        ctx.setMotorDutyCycle(20.0);
        for (int i = 0; i < 100; i++) {
            ctx.advanceTick();
        }
        
        double finalSpeed = ctx.encoderAngularVelocityRpm();
        QVERIFY(finalSpeed < initialSpeed);
        QVERIFY(finalSpeed <= 300.0); // 20% * 15 = 300 RPM
    }
    
    void testSimulationContextBrakeLoadEffect() {
        SimulationContext ctx;
        
        // Accelerate motor
        ctx.setMotorDirection(SimulationContext::MotorDirection::Forward);
        ctx.setMotorDutyCycle(100.0);
        for (int i = 0; i < 300; i++) {
            ctx.advanceTick();
        }
        
        double speedWithoutBrake = ctx.encoderAngularVelocityRpm();
        
        // Apply brake load
        ctx.setBrakeCurrent(2.0);
        ctx.setBrakeOutputEnabled(true);
        for (int i = 0; i < 100; i++) {
            ctx.advanceTick();
        }
        
        double speedWithBrake = ctx.encoderAngularVelocityRpm();
        
        // Brake should reduce speed
        QVERIFY(speedWithBrake < speedWithoutBrake);
    }
    
    void testSimulationContextEncoderAngleWrapping() {
        SimulationContext ctx;
        
        // Run motor forward to accumulate angle
        ctx.setMotorDirection(SimulationContext::MotorDirection::Forward);
        ctx.setMotorDutyCycle(100.0);
        
        // Run for enough ticks to exceed 360 degrees
        for (int i = 0; i < 1000; i++) {
            ctx.advanceTick();
        }
        
        double angle = ctx.encoderAngleDeg();
        
        // Angle should be wrapped to 0-360 range
        QVERIFY(angle >= 0.0);
        QVERIFY(angle < 360.0);
    }
    
    void testSimulationContextEncoderZeroOffset() {
        SimulationContext ctx;
        
        // Advance angle
        ctx.setMotorDirection(SimulationContext::MotorDirection::Forward);
        ctx.setMotorDutyCycle(50.0);
        for (int i = 0; i < 100; i++) {
            ctx.advanceTick();
        }
        
        double angleBeforeZero = ctx.encoderAngleDeg();
        QVERIFY(angleBeforeZero > 0.0);
        
        // Set zero offset
        ctx.setEncoderZeroOffset(angleBeforeZero);
        
        // Angle should now be near zero
        double angleAfterZero = ctx.encoderAngleDeg();
        QVERIFY(qAbs(angleAfterZero) < 1.0);
    }
    
    void testSimulationContextReverseDirection() {
        SimulationContext ctx;
        
        // Run forward
        ctx.setMotorDirection(SimulationContext::MotorDirection::Forward);
        ctx.setMotorDutyCycle(50.0);
        for (int i = 0; i < 100; i++) {
            ctx.advanceTick();
        }
        
        double forwardSpeed = ctx.encoderAngularVelocityRpm();
        QVERIFY(forwardSpeed > 0.0);
        
        // Switch to reverse
        ctx.setMotorDirection(SimulationContext::MotorDirection::Reverse);
        ctx.setMotorDutyCycle(50.0);
        for (int i = 0; i < 200; i++) {
            ctx.advanceTick();
        }
        
        double reverseSpeed = ctx.encoderAngularVelocityRpm();
        QVERIFY(reverseSpeed < 0.0);
    }
    
    // ========== SimulatedMotorDevice Tests ==========
    
    void testSimulatedMotorInitialize() {
        SimulationContext ctx;
        SimulatedMotorDevice motor(&ctx);
        
        QVERIFY(motor.initialize());
        QVERIFY(motor.lastError().isEmpty());
    }
    
    void testSimulatedMotorSetForward() {
        SimulationContext ctx;
        SimulatedMotorDevice motor(&ctx);
        motor.initialize();
        
        QVERIFY(motor.setMotor(Devices::IMotorDriveDevice::Direction::Forward, 50.0));
        QCOMPARE(ctx.motorDirection(), SimulationContext::MotorDirection::Forward);
        QCOMPARE(ctx.motorDutyCycle(), 50.0);
    }
    
    void testSimulatedMotorSetReverse() {
        SimulationContext ctx;
        SimulatedMotorDevice motor(&ctx);
        motor.initialize();
        
        QVERIFY(motor.setMotor(Devices::IMotorDriveDevice::Direction::Reverse, 30.0));
        QCOMPARE(ctx.motorDirection(), SimulationContext::MotorDirection::Reverse);
        QCOMPARE(ctx.motorDutyCycle(), 30.0);
    }
    
    void testSimulatedMotorBrake() {
        SimulationContext ctx;
        SimulatedMotorDevice motor(&ctx);
        motor.initialize();
        
        motor.setMotor(Devices::IMotorDriveDevice::Direction::Forward, 50.0);
        QVERIFY(motor.brake());
        QCOMPARE(ctx.motorDirection(), SimulationContext::MotorDirection::Stopped);
    }
    
    void testSimulatedMotorCoast() {
        SimulationContext ctx;
        SimulatedMotorDevice motor(&ctx);
        motor.initialize();
        
        motor.setMotor(Devices::IMotorDriveDevice::Direction::Forward, 50.0);
        QVERIFY(motor.coast());
        QCOMPARE(ctx.motorDirection(), SimulationContext::MotorDirection::Stopped);
    }
    
    void testSimulatedMotorReadCurrent() {
        SimulationContext ctx;
        SimulatedMotorDevice motor(&ctx);
        motor.initialize();
        
        double current = 0.0;
        QVERIFY(motor.readCurrent(current));
        QVERIFY(current >= 0.0);
        QVERIFY(current <= 10.0); // Reasonable range
    }
    
    void testSimulatedMotorMagnetDetection() {
        SimulationContext ctx;
        SimulatedMotorDevice motor(&ctx);
        motor.initialize();
        
        bool level = false;
        
        // Initially magnet not detected (AI1 high = true)
        QVERIFY(motor.readAI1Level(level));
        QVERIFY(level);  // No magnet -> AI1 high (true)
        
        // Start motor
        motor.setMotor(Devices::IMotorDriveDevice::Direction::Forward, 20.0);
        
        // Advance simulation and check for magnet detection during movement
        bool magnetDetected = false;
        for (int i = 0; i < 100; i++) {
            ctx.advanceTick();
            QVERIFY(motor.readAI1Level(level));
            if (!level) {
                magnetDetected = true;  // Magnet detected -> AI1 low (false)
                break;
            }
        }
        
        // Magnet should have been detected as the motor passed through its position
        QVERIFY(magnetDetected);
    }
    
    // ========== SimulatedTorqueDevice Tests ==========
    
    void testSimulatedTorqueInitialize() {
        SimulationContext ctx;
        SimulatedTorqueDevice torque(&ctx);
        
        QVERIFY(torque.initialize());
        QVERIFY(torque.lastError().isEmpty());
    }
    
    void testSimulatedTorqueReadValue() {
        SimulationContext ctx;
        SimulatedTorqueDevice torque(&ctx);
        torque.initialize();
        
        double torqueNm = 0.0;
        QVERIFY(torque.readTorque(torqueNm));
        QVERIFY(torqueNm >= 0.0);
    }
    
    void testSimulatedTorqueIncreasesWithBrake() {
        SimulationContext ctx;
        SimulatedTorqueDevice torque(&ctx);
        torque.initialize();
        
        // Read baseline torque
        double baselineTorque = 0.0;
        torque.readTorque(baselineTorque);
        
        // Apply brake load
        ctx.setBrakeCurrent(2.0);
        ctx.setBrakeOutputEnabled(true);
        
        // Advance simulation
        for (int i = 0; i < 50; i++) {
            ctx.advanceTick();
        }
        
        double loadedTorque = 0.0;
        torque.readTorque(loadedTorque);
        
        // Torque should increase with brake load
        QVERIFY(loadedTorque > baselineTorque);
    }
    
    // ========== SimulatedEncoderDevice Tests ==========
    
    void testSimulatedEncoderInitialize() {
        SimulationContext ctx;
        SimulatedEncoderDevice encoder(&ctx);
        
        QVERIFY(encoder.initialize());
        QVERIFY(encoder.lastError().isEmpty());
    }
    
    void testSimulatedEncoderReadAngle() {
        SimulationContext ctx;
        SimulatedEncoderDevice encoder(&ctx);
        encoder.initialize();
        
        double angle = 0.0;
        QVERIFY(encoder.readAngle(angle));
        QCOMPARE(angle, 0.0);
    }
    
    void testSimulatedEncoderAngleChangesWithMotion() {
        SimulationContext ctx;
        SimulatedEncoderDevice encoder(&ctx);
        encoder.initialize();
        
        // Start motor
        ctx.setMotorDirection(SimulationContext::MotorDirection::Forward);
        ctx.setMotorDutyCycle(50.0);
        
        // Advance simulation
        for (int i = 0; i < 100; i++) {
            ctx.advanceTick();
        }
        
        double angle = 0.0;
        QVERIFY(encoder.readAngle(angle));
        QVERIFY(angle > 0.0);
    }
    
    void testSimulatedEncoderSetZero() {
        SimulationContext ctx;
        SimulatedEncoderDevice encoder(&ctx);
        encoder.initialize();
        
        // Advance angle
        ctx.setMotorDirection(SimulationContext::MotorDirection::Forward);
        ctx.setMotorDutyCycle(50.0);
        for (int i = 0; i < 100; i++) {
            ctx.advanceTick();
        }
        
        double angleBeforeZero = 0.0;
        encoder.readAngle(angleBeforeZero);
        QVERIFY(angleBeforeZero > 0.0);
        
        // Set zero
        QVERIFY(encoder.setZeroPoint());
        
        double angleAfterZero = 0.0;
        encoder.readAngle(angleAfterZero);
        QVERIFY(qAbs(angleAfterZero) < 1.0);
    }
    
    void testSimulatedEncoderReadSpeed() {
        SimulationContext ctx;
        SimulatedEncoderDevice encoder(&ctx);
        encoder.initialize();

        // Start motor and let it stabilize
        ctx.setMotorDirection(SimulationContext::MotorDirection::Forward);
        ctx.setMotorDutyCycle(50.0);
        for (int i = 0; i < 200; i++) {
            ctx.advanceTick();
        }

        double speedRpm = 0.0;
        QVERIFY(encoder.readAngularVelocity(speedRpm));
        QVERIFY(speedRpm > 0.0);
        QVERIFY(speedRpm <= 750.0); // 50% * 15 = 750 RPM max
    }
    
    // ========== SimulatedBrakeDevice Tests ==========
    
    void testSimulatedBrakeInitialize() {
        SimulationContext ctx;
        SimulatedBrakeDevice brake(&ctx);
        
        QVERIFY(brake.initialize());
        QVERIFY(brake.lastError().isEmpty());
    }
    
    void testSimulatedBrakeSetConstantCurrent() {
        SimulationContext ctx;
        SimulatedBrakeDevice brake(&ctx);
        brake.initialize();

        QVERIFY(brake.setBrakeMode(1, "CC"));
        QVERIFY(brake.setCurrent(1, 2.5));
        QVERIFY(brake.setOutputEnable(1, true));
        QCOMPARE(ctx.brakeCurrent(), 2.5);
        QVERIFY(ctx.brakeOutputEnabled());
    }
    
    void testSimulatedBrakeSetConstantVoltage() {
        SimulationContext ctx;
        SimulatedBrakeDevice brake(&ctx);
        brake.initialize();

        QVERIFY(brake.setBrakeMode(1, "CV"));
        QVERIFY(brake.setVoltage(1, 12.0));
        QVERIFY(brake.setOutputEnable(1, true));
        QCOMPARE(ctx.brakeVoltage(), 12.0);
        QVERIFY(ctx.brakeOutputEnabled());
    }
    
    void testSimulatedBrakeDisableOutput() {
        SimulationContext ctx;
        SimulatedBrakeDevice brake(&ctx);
        brake.initialize();

        brake.setBrakeMode(1, "CC");
        brake.setCurrent(1, 2.0);
        brake.setOutputEnable(1, true);
        QVERIFY(ctx.brakeOutputEnabled());

        QVERIFY(brake.setOutputEnable(1, false));
        QVERIFY(!ctx.brakeOutputEnabled());
    }
    
    void testSimulatedBrakeReadCurrent() {
        SimulationContext ctx;
        SimulatedBrakeDevice brake(&ctx);
        brake.initialize();

        brake.setBrakeMode(1, "CC");
        brake.setCurrent(1, 2.5);
        brake.setOutputEnable(1, true);

        double current = 0.0;
        QVERIFY(brake.readCurrent(1, current));
        QCOMPARE(current, 2.5);
    }
    
    void testSimulatedBrakeReadVoltage() {
        SimulationContext ctx;
        SimulatedBrakeDevice brake(&ctx);
        brake.initialize();

        brake.setBrakeMode(1, "CV");
        brake.setVoltage(1, 12.0);
        brake.setOutputEnable(1, true);

        double voltage = 0.0;
        QVERIFY(brake.readVoltage(1, voltage));
        QCOMPARE(voltage, 12.0);
    }
    
    void testSimulatedBrakeInvalidChannel() {
        SimulationContext ctx;
        SimulatedBrakeDevice brake(&ctx);
        brake.initialize();

        // Channel 0 or > 4 should fail
        QVERIFY(!brake.setCurrent(0, 2.0));
        QVERIFY(!brake.setCurrent(5, 2.0));
    }
    
    // ========== Integration Tests ==========
    
    void testFullSimulationCycle() {
        StationConfig config = makeTestConfig();
        auto runtime = StationRuntimeFactory::create(config, true);
        
        QVERIFY(runtime->initialize());
        
        // Set motor forward
        QVERIFY(runtime->motor()->setMotor(Devices::IMotorDriveDevice::Direction::Forward, 50.0));
        
        // Apply brake
        QVERIFY(runtime->brake()->setBrakeMode(1, "CC"));
        QVERIFY(runtime->brake()->setCurrent(1, 1.5));
        QVERIFY(runtime->brake()->setOutputEnable(1, true));
        
        // Read encoder angle
        double angle = 0.0;
        QVERIFY(runtime->encoder()->readAngle(angle));
        
        // Read torque
        double torque = 0.0;
        QVERIFY(runtime->torque()->readTorque(torque));
        
        runtime->shutdown();
        QVERIFY(!runtime->isInitialized());
    }
    
    void testSimulationDeterminism() {
        // Run same simulation twice, verify identical results
        SimulationContext ctx1, ctx2;
        
        // Configure identically
        ctx1.setMotorDirection(SimulationContext::MotorDirection::Forward);
        ctx1.setMotorDutyCycle(50.0);
        ctx2.setMotorDirection(SimulationContext::MotorDirection::Forward);
        ctx2.setMotorDutyCycle(50.0);
        
        // Run same number of ticks
        for (int i = 0; i < 100; i++) {
            ctx1.advanceTick();
            ctx2.advanceTick();
        }
        
        // Results should be identical
        QCOMPARE(ctx1.encoderAngleDeg(), ctx2.encoderAngleDeg());
        QCOMPARE(ctx1.encoderAngularVelocityRpm(), ctx2.encoderAngularVelocityRpm());
    }
};

QTEST_MAIN(SimulationRuntimeTests)
#include "SimulationRuntimeTests.moc"
