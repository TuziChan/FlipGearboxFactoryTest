#include <QtTest>
#include "src/infrastructure/simulation/MockMotorDevice.h"
#include "src/infrastructure/simulation/MockEncoderDevice.h"

using namespace Infrastructure::Simulation;

/**
 * @brief Test suite for MockMotorDevice automatic magnet detection
 * 
 * Verifies the enhanced magnet detection automation features:
 * - Automatic AI1 level updates based on encoder angle
 * - Configurable magnet positions and detection window
 * - Magnet pass counting
 * - Integration with encoder angle simulation
 */
class MockMotorMagnetDetectionTests : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic automatic detection tests
    void testAutomaticSingleMagnetDetection();
    void testAutomaticMultipleMagnetDetection();
    void testMagnetDetectionWithEncoderSimulation();

    // Configuration tests
    void testCustomMagnetPositions();
    void testCustomDetectionWindow();
    void testEnableMagnetDetection();

    // State tracking tests
    void testMagnetPassCounting();
    void testMagnetStateReset();

    // Boundary condition tests
    void testMagnetDetectionNearZeroDegree();
    void testMagnetDetectionAcross360Boundary();

    // Integration tests
    void testMagnetDetectionWithMotorMovement();
    void testReverseDirectionMagnetDetection();

    // Edge cases
    void testNoDetectionWhenDisabled();
    void testNoDetectionWithoutLinkedEncoder();
    void testManualAI1ControlWhenDetectionDisabled();

private:
    MockMotorDevice* m_motor;
    MockEncoderDevice* m_encoder;
    double m_testAngle;
};

void MockMotorMagnetDetectionTests::initTestCase() {
    qDebug() << "=== MockMotorDevice Magnet Detection Tests ===";
}

void MockMotorMagnetDetectionTests::cleanupTestCase() {
    qDebug() << "=== Tests Complete ===";
}

void MockMotorMagnetDetectionTests::init() {
    m_motor = new MockMotorDevice(1, this);
    m_encoder = new MockEncoderDevice(3, 4096, this);
    m_testAngle = 0.0;
}

void MockMotorMagnetDetectionTests::cleanup() {
    delete m_motor;
    delete m_encoder;
}

void MockMotorMagnetDetectionTests::testAutomaticSingleMagnetDetection() {
    // Setup: Single magnet at 3°
    m_motor->setMagnetPositions({3.0});
    m_motor->linkEncoderAngle(&m_testAngle);
    m_motor->setMagnetDetectionEnabled(true);

    // Start away from magnet
    m_testAngle = 0.0;
    QByteArray request = QByteArray::fromHex("01 03 0052 0001");  // Read AI1 level
    QByteArray response = m_motor->processRequest(request);
    QVERIFY(!response.isEmpty());
    
    // Extract AI1 level from response (should be high initially)
    uint16_t ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 1);  // High (no magnet)

    // Move to magnet position
    m_testAngle = 3.0;
    response = m_motor->processRequest(request);
    ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 0);  // Low (magnet detected)

    // Verify pass count
    QCOMPARE(m_motor->getMagnetPassCount(0), 1);

    // Move away from magnet
    m_testAngle = 5.0;
    response = m_motor->processRequest(request);
    ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 1);  // High (no magnet)
}

void MockMotorMagnetDetectionTests::testAutomaticMultipleMagnetDetection() {
    // Setup: Three magnets at default positions
    m_motor->setMagnetPositions({3.0, 49.0, 113.5});
    m_motor->linkEncoderAngle(&m_testAngle);
    m_motor->setMagnetDetectionEnabled(true);

    QByteArray request = QByteArray::fromHex("01 03 0052 0001");

    // Detect first magnet at 3°
    m_testAngle = 3.0;
    QByteArray response = m_motor->processRequest(request);
    uint16_t ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 0);
    QCOMPARE(m_motor->getMagnetPassCount(0), 1);

    // Move away
    m_testAngle = 10.0;
    response = m_motor->processRequest(request);
    ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 1);

    // Detect second magnet at 49°
    m_testAngle = 49.0;
    response = m_motor->processRequest(request);
    ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 0);
    QCOMPARE(m_motor->getMagnetPassCount(1), 1);

    // Move away
    m_testAngle = 60.0;
    response = m_motor->processRequest(request);
    ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 1);

    // Detect third magnet at 113.5°
    m_testAngle = 113.5;
    response = m_motor->processRequest(request);
    ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 0);
    QCOMPARE(m_motor->getMagnetPassCount(2), 1);
}

void MockMotorMagnetDetectionTests::testMagnetDetectionWithEncoderSimulation() {
    // Setup magnets
    m_motor->setMagnetPositions({3.0, 49.0, 113.5});
    m_motor->linkEncoderAngle(&m_testAngle);
    m_motor->setMagnetDetectionEnabled(true);

    QByteArray request = QByteArray::fromHex("01 03 0052 0001");
    int magnetDetectionCount = 0;
    bool lastLevel = true;

    // Simulate movement from 0° to 120° in 0.5° steps
    for (double angle = 0.0; angle <= 120.0; angle += 0.5) {
        m_testAngle = angle;
        QByteArray response = m_motor->processRequest(request);
        uint16_t ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
        bool currentLevel = (ai1Level == 1);

        // Detect falling edge (high -> low)
        if (lastLevel && !currentLevel) {
            magnetDetectionCount++;
        }

        lastLevel = currentLevel;
    }

    // Should have detected all 3 magnets
    QCOMPARE(magnetDetectionCount, 3);
    QCOMPARE(m_motor->getMagnetPassCount(0), 1);
    QCOMPARE(m_motor->getMagnetPassCount(1), 1);
    QCOMPARE(m_motor->getMagnetPassCount(2), 1);
}

void MockMotorMagnetDetectionTests::testCustomMagnetPositions() {
    // Setup custom magnet positions
    QVector<double> customPositions = {10.0, 90.0, 180.0, 270.0};
    m_motor->setMagnetPositions(customPositions);
    m_motor->linkEncoderAngle(&m_testAngle);
    m_motor->setMagnetDetectionEnabled(true);

    QCOMPARE(m_motor->getMagnetPositions(), customPositions);

    QByteArray request = QByteArray::fromHex("01 03 0052 0001");

    // Test detection at each custom position
    for (int i = 0; i < customPositions.size(); ++i) {
        m_testAngle = customPositions[i];
        QByteArray response = m_motor->processRequest(request);
        uint16_t ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
        QCOMPARE(ai1Level, 0);  // Should detect magnet
        QCOMPARE(m_motor->getMagnetPassCount(i), 1);

        // Move away
        m_testAngle = customPositions[i] + 5.0;
        response = m_motor->processRequest(request);
        ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
        QCOMPARE(ai1Level, 1);  // Should not detect
    }
}

void MockMotorMagnetDetectionTests::testCustomDetectionWindow() {
    // Setup with larger detection window
    m_motor->setMagnetPositions({50.0});
    m_motor->setDetectionWindow(2.0);  // 2° window
    m_motor->linkEncoderAngle(&m_testAngle);
    m_motor->setMagnetDetectionEnabled(true);

    QByteArray request = QByteArray::fromHex("01 03 0052 0001");

    // Test at 48° (2° away, should detect)
    m_testAngle = 48.0;
    QByteArray response = m_motor->processRequest(request);
    uint16_t ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 0);

    // Test at 47° (3° away, should not detect)
    m_testAngle = 47.0;
    response = m_motor->processRequest(request);
    ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 1);

    // Test at 52° (2° away, should detect)
    m_testAngle = 52.0;
    response = m_motor->processRequest(request);
    ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 0);
}

void MockMotorMagnetDetectionTests::testEnableMagnetDetection() {
    m_motor->setMagnetPositions({3.0});
    m_motor->linkEncoderAngle(&m_testAngle);

    // Initially disabled
    QVERIFY(!m_motor->isMagnetDetectionEnabled());

    // Enable detection
    m_motor->setMagnetDetectionEnabled(true);
    QVERIFY(m_motor->isMagnetDetectionEnabled());

    // Verify detection works
    m_testAngle = 3.0;
    QByteArray request = QByteArray::fromHex("01 03 0052 0001");
    QByteArray response = m_motor->processRequest(request);
    uint16_t ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 0);

    // Disable detection
    m_motor->setMagnetDetectionEnabled(false);
    QVERIFY(!m_motor->isMagnetDetectionEnabled());
}

void MockMotorMagnetDetectionTests::testMagnetPassCounting() {
    m_motor->setMagnetPositions({3.0});
    m_motor->linkEncoderAngle(&m_testAngle);
    m_motor->setMagnetDetectionEnabled(true);

    QByteArray request = QByteArray::fromHex("01 03 0052 0001");

    // First pass
    m_testAngle = 0.0;
    m_motor->processRequest(request);
    m_testAngle = 3.0;
    m_motor->processRequest(request);
    QCOMPARE(m_motor->getMagnetPassCount(0), 1);

    // Second pass
    m_testAngle = 10.0;
    m_motor->processRequest(request);
    m_testAngle = 3.0;
    m_motor->processRequest(request);
    QCOMPARE(m_motor->getMagnetPassCount(0), 2);

    // Third pass
    m_testAngle = 10.0;
    m_motor->processRequest(request);
    m_testAngle = 3.0;
    m_motor->processRequest(request);
    QCOMPARE(m_motor->getMagnetPassCount(0), 3);
}

void MockMotorMagnetDetectionTests::testMagnetStateReset() {
    m_motor->setMagnetPositions({3.0, 49.0});
    m_motor->linkEncoderAngle(&m_testAngle);
    m_motor->setMagnetDetectionEnabled(true);

    QByteArray request = QByteArray::fromHex("01 03 0052 0001");

    // Detect first magnet
    m_testAngle = 3.0;
    m_motor->processRequest(request);
    QCOMPARE(m_motor->getMagnetPassCount(0), 1);

    // Reset states
    m_motor->resetMagnetDetection();
    QCOMPARE(m_motor->getMagnetPassCount(0), 0);
    QCOMPARE(m_motor->getMagnetPassCount(1), 0);

    // Detect again after reset
    m_testAngle = 10.0;
    m_motor->processRequest(request);
    m_testAngle = 3.0;
    m_motor->processRequest(request);
    QCOMPARE(m_motor->getMagnetPassCount(0), 1);
}

void MockMotorMagnetDetectionTests::testMagnetDetectionNearZeroDegree() {
    m_motor->setMagnetPositions({3.0});
    m_motor->linkEncoderAngle(&m_testAngle);
    m_motor->setMagnetDetectionEnabled(true);

    QByteArray request = QByteArray::fromHex("01 03 0052 0001");

    // Test at 0° (should not detect)
    m_testAngle = 0.0;
    QByteArray response = m_motor->processRequest(request);
    uint16_t ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 1);

    // Test at 2.5° (0.5° away, should detect)
    m_testAngle = 2.5;
    response = m_motor->processRequest(request);
    ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 0);

    // Test at 3.5° (0.5° away, should detect)
    m_testAngle = 3.5;
    response = m_motor->processRequest(request);
    ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 0);

    // Test at 4.0° (1° away, should not detect)
    m_testAngle = 4.0;
    response = m_motor->processRequest(request);
    ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 1);
}

void MockMotorMagnetDetectionTests::testMagnetDetectionAcross360Boundary() {
    m_motor->setMagnetPositions({358.0});
    m_motor->linkEncoderAngle(&m_testAngle);
    m_motor->setMagnetDetectionEnabled(true);

    QByteArray request = QByteArray::fromHex("01 03 0052 0001");

    // Test at 357.5° (0.5° away, should detect)
    m_testAngle = 357.5;
    QByteArray response = m_motor->processRequest(request);
    uint16_t ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 0);

    // Test at 358.5° (0.5° away, should detect)
    m_testAngle = 358.5;
    response = m_motor->processRequest(request);
    ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 0);

    // Test at 0° (2° away, should not detect)
    m_testAngle = 0.0;
    response = m_motor->processRequest(request);
    ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 1);
}

void MockMotorMagnetDetectionTests::testMagnetDetectionWithMotorMovement() {
    m_motor->setMagnetPositions({3.0, 49.0, 113.5});
    m_motor->linkEncoderAngle(&m_testAngle);
    m_motor->setMagnetDetectionEnabled(true);

    // Set motor speed
    QByteArray setSpeedRequest = QByteArray::fromHex("01 06 0040 01F4");  // Set speed to 500
    m_motor->processRequest(setSpeedRequest);

    // Verify motor is running
    QVERIFY(m_motor->getMotorSpeed() == 500);

    // Simulate movement and detection
    QByteArray readAI1Request = QByteArray::fromHex("01 03 0052 0001");
    int detectionCount = 0;
    bool lastLevel = true;

    for (double angle = 0.0; angle <= 120.0; angle += 1.0) {
        m_testAngle = angle;
        QByteArray response = m_motor->processRequest(readAI1Request);
        uint16_t ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
        bool currentLevel = (ai1Level == 1);

        if (lastLevel && !currentLevel) {
            detectionCount++;
        }

        lastLevel = currentLevel;
    }

    QCOMPARE(detectionCount, 3);
}

void MockMotorMagnetDetectionTests::testReverseDirectionMagnetDetection() {
    m_motor->setMagnetPositions({3.0, 49.0, 113.5});
    m_motor->linkEncoderAngle(&m_testAngle);
    m_motor->setMagnetDetectionEnabled(true);

    QByteArray request = QByteArray::fromHex("01 03 0052 0001");
    QVector<double> detectedAngles;
    bool lastLevel = true;

    // Move backward from 120° to 0°
    for (double angle = 120.0; angle >= 0.0; angle -= 0.5) {
        m_testAngle = angle;
        QByteArray response = m_motor->processRequest(request);
        uint16_t ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
        bool currentLevel = (ai1Level == 1);

        if (lastLevel && !currentLevel) {
            detectedAngles.append(angle);
        }

        lastLevel = currentLevel;
    }

    // Should detect all 3 magnets in reverse order
    QCOMPARE(detectedAngles.size(), 3);
    QVERIFY(qAbs(detectedAngles[0] - 113.5) < 1.0);
    QVERIFY(qAbs(detectedAngles[1] - 49.0) < 1.0);
    QVERIFY(qAbs(detectedAngles[2] - 3.0) < 1.0);
}

void MockMotorMagnetDetectionTests::testNoDetectionWhenDisabled() {
    m_motor->setMagnetPositions({3.0, 49.0, 113.5});
    m_motor->linkEncoderAngle(&m_testAngle);
    m_motor->setMagnetDetectionEnabled(false);  // Disabled

    QByteArray request = QByteArray::fromHex("01 03 0052 0001");

    // Move through all magnet positions
    m_testAngle = 3.0;
    QByteArray response = m_motor->processRequest(request);
    uint16_t ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 0);  // Should remain at initial level (low when disabled)

    m_testAngle = 49.0;
    response = m_motor->processRequest(request);
    ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 0);

    // No magnets should be detected
    QCOMPARE(m_motor->getMagnetPassCount(0), 0);
    QCOMPARE(m_motor->getMagnetPassCount(1), 0);
    QCOMPARE(m_motor->getMagnetPassCount(2), 0);
}

void MockMotorMagnetDetectionTests::testNoDetectionWithoutLinkedEncoder() {
    m_motor->setMagnetPositions({3.0});
    // Don't link encoder angle
    m_motor->setMagnetDetectionEnabled(true);

    QByteArray request = QByteArray::fromHex("01 03 0052 0001");

    // Even at magnet position, should not detect without linked encoder
    m_testAngle = 3.0;
    QByteArray response = m_motor->processRequest(request);
    uint16_t ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 1);  // Should remain high

    QCOMPARE(m_motor->getMagnetPassCount(0), 0);
}

void MockMotorMagnetDetectionTests::testManualAI1ControlWhenDetectionDisabled() {
    m_motor->setMagnetDetectionEnabled(false);

    // Manual control should work
    m_motor->setAI1InputLevel(true);
    QByteArray request = QByteArray::fromHex("01 03 0052 0001");
    QByteArray response = m_motor->processRequest(request);
    uint16_t ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 1);

    m_motor->setAI1InputLevel(false);
    response = m_motor->processRequest(request);
    ai1Level = (static_cast<uint8_t>(response[3]) << 8) | static_cast<uint8_t>(response[4]);
    QCOMPARE(ai1Level, 0);
}

QTEST_MAIN(MockMotorMagnetDetectionTests)
#include "MockMotorMagnetDetectionTests.moc"
