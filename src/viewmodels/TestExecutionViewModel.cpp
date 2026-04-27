#include "TestExecutionViewModel.h"
#include "../infrastructure/config/RecipeConfig.h"
#include "../infrastructure/config/ConfigLoader.h"
#include "../infrastructure/validation/PhysicsValidator.h"
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QVariantMap>

namespace ViewModels {

TestExecutionViewModel::TestExecutionViewModel(Infrastructure::Config::StationRuntime* runtime,
Infrastructure::Config::RuntimeManager* runtimeManager,
QObject* parent)
: QObject(parent)
, m_runtime(runtime)
, m_runtimeManager(runtimeManager)
, m_currentRecipe(Infrastructure::Config::RecipeConfig::createDefault())
, m_running(false)
, m_serialNumber()
, m_selectedModel("")
, m_backlashCompensationDeg(0.0)
, m_currentPhase("Idle")
, m_statusMessage("Ready")
, m_progressPercent(0)
, m_elapsedMs(0)
, m_motorCurrent(0.0)
, m_motorOnline(false)
, m_speed(0.0)
, m_torque(0.0)
, m_power(0.0)
, m_torqueOnline(false)
, m_angle(0.0)
, m_encoderTotalAngle(0.0)
, m_encoderVelocity(0.0)
, m_encoderOnline(false)
, m_brakeCurrent(0.0)
, m_brakeVoltage(0.0)
, m_brakePower(0.0)
, m_brakeOnline(false)
, m_ai1Level(false)
, m_overallVerdict("Pending")
, m_testPassed(false)
, m_idleForwardResult()
, m_idleReverseResult()
, m_angleResults()
, m_loadForwardResult()
, m_loadReverseResult()
, m_physicsViolations()
, m_physicsViolationStats()
, m_lastTelemetry()
{
if (!m_selectedModel.isEmpty()) {
loadRecipe(m_selectedModel);
}

// Connect to test engine signals
if (m_runtime && m_runtime->testEngine()) {
connectEngine(m_runtime->testEngine());
}

// Connect to runtime recreation signal
if (m_runtimeManager) {
connect(m_runtimeManager, &Infrastructure::Config::RuntimeManager::runtimeRecreated,
this, &TestExecutionViewModel::onRuntimeRecreated);
}
}

void TestExecutionViewModel::setSerialNumber(const QString& sn) {
    if (m_serialNumber != sn) {
        m_serialNumber = sn;
        emit serialNumberChanged();
    }
}

void TestExecutionViewModel::setSelectedModel(const QString& model) {
    if (m_selectedModel != model) {
        m_selectedModel = model;
        emit selectedModelChanged();
    }
}

void TestExecutionViewModel::setBacklashCompensationDeg(double compensationDeg) {
    if (!qFuzzyCompare(m_backlashCompensationDeg + 1.0, compensationDeg + 1.0)) {
        m_backlashCompensationDeg = compensationDeg;
        emit backlashCompensationDegChanged();
    }
}

void TestExecutionViewModel::startTest() {
    if (!m_runtime || !m_runtime->testEngine()) {
        emit errorOccurred("Test engine not available");
        return;
    }

    if (m_serialNumber.isEmpty() || m_serialNumber.length() < 8) {
        emit errorOccurred("Serial number must be at least 8 characters");
        return;
    }

    m_runtime->testEngine()->setRecipe(buildExecutionRecipe());

    if (m_runtime->testEngine()->startTest(m_serialNumber)) {
        m_running = true;
        emit runningChanged();
        qDebug() << "Test started for SN:" << m_serialNumber;
    } else {
        emit errorOccurred("Failed to start test");
    }
}

void TestExecutionViewModel::stopTest() {
    if (m_runtime && m_runtime->testEngine()) {
        m_runtime->testEngine()->emergencyStop();
        m_running = false;
        emit runningChanged();
        qDebug() << "Test stopped";
    }
}

void TestExecutionViewModel::resetTest() {
    if (m_runtime && m_runtime->testEngine()) {
        m_runtime->testEngine()->reset();
        m_running = false;
        m_currentPhase = "Idle";
        m_statusMessage = "Ready";
        m_progressPercent = 0;
        m_elapsedMs = 0;
        m_overallVerdict = "Pending";
        m_testPassed = false;
        m_idleForwardResult.clear();
        m_idleReverseResult.clear();
        m_angleResults.clear();
        m_loadForwardResult.clear();
        m_loadReverseResult.clear();
        clearPhysicsValidation();

        // Reset all telemetry fields
        m_motorCurrent = 0.0;
        m_motorOnline = false;
        m_speed = 0.0;
        m_torque = 0.0;
        m_power = 0.0;
        m_torqueOnline = false;
        m_angle = 0.0;
        m_encoderTotalAngle = 0.0;
        m_encoderVelocity = 0.0;
        m_encoderOnline = false;
        m_brakeCurrent = 0.0;
        m_brakeVoltage = 0.0;
        m_brakePower = 0.0;
        m_brakeOnline = false;
        m_ai1Level = false;

        emit runningChanged();
        emit currentPhaseChanged();
        emit statusMessageChanged();
        emit progressPercentChanged();
        emit elapsedMsChanged();
        emit resultsChanged();
        emit motorTelemetryChanged();
        emit torqueTelemetryChanged();
        emit encoderTelemetryChanged();
        emit brakeTelemetryChanged();
        emit physicsValidationChanged();

        qDebug() << "Test reset";
    }
}

void TestExecutionViewModel::loadRecipe(const QString& recipeName) {
    Infrastructure::Config::ConfigLoader loader;
    Domain::TestRecipe recipe;
    const QString recipePath = recipeFilePathForModel(recipeName);

    if (loader.loadRecipe(recipePath, recipe)) {
        m_currentRecipe = recipe;
    } else {
        m_currentRecipe = Infrastructure::Config::RecipeConfig::createDefault();
        m_currentRecipe.name = recipeName;
        emit errorOccurred(QString("加载配方失败: %1").arg(loader.lastError()));
    }

    if (m_selectedModel != recipeName) {
        m_selectedModel = recipeName;
        emit selectedModelChanged();
    }

    emit recipeNameChanged();
    qDebug() << "Recipe loaded:" << recipeName << "from" << recipePath;
}

void TestExecutionViewModel::disconnectEngine() {
// Disconnect all existing connections
for (auto& conn : m_connections) {
if (conn) {
QObject::disconnect(conn);
}
}
m_connections.clear();
m_connectedEngine = nullptr;
}

void TestExecutionViewModel::connectEngine(Domain::GearboxTestEngine* engine) {
if (!engine) return;

// Disconnect old engine first
disconnectEngine();

// Connect to new engine
m_connections << connect(engine, &Domain::GearboxTestEngine::stateChanged,
this, &TestExecutionViewModel::onEngineStateChanged);
m_connections << connect(engine, &Domain::GearboxTestEngine::testCompleted,
this, &TestExecutionViewModel::onTestCompleted);
m_connections << connect(engine, &Domain::GearboxTestEngine::testFailed,
this, &TestExecutionViewModel::onTestFailed);

m_connectedEngine = engine;
}

void TestExecutionViewModel::updateRuntime(Infrastructure::Config::StationRuntime* newRuntime) {
qDebug() << "TestExecutionViewModel: Updating runtime reference";
m_runtime = newRuntime;

// Reconnect signals to new test engine (automatically disconnects old)
if (m_runtime && m_runtime->testEngine()) {
connectEngine(m_runtime->testEngine());
}
}

void TestExecutionViewModel::onEngineStateChanged(const Domain::TestRunState& state) {
    updateFromState(state);
}

void TestExecutionViewModel::onTestCompleted(const Domain::TestResults& results) {
    m_running = false;
    m_testPassed = results.overallPassed;
    m_overallVerdict = results.overallPassed ? "PASSED" : "FAILED";
    m_impactForwardResult = toVariantMap(results.impactForward);
    m_impactReverseResult = toVariantMap(results.impactReverse);
    m_idleForwardResult = toVariantMap(results.idleForward);
    m_idleReverseResult = toVariantMap(results.idleReverse);
    m_angleResults = toVariantList(results.angleResults);
    m_loadForwardResult = toVariantMap(results.loadForward);
    m_loadReverseResult = toVariantMap(results.loadReverse);

    emit runningChanged();
    emit resultsChanged();

    qDebug() << "Test completed:" << m_overallVerdict;
}

void TestExecutionViewModel::onTestFailed(const Domain::FailureReason& reason) {
    m_running = false;
    m_testPassed = false;
    m_overallVerdict = "FAILED";

    QString category;
    switch (reason.category) {
        case Domain::FailureCategory::None:
            category = "None";
            break;
        case Domain::FailureCategory::Communication:
            category = "Communication";
            break;
        case Domain::FailureCategory::Process:
            category = "Process";
            break;
        case Domain::FailureCategory::Judgment:
            category = "Judgment";
            break;
    }

    m_statusMessage = QString("%1 Failure: %2").arg(category, reason.description);

    emit runningChanged();
    emit resultsChanged();
    emit statusMessageChanged();
    emit errorOccurred(m_statusMessage);

    qDebug() << "Test failed:" << m_statusMessage;
}

void TestExecutionViewModel::updateFromState(const Domain::TestRunState& state) {
    QString newPhase = state.phaseString();
    if (m_currentPhase != newPhase) {
        m_currentPhase = newPhase;
        emit currentPhaseChanged();
    }

    if (m_statusMessage != state.statusMessage) {
        m_statusMessage = state.statusMessage;
        emit statusMessageChanged();
    }

    if (m_progressPercent != state.progressPercent) {
        m_progressPercent = state.progressPercent;
        emit progressPercentChanged();
    }

    if (m_elapsedMs != state.elapsedMs) {
        m_elapsedMs = state.elapsedMs;
        emit elapsedMsChanged();
    }

    // Update physics validation with current telemetry
    updatePhysicsValidation(state.currentTelemetry);

    bool motorTelemetryUpdated = false;
    bool torqueTelemetryUpdated = false;
    bool encoderTelemetryUpdated = false;
    bool brakeTelemetryUpdated = false;

    if (m_motorCurrent != state.currentTelemetry.motorCurrentA) {
        m_motorCurrent = state.currentTelemetry.motorCurrentA;
        motorTelemetryUpdated = true;
    }

    if (m_motorOnline != state.currentTelemetry.motorOnline) {
        m_motorOnline = state.currentTelemetry.motorOnline;
        motorTelemetryUpdated = true;
    }

    if (m_speed != state.currentTelemetry.dynSpeedRpm) {
        m_speed = state.currentTelemetry.dynSpeedRpm;
        torqueTelemetryUpdated = true;
    }

    if (m_torque != state.currentTelemetry.dynTorqueNm) {
        m_torque = state.currentTelemetry.dynTorqueNm;
        torqueTelemetryUpdated = true;
    }

    if (m_power != state.currentTelemetry.dynPowerW) {
        m_power = state.currentTelemetry.dynPowerW;
        torqueTelemetryUpdated = true;
    }

    if (m_torqueOnline != state.currentTelemetry.torqueOnline) {
        m_torqueOnline = state.currentTelemetry.torqueOnline;
        torqueTelemetryUpdated = true;
    }

    if (m_angle != state.currentTelemetry.encoderAngleDeg) {
        m_angle = state.currentTelemetry.encoderAngleDeg;
        encoderTelemetryUpdated = true;
    }

    if (m_encoderTotalAngle != state.currentTelemetry.encoderTotalAngleDeg) {
        m_encoderTotalAngle = state.currentTelemetry.encoderTotalAngleDeg;
        encoderTelemetryUpdated = true;
    }

    if (m_encoderVelocity != state.currentTelemetry.encoderVelocityRpm) {
        m_encoderVelocity = state.currentTelemetry.encoderVelocityRpm;
        encoderTelemetryUpdated = true;
    }

    if (m_encoderOnline != state.currentTelemetry.encoderOnline) {
        m_encoderOnline = state.currentTelemetry.encoderOnline;
        encoderTelemetryUpdated = true;
    }

    if (m_brakeCurrent != state.currentTelemetry.brakeCurrentA) {
        m_brakeCurrent = state.currentTelemetry.brakeCurrentA;
        brakeTelemetryUpdated = true;
    }

    if (m_brakeVoltage != state.currentTelemetry.brakeVoltageV) {
        m_brakeVoltage = state.currentTelemetry.brakeVoltageV;
        brakeTelemetryUpdated = true;
    }

    if (m_brakePower != state.currentTelemetry.brakePowerW) {
        m_brakePower = state.currentTelemetry.brakePowerW;
        brakeTelemetryUpdated = true;
    }

    if (m_brakeOnline != state.currentTelemetry.brakeOnline) {
        m_brakeOnline = state.currentTelemetry.brakeOnline;
        brakeTelemetryUpdated = true;
    }

    if (m_ai1Level != state.currentTelemetry.aqmdAi1Level) {
        m_ai1Level = state.currentTelemetry.aqmdAi1Level;
        motorTelemetryUpdated = true;
    }

    if (motorTelemetryUpdated) {
        emit motorTelemetryChanged();
    }
    if (torqueTelemetryUpdated) {
        emit torqueTelemetryChanged();
    }
    if (encoderTelemetryUpdated) {
        emit encoderTelemetryChanged();
    }
    if (brakeTelemetryUpdated) {
        emit brakeTelemetryChanged();
    }
}

QString TestExecutionViewModel::recipeFilePathForModel(const QString& model) const {
    const QString recipeDir = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../../config/recipes");
    QString recipeFileName = model.trimmed();
    if (!recipeFileName.endsWith(".json", Qt::CaseInsensitive)) {
        recipeFileName += ".json";
    }
    return QDir(recipeDir).absoluteFilePath(recipeFileName);
}

Domain::TestRecipe TestExecutionViewModel::buildExecutionRecipe() const {
    Domain::TestRecipe recipe = m_currentRecipe;
    recipe.name = QString("%1 | 回差补偿 %2°")
                      .arg(m_currentRecipe.name)
                      .arg(QString::number(m_backlashCompensationDeg, 'f', 2));
    recipe.position1TargetDeg += m_backlashCompensationDeg;
    recipe.position2TargetDeg += m_backlashCompensationDeg;
    recipe.position3TargetDeg += m_backlashCompensationDeg;
    recipe.encoderZeroAngleDeg += m_backlashCompensationDeg;
    return recipe;
}

QVariantMap TestExecutionViewModel::toVariantMap(const Domain::IdleRunResult& result) const {
    return {
        {"direction", result.direction},
        {"currentAvg", result.currentAvg},
        {"currentMax", result.currentMax},
        {"speedAvg", result.speedAvg},
        {"speedMax", result.speedMax},
        {"currentAvgPassed", result.currentAvgPassed},
        {"currentMaxPassed", result.currentMaxPassed},
        {"speedAvgPassed", result.speedAvgPassed},
        {"speedMaxPassed", result.speedMaxPassed},
        {"overallPassed", result.overallPassed}
    };
}

QVariantMap TestExecutionViewModel::toVariantMap(const Domain::LoadTestResult& result) const {
    return {
        {"direction", result.direction},
        {"lockCurrentA", result.lockCurrentA},
        {"lockTorqueNm", result.lockTorqueNm},
        {"currentPassed", result.currentPassed},
        {"torquePassed", result.torquePassed},
        {"overallPassed", result.overallPassed},
        {"lockAchieved", result.lockAchieved}
    };
}

void TestExecutionViewModel::onRuntimeRecreated(Infrastructure::Config::StationRuntime* newRuntime) {
updateRuntime(newRuntime);
}

TestExecutionViewModel::~TestExecutionViewModel() {
disconnectEngine();
}

QVariantList TestExecutionViewModel::toVariantList(const QVector<Domain::AngleResult>& results) const {
    QVariantList list;
    for (const auto& r : results) {
        list.append(QVariantMap{
            {"positionName", r.positionName},
            {"targetAngleDeg", r.targetAngleDeg},
            {"measuredAngleDeg", r.measuredAngleDeg},
            {"deviationDeg", r.deviationDeg},
            {"toleranceDeg", r.toleranceDeg},
            {"passed", r.passed}
        });
    }
    return list;
}

void TestExecutionViewModel::updatePhysicsValidation(const Domain::TelemetrySnapshot& current) {
    using namespace Infrastructure::Validation;

    // Skip validation if this is the first telemetry snapshot
    if (!m_lastTelemetry.timestamp.isValid()) {
        m_lastTelemetry = current;
        return;
    }

    // Run physics validation
    PhysicsValidator::ValidationConfig config;
    auto results = PhysicsValidator::validateAll(current, m_lastTelemetry, config);

    // Convert to QVariantList for QML
    QVariantList violations;
    int warningCount = 0;
    int criticalCount = 0;

    for (const auto& result : results) {
        if (!result.passed) {
            QVariantMap violation = toVariantMap(result);
            violations.append(violation);

            // Classify severity based on error percentage
            if (result.errorPercent > 0.20) {
                criticalCount++;
            } else {
                warningCount++;
            }
        }
    }

    // Update member variables
    bool changed = false;
    if (m_physicsViolations != violations) {
        m_physicsViolations = violations;
        changed = true;
    }

    // Update statistics
    QVariantMap stats;
    stats["totalViolations"] = violations.size();
    stats["warningCount"] = warningCount;
    stats["criticalCount"] = criticalCount;
    stats["lastCheckTime"] = current.timestamp;

    if (m_physicsViolationStats != stats) {
        m_physicsViolationStats = stats;
        changed = true;
    }

    // Emit signal if changed
    if (changed) {
        emit physicsValidationChanged();
    }

    // Store current telemetry for next validation
    m_lastTelemetry = current;
}

void TestExecutionViewModel::clearPhysicsValidation() {
    m_physicsViolations.clear();
    m_physicsViolationStats.clear();
    m_lastTelemetry = Domain::TelemetrySnapshot();
    emit physicsValidationChanged();
}

QVariantMap TestExecutionViewModel::toVariantMap(
    const Infrastructure::Validation::PhysicsValidator::ValidationResult& result) const
{
    return {
        {"passed", result.passed},
        {"ruleName", result.ruleName},
        {"message", result.message},
        {"actualValue", result.actualValue},
        {"expectedValue", result.expectedValue},
        {"threshold", result.threshold},
        {"errorPercent", result.errorPercent}
    };
}

QVariantMap TestExecutionViewModel::toVariantMap(const Domain::ImpactCycleResult& result) const {
    return {
        {"cycleNumber", result.cycleNumber},
        {"peakCurrentA", result.peakCurrentA},
        {"peakTorqueNm", result.peakTorqueNm},
        {"avgCurrentA", result.avgCurrentA},
        {"avgTorqueNm", result.avgTorqueNm}
    };
}

QVariantMap TestExecutionViewModel::toVariantMap(const Domain::ImpactDirectionResult& result) const {
    QVariantList cyclesList;
    for (const auto& cycle : result.cycles) {
        cyclesList.append(toVariantMap(cycle));
    }

    return {
        {"direction", result.direction},
        {"cycles", cyclesList},
        {"maxCurrentA", result.maxCurrentA},
        {"maxTorqueNm", result.maxTorqueNm},
        {"avgCurrentA", result.avgCurrentA},
        {"avgTorqueNm", result.avgTorqueNm},
        {"currentPassed", result.currentPassed},
        {"torquePassed", result.torquePassed},
        {"overallPassed", result.overallPassed}
    };
}

} // namespace ViewModels
