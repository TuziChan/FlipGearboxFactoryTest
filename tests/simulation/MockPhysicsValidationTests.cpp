#include <QtTest/QtTest>
#include "../../src/infrastructure/simulation/SimulationContext.h"
#include "../../src/infrastructure/simulation/SimulatedTorqueDevice.h"
#include "../../src/infrastructure/simulation/SimulatedMotorDevice.h"
#include "../../src/infrastructure/simulation/SimulatedBrakeDevice.h"
#include "../../src/infrastructure/simulation/SimulatedEncoderDevice.h"
#include <cmath>

using namespace Infrastructure::Simulation;

/**
 * @brief P0优先级物理规律验证测试套件
 *
 * 覆盖检查点：
 * - R1: 角速度-转速一致性
 * - R4: 功率守恒定律
 * - R7: 制动功率计算
 */
class MockPhysicsValidationTests : public QObject {
    Q_OBJECT

private slots:
    // R7: 制动功率计算测试
    void testBrakePowerCalculation_R7();
    void testBrakePowerCalculation_MultiChannel_R7();

    // R4: 功率守恒定律测试
    void testPowerConservationLaw_R4();
    void testPowerConservationLaw_WithBrake_R4();

    // R1: 角速度-转速一致性测试
    void testSpeedConsistency_R1();
    void testSpeedConsistency_Reverse_R1();
    void testSpeedConsistency_WithBraking_R1();

    // 综合测试
    void testCombinedPhysicsValidation();
};

// ============================================================================
// R7: 制动功率计算 (P0优先级)
// 物理公式: P_brake = U × I
// 判据: |P_actual - P_theory| < 0.5W
// ============================================================================

void MockPhysicsValidationTests::testBrakePowerCalculation_R7() {
    SimulationContext ctx;
    SimulatedBrakeDevice brakeDevice(&ctx);

    QVERIFY(brakeDevice.initialize());

    // 测试场景1: 低功率工况
    QVERIFY(brakeDevice.setCurrent(1, 1.0));
    QVERIFY(brakeDevice.setVoltage(1, 12.0));
    QVERIFY(brakeDevice.setOutputEnable(1, true));

    double voltage, current, power;
    QVERIFY(brakeDevice.readVoltage(1, voltage));
    QVERIFY(brakeDevice.readCurrent(1, current));
    QVERIFY(brakeDevice.readPower(1, power));

    double theoreticalPower = voltage * current;
    double error = std::abs(power - theoreticalPower);

    QVERIFY2(error < 0.5,
        QString("Low power: V=%1V, I=%2A, P_actual=%3W, P_theory=%4W, error=%5W")
        .arg(voltage).arg(current).arg(power).arg(theoreticalPower).arg(error).toUtf8());

    // 测试场景2: 高功率工况
    QVERIFY(brakeDevice.setCurrent(1, 4.5));
    QVERIFY(brakeDevice.setVoltage(1, 24.0));

    QVERIFY(brakeDevice.readVoltage(1, voltage));
    QVERIFY(brakeDevice.readCurrent(1, current));
    QVERIFY(brakeDevice.readPower(1, power));

    theoreticalPower = voltage * current;
    error = std::abs(power - theoreticalPower);

    QVERIFY2(error < 0.5,
        QString("High power: V=%1V, I=%2A, P_actual=%3W, P_theory=%4W, error=%5W")
        .arg(voltage).arg(current).arg(power).arg(theoreticalPower).arg(error).toUtf8());

    // 测试场景3: 零功率工况
    QVERIFY(brakeDevice.setCurrent(1, 0.0));
    QVERIFY(brakeDevice.setVoltage(1, 0.0));

    QVERIFY(brakeDevice.readPower(1, power));
    QVERIFY2(std::abs(power) < 0.5, "Zero power scenario failed");
}

void MockPhysicsValidationTests::testBrakePowerCalculation_MultiChannel_R7() {
    SimulationContext ctx;
    SimulatedBrakeDevice brakeDevice(&ctx);

    QVERIFY(brakeDevice.initialize());

    // 测试双通道功率计算独立性
    QVERIFY(brakeDevice.setCurrent(1, 2.0));
    QVERIFY(brakeDevice.setVoltage(1, 15.0));
    QVERIFY(brakeDevice.setCurrent(2, 3.0));
    QVERIFY(brakeDevice.setVoltage(2, 20.0));

    double voltage1, current1, power1;
    double voltage2, current2, power2;

    QVERIFY(brakeDevice.readVoltage(1, voltage1));
    QVERIFY(brakeDevice.readCurrent(1, current1));
    QVERIFY(brakeDevice.readPower(1, power1));

    QVERIFY(brakeDevice.readVoltage(2, voltage2));
    QVERIFY(brakeDevice.readCurrent(2, current2));
    QVERIFY(brakeDevice.readPower(2, power2));

    QVERIFY2(std::abs(power1 - voltage1 * current1) < 0.5, "Channel 1 power mismatch");
    QVERIFY2(std::abs(power2 - voltage2 * current2) < 0.5, "Channel 2 power mismatch");
}

// ============================================================================
// R4: 功率守恒定律 (P0优先级)
// 物理公式: P(W) = T(N·m) × ω(rad/s) = T × RPM × 2π/60
// 判据: 偏差 < 10%
// ============================================================================

void MockPhysicsValidationTests::testPowerConservationLaw_R4() {
    SimulationContext ctx;
    SimulatedTorqueDevice torqueDevice(&ctx);

    QVERIFY(torqueDevice.initialize());

    // 设置电机运行状态
    ctx.setMotorDirection(SimulationContext::MotorDirection::Forward);
    ctx.setMotorDutyCycle(50.0);

    // 等待转速稳定
    for (int i = 0; i < 50; i++) {
        ctx.advanceTick();
    }

    // 采样100个tick验证功率守恒
    int validSamples = 0;
    int passedSamples = 0;

    for (int i = 0; i < 100; i++) {
        double torque, speed, power;
        QVERIFY(torqueDevice.readAll(torque, speed, power));

        // 跳过低功率区间（噪声影响大）
        if (power < 10.0) {
            continue;
        }

        validSamples++;

        double theoreticalPower = torque * speed * 2.0 * M_PI / 60.0;
        double relativeError = std::abs(power - theoreticalPower) / theoreticalPower;

        if (relativeError < 0.10) {
            passedSamples++;
        }
    }

    // 要求至少95%的有效样本通过
    double passRate = static_cast<double>(passedSamples) / validSamples;
    QVERIFY2(passRate >= 0.95,
        QString("Power conservation pass rate: %1% (%2/%3)")
        .arg(passRate * 100.0).arg(passedSamples).arg(validSamples).toUtf8());
}

void MockPhysicsValidationTests::testPowerConservationLaw_WithBrake_R4() {
    SimulationContext ctx;
    SimulatedTorqueDevice torqueDevice(&ctx);
    SimulatedBrakeDevice brakeDevice(&ctx);

    QVERIFY(torqueDevice.initialize());
    QVERIFY(brakeDevice.initialize());

    // 设置电机和制动
    ctx.setMotorDirection(SimulationContext::MotorDirection::Forward);
    ctx.setMotorDutyCycle(80.0);

    QVERIFY(brakeDevice.setCurrent(1, 2.0));
    QVERIFY(brakeDevice.setOutputEnable(1, true));

    // 等待系统稳定
    for (int i = 0; i < 100; i++) {
        ctx.advanceTick();
    }

    // 验证负载工况下的功率守恒
    int validSamples = 0;
    int passedSamples = 0;

    for (int i = 0; i < 100; i++) {
        double torque, speed, power;
        QVERIFY(torqueDevice.readAll(torque, speed, power));

        if (power < 10.0) continue;

        validSamples++;

        double theoreticalPower = torque * speed * 2.0 * M_PI / 60.0;
        double relativeError = std::abs(power - theoreticalPower) / theoreticalPower;

        if (relativeError < 0.10) {
            passedSamples++;
        }
    }

    double passRate = static_cast<double>(passedSamples) / validSamples;
    QVERIFY2(passRate >= 0.95,
        QString("Power conservation with brake pass rate: %1%").arg(passRate * 100.0).toUtf8());
}

// ============================================================================
// R1: 角速度-转速一致性 (P0优先级)
// 物理公式: 编码器角速度(RPM) ≈ 扭矩传感器转速(RPM)
// 判据: 误差 < 5%
// ============================================================================

void MockPhysicsValidationTests::testSpeedConsistency_R1() {
    SimulationContext ctx;
    SimulatedTorqueDevice torqueDevice(&ctx);
    SimulatedEncoderDevice encoderDevice(&ctx);

    QVERIFY(torqueDevice.initialize());
    QVERIFY(encoderDevice.initialize());

    // 测试场景1: 正转
    ctx.setMotorDirection(SimulationContext::MotorDirection::Forward);
    ctx.setMotorDutyCycle(60.0);

    // 等待转速稳定
    for (int i = 0; i < 50; i++) {
        ctx.advanceTick();
    }

    int validSamples = 0;
    int passedSamples = 0;

    for (int i = 0; i < 100; i++) {
        double torqueSpeed;
        QVERIFY(torqueDevice.readSpeed(torqueSpeed));

        double encoderSpeed = std::abs(ctx.encoderAngularVelocityRpm());

        // 跳过低速区间（相对误差不稳定）
        if (encoderSpeed < 10.0) continue;

        validSamples++;

        double maxSpeed = std::max(torqueSpeed, encoderSpeed);
        double relativeError = std::abs(torqueSpeed - encoderSpeed) / maxSpeed;

        if (relativeError < 0.05) {
            passedSamples++;
        }
    }

    double passRate = static_cast<double>(passedSamples) / validSamples;
    QVERIFY2(passRate >= 0.95,
        QString("Speed consistency pass rate: %1% (%2/%3)")
        .arg(passRate * 100.0).arg(passedSamples).arg(validSamples).toUtf8());
}

void MockPhysicsValidationTests::testSpeedConsistency_Reverse_R1() {
    SimulationContext ctx;
    SimulatedTorqueDevice torqueDevice(&ctx);

    QVERIFY(torqueDevice.initialize());

    // 测试场景2: 反转
    ctx.setMotorDirection(SimulationContext::MotorDirection::Reverse);
    ctx.setMotorDutyCycle(40.0);

    for (int i = 0; i < 50; i++) {
        ctx.advanceTick();
    }

    int validSamples = 0;
    int passedSamples = 0;

    for (int i = 0; i < 100; i++) {
        double torqueSpeed;
        QVERIFY(torqueDevice.readSpeed(torqueSpeed));

        // 注意：扭矩传感器使用abs()，编码器保留符号
        double encoderSpeed = std::abs(ctx.encoderAngularVelocityRpm());

        if (encoderSpeed < 10.0) continue;

        validSamples++;

        double relativeError = std::abs(torqueSpeed - encoderSpeed) /
                               std::max(torqueSpeed, encoderSpeed);

        if (relativeError < 0.05) {
            passedSamples++;
        }
    }

    double passRate = static_cast<double>(passedSamples) / validSamples;
    QVERIFY2(passRate >= 0.95,
        QString("Speed consistency (reverse) pass rate: %1%").arg(passRate * 100.0).toUtf8());
}

void MockPhysicsValidationTests::testSpeedConsistency_WithBraking_R1() {
    SimulationContext ctx;
    SimulatedTorqueDevice torqueDevice(&ctx);
    SimulatedBrakeDevice brakeDevice(&ctx);

    QVERIFY(torqueDevice.initialize());
    QVERIFY(brakeDevice.initialize());

    // 测试场景3: 制动减速过程
    ctx.setMotorDirection(SimulationContext::MotorDirection::Forward);
    ctx.setMotorDutyCycle(80.0);

    // 先加速到高速
    for (int i = 0; i < 100; i++) {
        ctx.advanceTick();
    }

    // 施加制动
    QVERIFY(brakeDevice.setCurrent(1, 3.0));
    QVERIFY(brakeDevice.setOutputEnable(1, true));

    int validSamples = 0;
    int passedSamples = 0;

    // 监控减速过程中的转速一致性
    for (int i = 0; i < 200; i++) {
        double torqueSpeed;
        QVERIFY(torqueDevice.readSpeed(torqueSpeed));

        double encoderSpeed = std::abs(ctx.encoderAngularVelocityRpm());

        if (encoderSpeed < 10.0) continue;

        validSamples++;

        double relativeError = std::abs(torqueSpeed - encoderSpeed) /
                               std::max(torqueSpeed, encoderSpeed);

        if (relativeError < 0.05) {
            passedSamples++;
        }
    }

    double passRate = static_cast<double>(passedSamples) / validSamples;
    QVERIFY2(passRate >= 0.95,
        QString("Speed consistency during braking pass rate: %1%").arg(passRate * 100.0).toUtf8());
}

// ============================================================================
// 综合测试：多物理规律联合验证
// ============================================================================

void MockPhysicsValidationTests::testCombinedPhysicsValidation() {
    SimulationContext ctx;
    SimulatedTorqueDevice torqueDevice(&ctx);
    SimulatedBrakeDevice brakeDevice(&ctx);

    QVERIFY(torqueDevice.initialize());
    QVERIFY(brakeDevice.initialize());

    // 设置测试工况
    ctx.setMotorDirection(SimulationContext::MotorDirection::Forward);
    ctx.setMotorDutyCycle(70.0);
    QVERIFY(brakeDevice.setCurrent(1, 2.5));
    QVERIFY(brakeDevice.setVoltage(1, 18.0));
    QVERIFY(brakeDevice.setOutputEnable(1, true));

    // 等待稳定
    for (int i = 0; i < 100; i++) {
        ctx.advanceTick();
    }

    // 同时验证R1、R4、R7
    int r1_passed = 0, r4_passed = 0, r7_passed = 0;
    int validSamples = 0;

    for (int i = 0; i < 100; i++) {
        // R1: 转速一致性
        double torqueSpeed;
        QVERIFY(torqueDevice.readSpeed(torqueSpeed));
        double encoderSpeed = std::abs(ctx.encoderAngularVelocityRpm());

        // R4: 功率守恒
        double torque, speed, power;
        QVERIFY(torqueDevice.readAll(torque, speed, power));

        // R7: 制动功率
        double brakeVoltage, brakeCurrent, brakePower;
        QVERIFY(brakeDevice.readVoltage(1, brakeVoltage));
        QVERIFY(brakeDevice.readCurrent(1, brakeCurrent));
        QVERIFY(brakeDevice.readPower(1, brakePower));

        if (encoderSpeed < 10.0 || power < 10.0) continue;

        validSamples++;

        // 检查R1
        double speedError = std::abs(torqueSpeed - encoderSpeed) /
                            std::max(torqueSpeed, encoderSpeed);
        if (speedError < 0.05) r1_passed++;

        // 检查R4
        double theoreticalPower = torque * speed * 2.0 * M_PI / 60.0;
        double powerError = std::abs(power - theoreticalPower) / theoreticalPower;
        if (powerError < 0.10) r4_passed++;

        // 检查R7
        double theoreticalBrakePower = brakeVoltage * brakeCurrent;
        double brakeError = std::abs(brakePower - theoreticalBrakePower);
        if (brakeError < 0.5) r7_passed++;
    }

    double r1_rate = static_cast<double>(r1_passed) / validSamples;
    double r4_rate = static_cast<double>(r4_passed) / validSamples;
    double r7_rate = static_cast<double>(r7_passed) / validSamples;

    QVERIFY2(r1_rate >= 0.95,
        QString("R1 (Speed Consistency) pass rate: %1%").arg(r1_rate * 100.0).toUtf8());
    QVERIFY2(r4_rate >= 0.95,
        QString("R4 (Power Conservation) pass rate: %1%").arg(r4_rate * 100.0).toUtf8());
    QVERIFY2(r7_rate >= 0.95,
        QString("R7 (Brake Power) pass rate: %1%").arg(r7_rate * 100.0).toUtf8());
}

QTEST_MAIN(MockPhysicsValidationTests)
#include "MockPhysicsValidationTests.moc"
