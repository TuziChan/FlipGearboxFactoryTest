#ifndef DIAGNOSTICSVIEWMODEL_H
#define DIAGNOSTICSVIEWMODEL_H

#include <QObject>
#include <QVariantList>
#include "../infrastructure/config/StationRuntime.h"

namespace ViewModels {

class DiagnosticsViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList deviceStatuses READ deviceStatuses NOTIFY deviceStatusesChanged)
    Q_PROPERTY(QVariantList communicationLogs READ communicationLogs NOTIFY communicationLogsChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(bool runtimeInitialized READ runtimeInitialized NOTIFY deviceStatusesChanged)

public:
    explicit DiagnosticsViewModel(Infrastructure::Config::StationRuntime* runtime, QObject* parent = nullptr);

    QVariantList deviceStatuses() const { return m_deviceStatuses; }
    QVariantList communicationLogs() const { return m_communicationLogs; }
    QString statusMessage() const { return m_statusMessage; }
    bool runtimeInitialized() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void refreshIncremental();
    Q_INVOKABLE void setMotorForward(double dutyCyclePercent = 40.0);
    Q_INVOKABLE void setMotorReverse(double dutyCyclePercent = 40.0);
    Q_INVOKABLE void stopMotor();
    Q_INVOKABLE void setBrakeOutput(bool enabled);
    Q_INVOKABLE void setBrakeCurrent(double currentA);
    Q_INVOKABLE void setEncoderZero();
    Q_INVOKABLE void clearLog();

signals:
    void deviceStatusesChanged();
    void communicationLogsChanged();
    void statusMessageChanged();

private:
    Infrastructure::Config::StationRuntime* m_runtime;
    QVariantList m_deviceStatuses;
    QVariantList m_communicationLogs;
    QString m_statusMessage;
    int m_nextRefreshIndex;

    void initializeStatuses();
    void updateDeviceStatus(int index);
    QVariantMap buildMotorStatus();
    QVariantMap buildTorqueStatus();
    QVariantMap buildEncoderStatus();
    QVariantMap buildBrakeStatus();
    QVariantMap buildOfflineStatus(const QString& name, const QString& reason) const;
    void appendLog(const QString& direction, const QString& device, const QString& message, bool success);
    void setStatusMessage(const QString& message);
};

} // namespace ViewModels

#endif // DIAGNOSTICSVIEWMODEL_H
