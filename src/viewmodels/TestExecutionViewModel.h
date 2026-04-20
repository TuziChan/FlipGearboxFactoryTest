#ifndef TESTEXECUTIONVIEWMODEL_H
#define TESTEXECUTIONVIEWMODEL_H

#include <QObject>
#include <QTimer>
#include <QPointer>
#include <QMetaObject::Connection>
#include "../infrastructure/config/StationRuntime.h"
#include "../infrastructure/config/RuntimeManager.h"
#include "../domain/TestRecipe.h"
#include "../domain/TestRunState.h"

namespace ViewModels {

class TestExecutionViewModel : public QObject {
Q_OBJECT

    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(QString serialNumber READ serialNumber WRITE setSerialNumber NOTIFY serialNumberChanged)
    Q_PROPERTY(QString selectedModel READ selectedModel WRITE setSelectedModel NOTIFY selectedModelChanged)
    Q_PROPERTY(double backlashCompensationDeg READ backlashCompensationDeg WRITE setBacklashCompensationDeg NOTIFY backlashCompensationDegChanged)
    Q_PROPERTY(QString recipeName READ recipeName NOTIFY recipeNameChanged)
    Q_PROPERTY(QString currentPhase READ currentPhase NOTIFY currentPhaseChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(int progressPercent READ progressPercent NOTIFY progressPercentChanged)
    Q_PROPERTY(qint64 elapsedMs READ elapsedMs NOTIFY elapsedMsChanged)
    Q_PROPERTY(double motorCurrent READ motorCurrent NOTIFY telemetryChanged)
    Q_PROPERTY(double speed READ speed NOTIFY telemetryChanged)
    Q_PROPERTY(double torque READ torque NOTIFY telemetryChanged)
    Q_PROPERTY(double power READ power NOTIFY telemetryChanged)
    Q_PROPERTY(double angle READ angle NOTIFY telemetryChanged)
    Q_PROPERTY(double brakeCurrent READ brakeCurrent NOTIFY telemetryChanged)
    Q_PROPERTY(bool ai1Level READ ai1Level NOTIFY telemetryChanged)
    Q_PROPERTY(QString overallVerdict READ overallVerdict NOTIFY resultsChanged)
    Q_PROPERTY(bool testPassed READ testPassed NOTIFY resultsChanged)
    Q_PROPERTY(QVariantMap idleForwardResult READ idleForwardResult NOTIFY resultsChanged)
    Q_PROPERTY(QVariantMap idleReverseResult READ idleReverseResult NOTIFY resultsChanged)
    Q_PROPERTY(QVariantList angleResults READ angleResults NOTIFY resultsChanged)
    Q_PROPERTY(QVariantMap loadForwardResult READ loadForwardResult NOTIFY resultsChanged)
    Q_PROPERTY(QVariantMap loadReverseResult READ loadReverseResult NOTIFY resultsChanged)

public:
explicit TestExecutionViewModel(Infrastructure::Config::StationRuntime* runtime,
Infrastructure::Config::RuntimeManager* runtimeManager,
QObject* parent = nullptr);
~TestExecutionViewModel() override;

    bool running() const { return m_running; }
    QString serialNumber() const { return m_serialNumber; }
    QString selectedModel() const { return m_selectedModel; }
    double backlashCompensationDeg() const { return m_backlashCompensationDeg; }
    QString recipeName() const { return m_currentRecipe.name; }
    QString currentPhase() const { return m_currentPhase; }
    QString statusMessage() const { return m_statusMessage; }
    int progressPercent() const { return m_progressPercent; }
    qint64 elapsedMs() const { return m_elapsedMs; }
    double motorCurrent() const { return m_motorCurrent; }
    double speed() const { return m_speed; }
    double torque() const { return m_torque; }
    double power() const { return m_power; }
    double angle() const { return m_angle; }
    double brakeCurrent() const { return m_brakeCurrent; }
    bool ai1Level() const { return m_ai1Level; }
    QString overallVerdict() const { return m_overallVerdict; }
    bool testPassed() const { return m_testPassed; }
    QVariantMap idleForwardResult() const { return m_idleForwardResult; }
    QVariantMap idleReverseResult() const { return m_idleReverseResult; }
    QVariantList angleResults() const { return m_angleResults; }
    QVariantMap loadForwardResult() const { return m_loadForwardResult; }
    QVariantMap loadReverseResult() const { return m_loadReverseResult; }

    void setSerialNumber(const QString& sn);
    void setSelectedModel(const QString& model);
    void setBacklashCompensationDeg(double compensationDeg);

    Q_INVOKABLE void startTest();
    Q_INVOKABLE void stopTest();
    Q_INVOKABLE void resetTest();
    Q_INVOKABLE void loadRecipe(const QString& recipeName);

signals:
    void runningChanged();
    void serialNumberChanged();
    void selectedModelChanged();
    void backlashCompensationDegChanged();
    void recipeNameChanged();
    void currentPhaseChanged();
    void statusMessageChanged();
    void progressPercentChanged();
    void elapsedMsChanged();
    void telemetryChanged();
    void resultsChanged();
    void errorOccurred(const QString& message);

private slots:
    void onEngineStateChanged(const Domain::TestRunState& state);
    void onTestCompleted(const Domain::TestResults& results);
    void onTestFailed(const Domain::FailureReason& reason);

private:
Infrastructure::Config::StationRuntime* m_runtime;
Infrastructure::Config::RuntimeManager* m_runtimeManager;
Domain::TestRecipe m_currentRecipe;

bool m_running;
QString m_serialNumber;
QString m_selectedModel;
double m_backlashCompensationDeg;
QString m_currentPhase;
QString m_statusMessage;
int m_progressPercent;
qint64 m_elapsedMs;
double m_motorCurrent;
double m_speed;
double m_torque;
double m_power;
double m_angle;
double m_brakeCurrent;
bool m_ai1Level;
QString m_overallVerdict;
bool m_testPassed;
QVariantMap m_idleForwardResult;
QVariantMap m_idleReverseResult;
QVariantList m_angleResults;
QVariantMap m_loadForwardResult;
QVariantMap m_loadReverseResult;

// Connection tracking to prevent signal leaks
QPointer<Domain::GearboxTestEngine> m_connectedEngine;
QList<QMetaObject::Connection> m_connections;

QString recipeFilePathForModel(const QString& model) const;
Domain::TestRecipe buildExecutionRecipe() const;
void updateFromState(const Domain::TestRunState& state);
QVariantMap toVariantMap(const Domain::IdleRunResult& result) const;
QVariantMap toVariantMap(const Domain::LoadTestResult& result) const;
QVariantList toVariantList(const QVector<Domain::AngleResult>& results) const;
void updateRuntime(Infrastructure::Config::StationRuntime* newRuntime);
void onRuntimeRecreated(Infrastructure::Config::StationRuntime* newRuntime);
void connectEngine(Domain::GearboxTestEngine* engine);
void disconnectEngine();
};

} // namespace ViewModels

#endif // TESTEXECUTIONVIEWMODEL_H
