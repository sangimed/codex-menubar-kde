#pragma once

#include <QJsonObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include <optional>

struct RateLimitWindowData
{
    bool valid = false;
    double usedPercent = 0.0;
    int windowDurationMinutes = 0;
    qint64 resetsAt = 0;
};

struct RateLimitSummary
{
    RateLimitWindowData fiveHour;
    RateLimitWindowData weekly;

    bool creditsReported = false;
    bool hasCredits = false;
    bool unlimitedCredits = false;
    QString creditsBalance;

    QString planType;
    QVariantList additionalLimits;
};

class RateLimitParser
{
public:
    static std::optional<RateLimitSummary> parse(const QJsonObject &container);

private:
    static RateLimitWindowData parseWindow(const QJsonValue &value);
    static RateLimitSummary parseSnapshot(
        const QJsonObject &snapshot,
        const QJsonObject &rateLimitsByLimitId
    );
};
