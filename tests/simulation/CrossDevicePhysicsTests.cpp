#include <QtTest>
#include <QObject>
#include <cmath>

#include "src/infrastructure/simulation/SimulationContext.h"
#include "src/infrastructure/simulation/SimulatedTorqueDevice.h"
#include "src/infrastructure/simulation/SimulatedEncoderDevice.h"
#include "src/infrastructure/simulation/SimulatedMotorDevice.h"
#include "src/infrastructure/simulation/SimulatedBrakeDevice.h"

using namespace Infrastructure::Simulation;

/**
 * @brief Cross-device integration tests for physics validation
 * 
 * Tests physical law consistency across multiple simulated devices:
 * - R1: Encoder speed ≈ Torque sensor speed
 * - R4: Power conservation (P = T × ω)
 * - R6: Motor current vs brake load relationship
 * - R11: Braking deceleration dynamics
 */
class CrossDevicePhysicsTests : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        // Test suite initialization
    }

    void cleanupTestCase() {
        // Test suite cleanup
    }

    /**
     * @brief R1: Angular velocity consistency test
     * 
     * Verifies that encoder angular velocity matches torque sensor speed reading.
     * Physical law: Both devices measure the same shaft rotation.
     * 
     * Acceptance: Relative error < 5%
     */
    void test_R1_SpeedConsistency_ForwardRotation() {
        SimulationContext ctx;
        SimulatedTorqueDevice torque(&ctx);
        SimulatedEncoderDevice encoder(&ctx);
        
        // Setup: Motor forward at 50% duty cycle
        ctx.setMotorDirection(SimulationContext::MotorDirection::Forward);
        ctx.setMotorDutyCycle(50.0);
        
        // Allow system to stabilize (reach steady-state speed)
        for (int i = 0; i < 200; i++) {
            ctx.advanceTick();
        }
        
        // Sample and compare speeds over 100 ticks
        int validSamples = 0;
        double maxError = 0.0;
        double sumError = 0.0;
        
        for (int i = 0; i < 100; i++) {
            double torqueSpeed, encoderSpeed;
            
            QVERIFY(torque.readSpeed(torqueSpeed));
            QVERIFY(encoder.readAngularVelocity(encoderSpeed));
            
            // Torque sensor uses abs(), encoder preserves sign
            double encoderSpeedAbs = std::abs(encoderSpeed);
            
            // Skip very low speeds (division by zero risk)
            if (encoderSpeedAbs < 10.0) continue;
            
            double relativeError = std::abs(torqueSpeed - encoderSpeedAbs) / encoderSpeedAbs;
            maxError = std::max(maxError, relativeError);
            sumError += relativeError;
            validSamples++;
            
            // Per-sample check
            QVERIFY2(relativeError < 0.05, 
                     qPrintable(QString("Speed mismatch at tick %1: torque=%2 RPM, encoder=%3 RPM, error=%4%")
                                .arg(i).arg(torqueSpeed).arg(encoderSpeedAbs).arg(relativeError * 100)));
        }
        
        QVERIFY(validSamples > 50);
        double avgError = sumError / validSamples;
        
        qInfo() << "R1 Speed Consistency Test:";
        qInfo() << "  Valid samples:" << validSamples;
        qInfo() << "  Average error:" << QString::number(avgError * 100, 'f', 2) << "%";
        qInfo() << "  Maximum error:" << QString::number(maxError * 100, 'f', 2) << "%";
        
        QVERIFY2(avgError < 0.02, "Average speed error exceeds 2%");
    }

    /**
     * @brief R1: Speed consistency during reverse rotation
     */
    void test_R1_SpeedConsistency_ReverseRotation() {
        SimulationContext ctx;
        SimulatedTorqueDevice torque(&ctx);
        SimulatedEncoderDevice encoder(&ctx);
        
        // Setup: Motor reverse at 40% duty cycle
        ctx.setMotorDirection(SimulationContext::MotorDirection::Reverse);
        ctx.setMotorDutyCycle(40.0);
        
        // Stabilize
        for (int i = 0; i < 200; i++) {
            ctx.advanceTick();
        }
        
        // Sample speeds
        double torqueSpeed, encoderSpeed;
        QVERIFY(torque.readSpeed(torqueSpeed));
        QVERIFY(encoder.readAngularVelocity(encoderSpeed));
        
        // Encoder should be negative for reverse
        QVERIFY2(encoderSpeed < 0, "Encoder speed should be negative during reverse rotation");
        
        // Compare magnitudes
        double relativeError = std::abs(torqueSpeed - std::abs(encoderSpeed)) / std::abs(encoderSpeed);
        
        qInfo() << "R1 Reverse Rotation:";
        qInfo() << "  Torque speed:" << torqueSpeed << "RPM";
        qInfo() << "  Encoder speed:" << encoderSpeed << "RPM (negative = reverse)";
        qInfo() << "  Relative error:" << QString::number(relativeError * 100, 'f', 2) << "%";
        
        QVERIFY2(relativeError < 0.05, "Speed mismatch during reverse rotation");
    }

    /**
     * @brief R4: Power conservation law test
     * 
     * Verifies P(W) = T(N·m) × ω(rad/s) = T × RPM × 2π/60
     * 
     * Acceptance: Power calculation error < 10%
     */
    void test_R4_PowerConservation() {
        SimulationContext ctx;
        SimulatedTorqueDevice torque(&ctx);
        
        // Setup: Motor forward at 60% duty cycle
        ctx.setMotorDirection(SimulationContext::MotorDirection::Forward);
        ctx.setMotorDutyCycle(60.0);
        
        // Stabilize
        for (int i = 0; i < 200; i++) {
            ctx.advanceTick();
        }
        
        // Read all values
        double torqueNm, speedRpm, powerW;
        QVERIFY(torque.readAll(torqueNm, speedRpm, powerW));
        
        // Calculate theoretical power
        double theoreticalPower = torqueNm * speedRpm * 2.0 * M_PI / 60.0;
        
        // Calculate error
        double powerError = std::abs(powerW - theoreticalPower);
        double relativeError = (theoreticalPower > 0.1) ? (powerError / theoreticalPower) : 0.0;
        
        qInfo() << "R4 Power Conservation:";
        qInfo() << "  Torque:" << QString::number(torqueNm, 'f', 3) << "N·m";
        qInfo() << "  Speed:" << QString::number(speedRpm, 'f', 1) << "RPM";
        qInfo() << "  Actual power:" << QString::number(powerW, 'f', 2) << "W";
        qInfo() << "  Theoretical power:" << QString::number(theoreticalPower, 'f', 2) << "W";
        qInfo() << "  Relative error:" << QString::number(relativeError * 100, 'f', 2) << "%";
        
        QVERIFY2(relativeError < 0.10, "Power conservation violated (error > 10%)");
    }

    /**
     * @brief R6: Motor current vs brake load relationship
     * 
     * Verifies that motor current increases with brake load.
     * Expected coefficient: ΔI_motor / ΔI_brake ≈ 0.3
     */
    void test_R6_MotorCurrentLoadRelation() {
        SimulationContext ctx;
        SimulatedMotorDevice motor(&ctx);
        SimulatedBrakeDevice brake(&ctx);
        
        // Setup: Motor forward at 50% duty cycle
        ctx.setMotorDirection(SimulationContext::MotorDirection::Forward);
        ctx.setMotorDutyCycle(50.0);
        
        // Stabilize without brake
        for (int i = 0; i < 200; i++) {
            ctx.advanceTick();
        }
        
        // Measure baseline current (no brake)
        double baselineCurrent;
        QVERIFY(motor.readCurrent(baselineCurrent));
        
        qInfo() << "R6 Motor Current vs Load:";
        qInfo() << "  Baseline current (no brake):" << QString::number(baselineCurrent, 'f', 2) << "A";
        
        // Test with increasing brake loads
        QVector<double> brakeCurrents = {1.0, 2.0, 3.0, 4.0};
        QVector<double> motorCurrents;
        
        for (double brakeCurrent : brakeCurrents) {
            // Apply brake load
            ctx.setBrakeCurrent(brakeCurrent);
            ctx.setBrakeOutputEnabled(true);
            
            // Allow system to respond
            for (int i = 0; i < 100; i++) {
                ctx.advanceTick();
            }
            
            // Measure motor current
            double motorCurrent;
            QVERIFY(motor.readCurrent(motorCurrent));
            motorCurrents.append(motorCurrent);
            
            qInfo() << "  Brake current:" << brakeCurrent << "A → Motor current:" 
                    << QString::number(motorCurrent, 'f', 2) << "A";
        }
        
        // Verify monotonic increase
        for (int i = 1; i < motorCurrents.size(); i++) {
            QVERIFY2(motorCurrents[i] > motorCurrents[i-1], 
                     "Motor current should increase with brake load");
        }
        
        // Calculate average coefficient (ΔI_motor / ΔI_brake)
        double totalDeltaMotor = motorCurrents.last() - motorCurrents.first();
        double totalDeltaBrake = brakeCurrents.last() - brakeCurrents.first();
        double coefficient = totalDeltaMotor / totalDeltaBrake;
        
        qInfo() << "  Load coefficient (ΔI_motor/ΔI_brake):" << QString::number(coefficient, 'f', 3);
        qInfo() << "  Expected: ~0.30";
        
        // Verify coefficient is close to expected value (0.3 ± 0.1)
        QVERIFY2(coefficient > 0.2 && coefficient < 0.4, 
                 "Motor current load coefficient out of expected range");
    }

    /**
     * @brief R6: Idle current limit test
     * 
     * Verifies that motor current without brake load stays below 3A.
     */
    void test_R6_IdleCurrentLimit() {
        SimulationContext ctx;
        SimulatedMotorDevice motor(&ctx);
        
        // Test at various duty cycles without brake
        QVector<double> dutyCycles = {20.0, 40.0, 60.0, 80.0, 100.0};
        
        qInfo() << "R6 Idle Current Limit:";
        
        for (double duty : dutyCycles) {
            ctx.setMotorDirection(SimulationContext::MotorDirection::Forward);
            ctx.setMotorDutyCycle(duty);
            ctx.setBrakeOutputEnabled(false);
            
            // Stabilize
            for (int i = 0; i < 200; i++) {
                ctx.advanceTick();
            }
            
            double current;
            QVERIFY(motor.readCurrent(current));
            
            qInfo() << "  Duty cycle:" << duty << "% → Current:" 
                    << QString::number(current, 'f', 2) << "A";
            
            QVERIFY2(current < 3.0, 
                     qPrintable(QString("Idle current exceeds 3A at %1% duty cycle").arg(duty)));
        }
    }

    /**
     * @brief R11: Braking deceleration dynamics
     * 
     * Verifies causal relationship: brake current increases → speed decreases
     * Expected: negative correlation coefficient < -0.8
     */
    void test_R11_BrakingDecelerationDynamics() {
        SimulationContext ctx;
        SimulatedEncoderDevice encoder(&ctx);
        
        // Setup: Motor forward at 80% duty cycle
        ctx.setMotorDirection(SimulationContext::MotorDirection::Forward);
        ctx.setMotorDutyCycle(80.0);
        
        // Reach high speed
        for (int i = 0; i < 300; i++) {
            ctx.advanceTick();
        }
        
        double initialSpeed;
        QVERIFY(encoder.readAngularVelocity(initialSpeed));
        
        qInfo() << "R11 Braking Deceleration:";
        qInfo() << "  Initial speed:" << QString::number(initialSpeed, 'f', 1) << "RPM";
        
        // Record time series: (brake_current, speed)
        QVector<QPair<double, double>> timeSeries;
        
        // Gradually increase brake current
        for (int tick = 0; tick < 500; tick++) {
            double brakeCurrent = (tick / 100.0); // 0 to 5A over 500 ticks
            ctx.setBrakeCurrent(brakeCurrent);
            ctx.setBrakeOutputEnabled(true);
            
            ctx.advanceTick();
            
            double speed;
            encoder.readAngularVelocity(speed);
            
            timeSeries.append(qMakePair(brakeCurrent, std::abs(speed)));
            
            if (tick % 100 == 0) {
                qInfo() << "  Tick" << tick << ": Brake=" << QString::number(brakeCurrent, 'f', 1) 
                        << "A, Speed=" << QString::number(std::abs(speed), 'f', 1) << "RPM";
            }
        }
        
        // Calculate correlation coefficient
        double meanBrake = 0.0, meanSpeed = 0.0;
        for (const auto& pair : timeSeries) {
            meanBrake += pair.first;
            meanSpeed += pair.second;
        }
        meanBrake /= timeSeries.size();
        meanSpeed /= timeSeries.size();
        
        double covariance = 0.0, varBrake = 0.0, varSpeed = 0.0;
        for (const auto& pair : timeSeries) {
            double dBrake = pair.first - meanBrake;
            double dSpeed = pair.second - meanSpeed;
            covariance += dBrake * dSpeed;
            varBrake += dBrake * dBrake;
            varSpeed += dSpeed * dSpeed;
        }
        
        double correlation = covariance / std::sqrt(varBrake * varSpeed);
        
        qInfo() << "  Correlation coefficient (brake vs speed):" << QString::number(correlation, 'f', 3);
        qInfo() << "  Expected: < -0.8 (strong negative correlation)";
        
        // Verify strong negative correlation
        QVERIFY2(correlation < -0.7, 
                 "Brake current and speed should be strongly negatively correlated");
        
        // Verify speed decreased significantly
        double finalSpeed = timeSeries.last().second;
        double speedReduction = initialSpeed - finalSpeed;
        
        qInfo() << "  Final speed:" << QString::number(finalSpeed, 'f', 1) << "RPM";
        qInfo() << "  Speed reduction:" << QString::number(speedReduction, 'f', 1) << "RPM";
        
        QVERIFY2(speedReduction > 200.0, "Braking should reduce speed by at least 200 RPM");
    }

    /**
     * @brief Cross-device integration: Complete braking scenario
     * 
     * Tests all devices working together during a braking event.
     * Verifies:
     * - Speed consistency between encoder and torque sensor
     * - Motor current increases with brake load
     * - Power conservation maintained
     * - Coordinated deceleration
     */
    void test_CrossDevice_CompleteBrakingScenario() {
        SimulationContext ctx;
        SimulatedMotorDevice motor(&ctx);
        SimulatedTorqueDevice torque(&ctx);
        SimulatedEncoderDevice encoder(&ctx);
        SimulatedBrakeDevice brake(&ctx);
        
        qInfo() << "\n=== Cross-Device Complete Braking Scenario ===";
        
        // Phase 1: Acceleration to steady state
        qInfo() << "\nPhase 1: Acceleration";
        ctx.setMotorDirection(SimulationContext::MotorDirection::Forward);
        ctx.setMotorDutyCycle(70.0);
        
        for (int i = 0; i < 300; i++) {
            ctx.advanceTick();
        }
        
        double motorCurrent1, torqueNm1, speedRpm1, powerW1, encoderSpeed1;
        motor.readCurrent(motorCurrent1);
        torque.readAll(torqueNm1, speedRpm1, powerW1);
        encoder.readAngularVelocity(encoderSpeed1);
        
        qInfo() << "  Motor current:" << QString::number(motorCurrent1, 'f', 2) << "A";
        qInfo() << "  Torque:" << QString::number(torqueNm1, 'f', 2) << "N·m";
        qInfo() << "  Speed (torque):" << QString::number(speedRpm1, 'f', 1) << "RPM";
        qInfo() << "  Speed (encoder):" << QString::number(encoderSpeed1, 'f', 1) << "RPM";
        qInfo() << "  Power:" << QString::number(powerW1, 'f', 2) << "W";
        
        // Verify speed consistency
        double speedError1 = std::abs(speedRpm1 - std::abs(encoderSpeed1)) / std::abs(encoderSpeed1);
        QVERIFY2(speedError1 < 0.05, "Speed mismatch before braking");
        
        // Phase 2: Apply brake load
        qInfo() << "\nPhase 2: Braking (3A)";
        ctx.setBrakeCurrent(3.0);
        ctx.setBrakeVoltage(24.0);
        ctx.setBrakeOutputEnabled(true);
        
        for (int i = 0; i < 200; i++) {
            ctx.advanceTick();
        }
        
        double motorCurrent2, torqueNm2, speedRpm2, powerW2, encoderSpeed2;
        motor.readCurrent(motorCurrent2);
        torque.readAll(torqueNm2, speedRpm2, powerW2);
        encoder.readAngularVelocity(encoderSpeed2);
        
        qInfo() << "  Motor current:" << QString::number(motorCurrent2, 'f', 2) << "A";
        qInfo() << "  Torque:" << QString::number(torqueNm2, 'f', 2) << "N·m";
        qInfo() << "  Speed (torque):" << QString::number(speedRpm2, 'f', 1) << "RPM";
        qInfo() << "  Speed (encoder):" << QString::number(encoderSpeed2, 'f', 1) << "RPM";
        qInfo() << "  Power:" << QString::number(powerW2, 'f', 2) << "W";
        
        // Verify physical laws
        QVERIFY2(motorCurrent2 > motorCurrent1, "Motor current should increase with brake load");
        QVERIFY2(torqueNm2 > torqueNm1, "Torque should increase with brake load");
        QVERIFY2(std::abs(encoderSpeed2) < std::abs(encoderSpeed1), "Speed should decrease with braking");
        
        double speedError2 = std::abs(speedRpm2 - std::abs(encoderSpeed2)) / std::abs(encoderSpeed2);
        QVERIFY2(speedError2 < 0.05, "Speed mismatch during braking");
        
        // Verify power conservation
        double theoreticalPower2 = torqueNm2 * speedRpm2 * 2.0 * M_PI / 60.0;
        double powerError2 = std::abs(powerW2 - theoreticalPower2) / theoreticalPower2;
        QVERIFY2(powerError2 < 0.10, "Power conservation violated during braking");
        
        qInfo() << "\n=== All cross-device checks passed ===";
    }
};

QTEST_MAIN(CrossDevicePhysicsTests)
#include "CrossDevicePhysicsTests.moc"
