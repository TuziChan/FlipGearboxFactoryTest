#include "TestExecutionViewModel.h"
#include "../infrastructure/config/RecipeConfig.h"
#include "../infrastructure/config/ConfigLoader.h"
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

namespace ViewModels {

TestExecutionViewModel::TestExecutionViewModel(Infrastructure::Config::StationRuntime* runtime,
                                                 QObject* parent)
    : QObject(parent)
    , m_runtime(runtime)
    , m_currentRecipe(Infrastructure::Config::RecipeConfig::createDefault())
    , m_running(false)
    , m_serialNumber()
    , m_selectedModel("GBX-42A")
    , m_backlashCompensationDeg(0.0)
    , m_currentPhase("Idle")
    , m_statusMessage("Ready")
    , m_progressPercent(0)
    , m_elapsedMs(0)
    , m_motorCurrent(0.0)
    , m_speed(0.0)
    , m_torque(0.0)
    , m_power(0.0)
    , m_angle(0.0)
    , m_brakeCurrent(0.0)
    , m_ai1Level(false)
    , m_overallVerdict("Pending")
    , m_testPassed(false)
{
    loadRecipe(m_selectedModel);

    if (m_runtime && m_runtime->testEngine()) {
        connect(m_runtime->testEngine(), &Domain::GearboxTestEngine::stateChanged,
                this, &TestExecutionViewModel::onEngineStateChanged);
        connect(m_runtime->testEngine(), &Domain::GearboxTestEngine::testCompleted,
                this, &TestExecutionViewModel::onTestCompleted);
        connect(m_runtime->testEngine(), &Domain::GearboxTestEngine::testFailed,
                this, &TestExecutionViewModel::onTestFailed);
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

        emit runningChanged();
        emit currentPhaseChanged();
        emit statusMessageChanged();
        emit progressPercentChanged();
        emit elapsedMsChanged();
        emit resultsChanged();

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

void TestExecutionViewModel::onEngineStateChanged(const Domain::TestRunState& state) {
    updateFromState(state);
}

void TestExecutionViewModel::onTestCompleted(const Domain::TestResults& results) {
    m_running = false;
    m_testPassed = results.overallPassed;
    m_overallVerdict = results.overallPassed ? "PASSED" : "FAILED";

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

    bool telemetryUpdated = false;

    if (m_motorCurrent != state.currentTelemetry.motorCurrentA) {
        m_motorCurrent = state.currentTelemetry.motorCurrentA;
        telemetryUpdated = true;
    }

    if (m_speed != state.currentTelemetry.dynSpeedRpm) {
        m_speed = state.currentTelemetry.dynSpeedRpm;
        telemetryUpdated = true;
    }

    if (m_torque != state.currentTelemetry.dynTorqueNm) {
        m_torque = state.currentTelemetry.dynTorqueNm;
        telemetryUpdated = true;
    }

    if (m_power != state.currentTelemetry.dynPowerW) {
        m_power = state.currentTelemetry.dynPowerW;
        telemetryUpdated = true;
    }

    if (m_angle != state.currentTelemetry.encoderAngleDeg) {
        m_angle = state.currentTelemetry.encoderAngleDeg;
        telemetryUpdated = true;
    }

    if (m_brakeCurrent != state.currentTelemetry.brakeCurrentA) {
        m_brakeCurrent = state.currentTelemetry.brakeCurrentA;
        telemetryUpdated = true;
    }

    if (m_ai1Level != state.currentTelemetry.aqmdAi1Level) {
        m_ai1Level = state.currentTelemetry.aqmdAi1Level;
        telemetryUpdated = true;
    }

    if (telemetryUpdated) {
        emit telemetryChanged();
    }
}

QString TestExecutionViewModel::recipeFilePathForModel(const QString& model) const {
    const QString recipeDir = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../../config/recipes");
    return QDir(recipeDir).absoluteFilePath(model + ".json");
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

} // namespace ViewModels
