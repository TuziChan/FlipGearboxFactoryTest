#include <QtTest>
#include <QSignalSpy>
#include "tests/mocks/MockDevices.h"

using namespace Tests::Mocks;

/**
 * @brief Test suite for magnet detection mock logic
 *
 * Tests the enhanced MockMotorDevice and MockEncoderDevice
 * to verify magnet detection behavior with state tracking.
 */
class MagnetDetectionMockTests : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic magnet detection tests
    void testSingleMagnetDetection();
    void testMultipleMagnetDetection();
    void testMagnetDetectionWithAngleSimulation();

    // State tracking tests
    void testMagnetPassCountTracking();
    void testSkipFirstMagnetWhenMovingToSecond();
    void testReverseDirectionMagnetDetection();

    // Boundary condition tests
    void testMagnetDetectionNearZeroDegree();
    void testMagnetDetectionAcross360Boundary();
    void testMagnetDetectionWindow();

    // Edge case tests
    void testNoMagnetDetectionWhenDisabled();
    void testMagnetStateReset();

private:
    MockMotorDevice* m_motor;
    MockEncoderDevice* m_encoder;
};

void MagnetDetectionMockTests::initTestCase() {
    qDebug() << "=== Magnet Detection Mock Tests ===";
}

void MagnetDetectionMockTests::cleanupTestCase() {
    qDebug() << "=== Tests Complete ===";
}

void MagnetDetectionMockTests::init() {
    m_motor = new MockMotorDevice(this);
    m_encoder = new MockEncoderDevice(this);

    m_motor->initialize();
    m_encoder->initialize();
}

void MagnetDetectionMockTests::cleanup() {
    delete m_motor;
    delete m_encoder;
}

void MagnetDetectionMockTests::testSingleMagnetDetection() {
    // Setup: Single magnet at 3°
    m_motor->setMagnetPositions({3.0});
    m_motor->linkEncoderAngle(&m_encoder->mockAngleDeg);
    m_motor->setMagnetDetectionEnabled(true);

    // Start away from magnet
    m_encoder->setAngle(0.0);
    bool level;
    m_motor->readAI1Level(level);
    QVERIFY(level == true);  // Should be high initially

    // Move to magnet position
    m_encoder->setAngle(3.0);
    m_motor->readAI1Level(level);
    QVERIFY(level == false);  // Should detect magnet (falling edge)

    // Move away from magnet
    m_encoder->setAngle(5.0);
    m_motor->readAI1Level(level);
    QVERIFY(level == true);  // Should return to high

    // Verify pass count
    QCOMPARE(m_motor->getMagnetPassCount(0), 1);
}

void MagnetDetectionMockTests::testMultipleMagnetDetection() {
    // Setup: Three magnets at 3°, 49°, 113°
    m_motor->setMagnetPositions({3.0, 49.0, 113.0});
    m_motor->linkEncoderAngle(&m_encoder->mockAngleDeg);
    m_motor->setMagnetDetectionEnabled(true);

    bool level;

    // Detect first magnet at 3°
    m_encoder->setAngle(3.0);
    m_motor->readAI1Level(level);
    QVERIFY(level == false);
    QCOMPARE(m_motor->getMagnetPassCount(0), 1);

    // Move away
    m_encoder->setAngle(10.0);
    m_motor->readAI1Level(level);
    QVERIFY(level == true);

    // Detect second magnet at 49°
    m_encoder->setAngle(49.0);
    m_motor->readAI1Level(level);
    QVERIFY(level == false);
    QCOMPARE(m_motor->getMagnetPassCount(1), 1);

    // Move away
    m_encoder->setAngle(60.0);
    m_motor->readAI1Level(level);
    QVERIFY(level == true);

    // Detect third magnet at 113°
    m_encoder->setAngle(113.0);
    m_motor->readAI1Level(level);
    QVERIFY(level == false);
    QCOMPARE(m_motor->getMagnetPassCount(2), 1);
}

void MagnetDetectionMockTests::testMagnetDetectionWithAngleSimulation() {
    // Setup: Magnets at 3°, 49°, 113°
    m_motor->setMagnetPositions({3.0, 49.0, 113.0});
    m_motor->linkEncoderAngle(&m_encoder->mockAngleDeg);
    m_motor->setMagnetDetectionEnabled(true);

    // Start angle simulation from 0° to 120° (should pass all three magnets)
    m_encoder->startAngleSimulation(0.0, 120.0, 0.5, true);

    bool level;
    int magnetDetectionCount = 0;
    bool lastLevel = true;

    // Simulate 300 ticks (should cover 0° to 120° at 0.5°/tick)
    for (int tick = 0; tick < 300; ++tick) {
        double angle;
        m_encoder->readAngle(angle);
        m_motor->readAI1Level(level);

        // Detect falling edge
        if (lastLevel && !level) {
            magnetDetectionCount++;
            qDebug() << "Magnet detected at tick" << tick << "angle" << angle;
        }

        lastLevel = level;

        if (!m_encoder->enableAngleSimulation) {
            break;  // Simulation complete
        }
    }

    // Should have detected all 3 magnets
    QCOMPARE(magnetDetectionCount, 3);
    QCOMPARE(m_motor->getMagnetPassCount(0), 1);  // 3° magnet
    QCOMPARE(m_motor->getMagnetPassCount(1), 1);  // 49° magnet
    QCOMPARE(m_motor->getMagnetPassCount(2), 1);  // 113° magnet
}

void MagnetDetectionMockTests::testMagnetPassCountTracking() {
    // Setup: Single magnet at 3°
    m_motor->setMagnetPositions({3.0});
    m_motor->linkEncoderAngle(&m_encoder->mockAngleDeg);
    m_motor->setMagnetDetectionEnabled(true);

    bool level;

    // First pass
    m_encoder->setAngle(0.0);
    m_motor->readAI1Level(level);
    m_encoder->setAngle(3.0);
    m_motor->readAI1Level(level);
    QCOMPARE(m_motor->getMagnetPassCount(0), 1);

    // Move away and return for second pass
    m_encoder->setAngle(10.0);
    m_motor->readAI1Level(level);
    m_encoder->setAngle(3.0);
    m_motor->readAI1Level(level);
    QCOMPARE(m_motor->getMagnetPassCount(0), 2);

    // Third pass
    m_encoder->setAngle(10.0);
    m_motor->readAI1Level(level);
    m_encoder->setAngle(3.0);
    m_motor->readAI1Level(level);
    QCOMPARE(m_motor->getMagnetPassCount(0), 3);
}

void MagnetDetectionMockTests::testSkipFirstMagnetWhenMovingToSecond() {
    // This tests the key requirement: when moving to position 2 (49°),
    // the system should detect the magnet at 49° but not re-trigger on 3°

    m_motor->setMagnetPositions({3.0, 49.0, 113.0});
    m_motor->linkEncoderAngle(&m_encoder->mockAngleDeg);
    m_motor->setMagnetDetectionEnabled(true);

    bool level;
    QVector<double> detectedAngles;
    bool lastLevel = true;

    // Simulate moving from 0° to 50° (should pass 3° and 49°)
    for (double angle = 0.0; angle <= 50.0; angle += 0.5) {
        m_encoder->setAngle(angle);
        m_motor->readAI1Level(level);

        // Detect falling edge
        if (lastLevel && !level) {
            detectedAngles.append(angle);
        }

        lastLevel = level;
    }

    // Should detect both magnets
    QCOMPARE(detectedAngles.size(), 2);
    QVERIFY(qAbs(detectedAngles[0] - 3.0) < 1.0);   // First detection near 3°
    QVERIFY(qAbs(detectedAngles[1] - 49.0) < 1.0);  // Second detection near 49°

    // Both magnets should have pass count = 1
    QCOMPARE(m_motor->getMagnetPassCount(0), 1);
    QCOMPARE(m_motor->getMagnetPassCount(1), 1);
}

void MagnetDetectionMockTests::testReverseDirectionMagnetDetection() {
    // Test reverse direction: moving from 120° back to 0°
    m_motor->setMagnetPositions({3.0, 49.0, 113.0});
    m_motor->linkEncoderAngle(&m_encoder->mockAngleDeg);
    m_motor->setMagnetDetectionEnabled(true);

    bool level;
    QVector<double> detectedAngles;
    bool lastLevel = true;

    // Start at 120° and move backward to 0°
    for (double angle = 120.0; angle >= 0.0; angle -= 0.5) {
        m_encoder->setAngle(angle);
        m_motor->readAI1Level(level);

        // Detect falling edge
        if (lastLevel && !level) {
            detectedAngles.append(angle);
        }

        lastLevel = level;
    }

    // Should detect all 3 magnets in reverse order
    QCOMPARE(detectedAngles.size(), 3);
    QVERIFY(qAbs(detectedAngles[0] - 113.0) < 1.0);  // First: 113°
    QVERIFY(qAbs(detectedAngles[1] - 49.0) < 1.0);   // Second: 49°
    QVERIFY(qAbs(detectedAngles[2] - 3.0) < 1.0);    // Third: 3°
}

void MagnetDetectionMockTests::testMagnetDetectionNearZeroDegree() {
    // Test magnet at 3° (very close to 0°)
    m_motor->setMagnetPositions({3.0});
    m_motor->linkEncoderAngle(&m_encoder->mockAngleDeg);
    m_motor->setMagnetDetectionEnabled(true);

    bool level;

    // Approach from below
    m_encoder->setAngle(0.0);
    m_motor->readAI1Level(level);
    QVERIFY(level == true);

    m_encoder->setAngle(2.5);
    m_motor->readAI1Level(level);
    QVERIFY(level == false);  // Within detection window

    m_encoder->setAngle(3.0);
    m_motor->readAI1Level(level);
    QVERIFY(level == false);  // At magnet

    m_encoder->setAngle(3.5);
    m_motor->readAI1Level(level);
    QVERIFY(level == false);  // Still within window

    m_encoder->setAngle(4.0);
    m_motor->readAI1Level(level);
    QVERIFY(level == true);  // Outside window
}

void MagnetDetectionMockTests::testMagnetDetectionAcross360Boundary() {
    // Test magnet at 358° (near 360°/0° boundary)
    m_motor->setMagnetPositions({358.0});
    m_motor->linkEncoderAngle(&m_encoder->mockAngleDeg);
    m_motor->setMagnetDetectionEnabled(true);

    bool level;

    // Approach from 355°
    m_encoder->setAngle(355.0);
    m_motor->readAI1Level(level);
    QVERIFY(level == true);

    m_encoder->setAngle(358.0);
    m_motor->readAI1Level(level);
    QVERIFY(level == false);  // At magnet

    // Cross boundary to 0°
    m_encoder->setAngle(0.0);
    m_motor->readAI1Level(level);
    QVERIFY(level == true);  // Should be outside detection window

    // Approach from other side (359°)
    m_encoder->setAngle(359.0);
    m_motor->readAI1Level(level);
    QVERIFY(level == false);  // Within window (359° is 1° away from 358°)
}

void MagnetDetectionMockTests::testMagnetDetectionWindow() {
    // Test detection window size (default 0.5°)
    m_motor->setMagnetPositions({50.0});
    m_motor->magnetDetectionWindowDeg = 0.5;
    m_motor->linkEncoderAngle(&m_encoder->mockAngleDeg);
    m_motor->setMagnetDetectionEnabled(true);

    bool level;

    // Just outside window (50.6° away)
    m_encoder->setAngle(49.4);
    m_motor->readAI1Level(level);
    QVERIFY(level == true);

    // Just inside window (0.5° away)
    m_encoder->setAngle(49.5);
    m_motor->readAI1Level(level);
    QVERIFY(level == false);

    // At magnet
    m_encoder->setAngle(50.0);
    m_motor->readAI1Level(level);
    QVERIFY(level == false);

    // Just inside window (0.5° away)
    m_encoder->setAngle(50.5);
    m_motor->readAI1Level(level);
    QVERIFY(level == false);

    // Just outside window
    m_encoder->setAngle(50.6);
    m_motor->readAI1Level(level);
    QVERIFY(level == true);
}

void MagnetDetectionMockTests::testNoMagnetDetectionWhenDisabled() {
    m_motor->setMagnetPositions({3.0, 49.0, 113.0});
    m_motor->linkEncoderAngle(&m_encoder->mockAngleDeg);
    m_motor->setMagnetDetectionEnabled(false);  // Disabled

    bool level;

    // Move through all magnet positions
    m_encoder->setAngle(3.0);
    m_motor->readAI1Level(level);
    QVERIFY(level == true);  // Should remain high

    m_encoder->setAngle(49.0);
    m_motor->readAI1Level(level);
    QVERIFY(level == true);

    m_encoder->setAngle(113.0);
    m_motor->readAI1Level(level);
    QVERIFY(level == true);

    // No magnets should be detected
    QCOMPARE(m_motor->getMagnetPassCount(0), 0);
    QCOMPARE(m_motor->getMagnetPassCount(1), 0);
    QCOMPARE(m_motor->getMagnetPassCount(2), 0);
}

void MagnetDetectionMockTests::testMagnetStateReset() {
    m_motor->setMagnetPositions({3.0, 49.0});
    m_motor->linkEncoderAngle(&m_encoder->mockAngleDeg);
    m_motor->setMagnetDetectionEnabled(true);

    bool level;

    // Detect first magnet
    m_encoder->setAngle(3.0);
    m_motor->readAI1Level(level);
    QCOMPARE(m_motor->getMagnetPassCount(0), 1);

    // Reset states
    m_motor->resetMagnetStates();
    QCOMPARE(m_motor->getMagnetPassCount(0), 0);
    QCOMPARE(m_motor->getMagnetPassCount(1), 0);

    // Detect again after reset
    m_encoder->setAngle(10.0);
    m_motor->readAI1Level(level);
    m_encoder->setAngle(3.0);
    m_motor->readAI1Level(level);
    QCOMPARE(m_motor->getMagnetPassCount(0), 1);  // Should be 1 again
}

QTEST_MAIN(MagnetDetectionMockTests)
#include "MagnetDetectionMockTests.moc"
