#include <QtTest>
#include <QSignalSpy>
#include "tests/mocks/MockDevices.h"
#include "src/domain/GearboxTestEngine.h"
#include "src/domain/TestRecipe.h"

using namespace Tests::Mocks;
using namespace Domain;

/**
 * @brief Integration tests for angle test with magnet detection
 *
 * Simulates the complete angle positioning phase with realistic
 * magnet detection behavior at 3°, 49°, and 113.5°.
 */
class AngleTestMagnetIntegrationTests : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Integration tests
    void testCompleteAngleTestSequence();
    void testAngleTestWithForwardAndReverse();
    void testMagnetDetectionDuringContinuousRotation();
    void testAngleTestBoundaryConditions();
    void testMultiplePassesThroughSameMagnet();

private:
    MockMotorDevice* m_motor;
    MockEncoderDevice* m_encoder;
    MockTorqueDevice* m_torque;
    MockBrakeDevice* m_brake;

    void simulateAngleMovement(double startAngle, double endAngle, double stepDeg);
};

void AngleTestMagnetIntegrationTests::initTestCase() {
    qDebug() << "=== Angle Test Magnet Integration Tests ===";
}

void AngleTestMagnetIntegrationTests::cleanupTestCase() {
    qDebug() << "=== Integration Tests Complete ===";
}

void AngleTestMagnetIntegrationTests::init() {
    m_motor = new MockMotorDevice(this);
    m_encoder = new MockEncoderDevice(this);
    m_torque = new MockTorqueDevice(this);
    m_brake = new MockBrakeDevice(this);

    m_motor->initialize();
    m_encoder->initialize();
    m_torque->initialize();
    m_brake->initialize();

    // Setup magnet detection
    m_motor->setMagnetPositions({3.0, 49.0, 113.5});
    m_motor->linkEncoderAngle(&m_encoder->mockAngleDeg);
    m_motor->setMagnetDetectionEnabled(true);
}

void AngleTestMagnetIntegrationTests::cleanup() {
    delete m_motor;
    delete m_encoder;
    delete m_torque;
    delete m_brake;
}

void AngleTestMagnetIntegrationTests::testCompleteAngleTestSequence() {
    // Simulate the complete angle test sequence with absolute angle targets:
    // Encoder zero is fixed at installation (~0°)
    // 0° -> Position1(49°) -> Position2(113.5°) -> Position1 Return(49°) -> Position3(113.5°) -> 0°

    QVector<QPair<QString, double>> sequence = {
        {"Start", 0.0},
        {"Position1", 49.0},
        {"Position2", 113.5},
        {"Position1 Return", 49.0},
        {"Position3", 113.5},
        {"Return to Zero", 0.0}
    };

    int totalMagnetDetections = 0;
    bool lastLevel = true;

    for (int i = 0; i < sequence.size() - 1; ++i) {
        double startAngle = sequence[i].second;
        double endAngle = sequence[i + 1].second;
        QString phaseName = sequence[i + 1].first;

        qDebug() << "\n--- Moving to" << phaseName << "---";
        qDebug() << "From" << startAngle << "to" << endAngle;

        bool magnetDetectedInPhase = false;
        double detectionAngle = 0.0;

        // Simulate movement
        double step = (endAngle > startAngle) ? 0.5 : -0.5;
        double currentAngle = startAngle;

        while ((step > 0 && currentAngle <= endAngle) ||
               (step < 0 && currentAngle >= endAngle)) {
            m_encoder->setAngle(currentAngle);

            bool level;
            m_motor->readAI1Level(level);

            // Detect falling edge
            if (lastLevel && !level) {
                magnetDetectedInPhase = true;
                detectionAngle = currentAngle;
                totalMagnetDetections++;
                qDebug() << "  Magnet detected at" << currentAngle << "°";
            }

            lastLevel = level;
            currentAngle += step;
        }

        // Verify magnet detection for expected positions
        if (phaseName == "Position1" || phaseName == "Position1 Return") {
            QVERIFY2(magnetDetectedInPhase, "Should detect magnet at Position1");
            QVERIFY(qAbs(detectionAngle - 49.0) < 1.0);
        } else if (phaseName == "Position2") {
            QVERIFY2(magnetDetectedInPhase, "Should detect magnet at Position2");
            QVERIFY(qAbs(detectionAngle - 113.5) < 1.0);
        } else if (phaseName == "Position3") {
            QVERIFY2(magnetDetectedInPhase, "Should detect magnet at Position3");
            QVERIFY(qAbs(detectionAngle - 113.5) < 1.0);
        }
    }

    // Total detections: Position1 (first) + Position2 + Position1 (return) + Position3 = 4
    QCOMPARE(totalMagnetDetections, 4);

    // Verify pass counts
    QCOMPARE(m_motor->getMagnetPassCount(0), 2);  // 3° magnet passed twice
    QCOMPARE(m_motor->getMagnetPassCount(1), 1);  // 49° magnet passed once
    QCOMPARE(m_motor->getMagnetPassCount(2), 1);  // 113.5° magnet passed once
}

void AngleTestMagnetIntegrationTests::testAngleTestWithForwardAndReverse() {
    // Test forward movement: 0° -> 50° (should detect 3° and 49°)
    qDebug() << "\n--- Forward Movement Test ---";

    QVector<double> forwardDetections;
    bool lastLevel = true;

    for (double angle = 0.0; angle <= 50.0; angle += 0.5) {
        m_encoder->setAngle(angle);
        bool level;
        m_motor->readAI1Level(level);

        if (lastLevel && !level) {
            forwardDetections.append(angle);
            qDebug() << "Forward: Magnet at" << angle << "°";
        }
        lastLevel = level;
    }

    QCOMPARE(forwardDetections.size(), 2);
    QVERIFY(qAbs(forwardDetections[0] - 3.0) < 1.0);
    QVERIFY(qAbs(forwardDetections[1] - 49.0) < 1.0);

    // Reset for reverse test
    m_motor->resetMagnetStates();
    lastLevel = true;

    // Test reverse movement: 50° -> 0° (should detect 49° and 3°)
    qDebug() << "\n--- Reverse Movement Test ---";

    QVector<double> reverseDetections;

    for (double angle = 50.0; angle >= 0.0; angle -= 0.5) {
        m_encoder->setAngle(angle);
        bool level;
        m_motor->readAI1Level(level);

        if (lastLevel && !level) {
            reverseDetections.append(angle);
            qDebug() << "Reverse: Magnet at" << angle << "°";
        }
        lastLevel = level;
    }

    QCOMPARE(reverseDetections.size(), 2);
    QVERIFY(qAbs(reverseDetections[0] - 49.0) < 1.0);
    QVERIFY(qAbs(reverseDetections[1] - 3.0) < 1.0);
}

void AngleTestMagnetIntegrationTests::testMagnetDetectionDuringContinuousRotation() {
    // Simulate continuous rotation through all magnets multiple times
    qDebug() << "\n--- Continuous Rotation Test ---";

    int totalDetections = 0;
    bool lastLevel = true;

    // Rotate from 0° to 360° (full rotation)
    for (double angle = 0.0; angle <= 360.0; angle += 0.5) {
        m_encoder->setAngle(angle);
        bool level;
        m_motor->readAI1Level(level);

        if (lastLevel && !level) {
            totalDetections++;
            qDebug() << "Detected magnet at" << angle << "°";
        }
        lastLevel = level;
    }

    // Should detect all 3 magnets
    QCOMPARE(totalDetections, 3);
    QCOMPARE(m_motor->getMagnetPassCount(0), 1);
    QCOMPARE(m_motor->getMagnetPassCount(1), 1);
    QCOMPARE(m_motor->getMagnetPassCount(2), 1);
}

void AngleTestMagnetIntegrationTests::testAngleTestBoundaryConditions() {
    qDebug() << "\n--- Boundary Conditions Test ---";

    // Test 1: Very slow approach to magnet (0.1° steps)
    m_encoder->setAngle(2.0);
    bool level;
    m_motor->readAI1Level(level);
    QVERIFY(level == true);

    for (double angle = 2.1; angle <= 4.0; angle += 0.1) {
        m_encoder->setAngle(angle);
        m_motor->readAI1Level(level);

        if (angle >= 2.5 && angle <= 3.5) {
            QVERIFY2(level == false,
                     QString("Should detect magnet at %1°").arg(angle).toUtf8());
        }
    }

    // Test 2: Rapid movement through magnet (large steps)
    m_motor->resetMagnetStates();
    m_encoder->setAngle(0.0);
    m_motor->readAI1Level(level);

    m_encoder->setAngle(3.0);  // Jump directly to magnet
    m_motor->readAI1Level(level);
    QVERIFY(level == false);

    // Test 3: Oscillation around magnet position
    m_motor->resetMagnetStates();
    int oscillationDetections = 0;
    bool lastLevelOsc = true;

    QVector<double> oscillationAngles = {2.0, 3.0, 2.5, 3.5, 2.8, 3.2, 4.0};
    for (double angle : oscillationAngles) {
        m_encoder->setAngle(angle);
        m_motor->readAI1Level(level);

        if (lastLevelOsc && !level) {
            oscillationDetections++;
        }
        lastLevelOsc = level;
    }

    // Should detect multiple times due to oscillation
    QVERIFY(oscillationDetections >= 2);
}

void AngleTestMagnetIntegrationTests::testMultiplePassesThroughSameMagnet() {
    qDebug() << "\n--- Multiple Passes Test ---";

    // Pass through 3° magnet 5 times
    for (int pass = 1; pass <= 5; ++pass) {
        // Move away
        m_encoder->setAngle(10.0);
        bool level;
        m_motor->readAI1Level(level);

        // Move to magnet
        m_encoder->setAngle(3.0);
        m_motor->readAI1Level(level);
        QVERIFY(level == false);

        // Verify pass count
        QCOMPARE(m_motor->getMagnetPassCount(0), pass);
        qDebug() << "Pass" << pass << "complete, count =" << m_motor->getMagnetPassCount(0);
    }

    QCOMPARE(m_motor->getMagnetPassCount(0), 5);
}

void AngleTestMagnetIntegrationTests::simulateAngleMovement(
    double startAngle, double endAngle, double stepDeg) {

    double currentAngle = startAngle;
    double step = (endAngle > startAngle) ? qAbs(stepDeg) : -qAbs(stepDeg);

    while ((step > 0 && currentAngle <= endAngle) ||
           (step < 0 && currentAngle >= endAngle)) {
        m_encoder->setAngle(currentAngle);
        currentAngle += step;
    }

    // Ensure we reach the exact end angle
    m_encoder->setAngle(endAngle);
}

QTEST_MAIN(AngleTestMagnetIntegrationTests)
#include "AngleTestMagnetIntegrationTests.moc"
