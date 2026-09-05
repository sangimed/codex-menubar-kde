#pragma once

#include "RateLimitParser.h"
#include "UsageHistoryStore.h"
#include "UsageNotificationManager.h"

#include <QJsonObject>
#include <QObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QTimer>

class CodexBackend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool connected READ connected NOTIFY stateChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY stateChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY stateChanged)
    Q_PROPERTY(QString codexExecutable READ codexExecutable NOTIFY stateChanged)

    Q_PROPERTY(bool hasFiveHour READ hasFiveHour NOTIFY usageChanged)
    Q_PROPERTY(double fiveHourUsedPercent READ fiveHourUsedPercent NOTIFY usageChanged)
    Q_PROPERTY(qint64 fiveHourResetsAt READ fiveHourResetsAt NOTIFY usageChanged)

    Q_PROPERTY(bool hasWeekly READ hasWeekly NOTIFY usageChanged)
    Q_PROPERTY(double weeklyUsedPercent READ weeklyUsedPercent NOTIFY usageChanged)
    Q_PROPERTY(qint64 weeklyResetsAt READ weeklyResetsAt NOTIFY usageChanged)

    Q_PROPERTY(bool creditsReported READ creditsReported NOTIFY usageChanged)
    Q_PROPERTY(bool hasCredits READ hasCredits NOTIFY usageChanged)
    Q_PROPERTY(bool unlimitedCredits READ unlimitedCredits NOTIFY usageChanged)
    Q_PROPERTY(QString creditsBalance READ creditsBalance NOTIFY usageChanged)

    Q_PROPERTY(QString planType READ planType NOTIFY usageChanged)
    Q_PROPERTY(QVariantList additionalLimits READ additionalLimits NOTIFY usageChanged)
    Q_PROPERTY(QVariantList history READ history NOTIFY historyChanged)

    Q_PROPERTY(
        int refreshIntervalSeconds
        READ refreshIntervalSeconds
        WRITE setRefreshIntervalSeconds
        NOTIFY refreshIntervalSecondsChanged
    )
    Q_PROPERTY(
        bool notificationsEnabled
        READ notificationsEnabled
        WRITE setNotificationsEnabled
        NOTIFY notificationSettingsChanged
    )
    Q_PROPERTY(
        int notificationThreshold
        READ notificationThreshold
        WRITE setNotificationThreshold
        NOTIFY notificationSettingsChanged
    )

public:
    explicit CodexBackend(QObject *parent = nullptr);
    ~CodexBackend() override;

    bool connected() const;
    bool loading() const;
    QString errorString() const;
    QString codexExecutable() const;

    bool hasFiveHour() const;
    double fiveHourUsedPercent() const;
    qint64 fiveHourResetsAt() const;

    bool hasWeekly() const;
    double weeklyUsedPercent() const;
    qint64 weeklyResetsAt() const;

    bool creditsReported() const;
    bool hasCredits() const;
    bool unlimitedCredits() const;
    QString creditsBalance() const;

    QString planType() const;
    QVariantList additionalLimits() const;
    QVariantList history() const;

    int refreshIntervalSeconds() const;
    void setRefreshIntervalSeconds(int seconds);

    bool notificationsEnabled() const;
    void setNotificationsEnabled(bool enabled);
    int notificationThreshold() const;
    void setNotificationThreshold(int threshold);

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void clearHistory();

Q_SIGNALS:
    void stateChanged();
    void usageChanged();
    void historyChanged();
    void refreshIntervalSecondsChanged();
    void notificationSettingsChanged();

private Q_SLOTS:
    void handleStarted();
    void handleReadyRead();
    void handleStandardError();
    void handleFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void handleProcessError(QProcess::ProcessError error);
    void restartAfterFailure();

private:
    void startProcess();
    void scheduleRestart();
    void setError(QString message);
    QString resolveCodexExecutable() const;
    QProcessEnvironment processEnvironment(const QString &executable) const;

    void sendMessage(const QJsonObject &message);
    void sendInitialize();
    void sendRateLimitRead();
    void handleMessage(const QJsonObject &message);
    void applySummary(const RateLimitSummary &summary, bool preserveMetadata);
    void reportMissingWindowsIfNeeded();

    QProcess m_process;
    QByteArray m_stdoutBuffer;
    QTimer m_refreshTimer;
    QTimer m_reconnectTimer;

    bool m_connected = false;
    bool m_loading = true;
    bool m_initialized = false;
    bool m_stopping = false;
    QString m_errorString;
    QString m_codexExecutable;

    RateLimitWindowData m_fiveHour;
    RateLimitWindowData m_weekly;

    bool m_creditsReported = false;
    bool m_hasCredits = false;
    bool m_unlimitedCredits = false;
    QString m_creditsBalance;
    QString m_planType;
    QVariantList m_additionalLimits;

    int m_refreshIntervalSeconds = 30;
    bool m_notificationsEnabled = false;
    int m_notificationThreshold = 20;
    int m_nextRequestId = 3;
    int m_reconnectAttempts = 0;

    UsageHistoryStore m_historyStore;
    UsageNotificationManager m_notificationManager;
};
