#include "UsageNotificationManager.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QVariantMap>
#include <QtMath>

void UsageNotificationManager::notifyThresholdCrossings(
    const RateLimitWindowData &previousFiveHour,
    const RateLimitWindowData &currentFiveHour,
    const RateLimitWindowData &previousWeekly,
    const RateLimitWindowData &currentWeekly,
    int threshold
) const
{
    notifyIfCrossed(
        QStringLiteral("5-hour"),
        previousFiveHour,
        currentFiveHour,
        threshold
    );
    notifyIfCrossed(
        QStringLiteral("Weekly"),
        previousWeekly,
        currentWeekly,
        threshold
    );
}

void UsageNotificationManager::notifyIfCrossed(
    const QString &name,
    const RateLimitWindowData &previous,
    const RateLimitWindowData &current,
    int threshold
) const
{
    if (!previous.valid || !current.valid) {
        return;
    }

    const double previousRemaining = 100.0 - previous.usedPercent;
    const double currentRemaining = 100.0 - current.usedPercent;

    if (previousRemaining > threshold && currentRemaining <= threshold) {
        sendNotification(
            QStringLiteral("%1 quota has %2% remaining.")
                .arg(name)
                .arg(qRound(currentRemaining))
        );
    }
}

void UsageNotificationManager::sendNotification(const QString &body) const
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        return;
    }

    QDBusInterface notifications(
        QStringLiteral("org.freedesktop.Notifications"),
        QStringLiteral("/org/freedesktop/Notifications"),
        QStringLiteral("org.freedesktop.Notifications"),
        QDBusConnection::sessionBus()
    );

    if (!notifications.isValid()) {
        return;
    }

    notifications.asyncCall(
        QStringLiteral("Notify"),
        QStringLiteral("Codex MenuBar KDE"),
        uint(0),
        QStringLiteral("codex-menubar-kde"),
        QStringLiteral("Codex quota running low"),
        body,
        QStringList{},
        QVariantMap{},
        6000
    );
}
