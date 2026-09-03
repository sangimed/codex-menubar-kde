#include "RateLimitParser.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QRegularExpression>
#include <QtGlobal>

namespace
{
constexpr int FiveHourWindowMinutes = 300;
constexpr int WeeklyWindowMinutes = 10'080;

QString jsonValueToString(const QJsonValue &value)
{
    if (value.isString()) {
        return value.toString();
    }

    if (value.isDouble()) {
        return QString::number(value.toDouble(), 'f', 2)
            .remove(QRegularExpression(QStringLiteral(R"(\.?0+$)")));
    }

    return {};
}

QVariantMap windowToVariant(const RateLimitWindowData &window)
{
    return {
        {QStringLiteral("usedPercent"), window.usedPercent},
        {QStringLiteral("windowDurationMinutes"), window.windowDurationMinutes},
        {QStringLiteral("resetsAt"), window.resetsAt},
    };
}
}

RateLimitWindowData RateLimitParser::parseWindow(const QJsonValue &value)
{
    if (!value.isObject()) {
        return {};
    }

    const auto object = value.toObject();
    const auto usedPercentValue = object.value(QStringLiteral("usedPercent"));
    if (!usedPercentValue.isDouble()) {
        return {};
    }

    const auto durationValue = object.value(QStringLiteral("windowDurationMins"));
    const auto resetsAtValue = object.value(QStringLiteral("resetsAt"));

    return {
        .valid = true,
        .usedPercent = qBound(0.0, usedPercentValue.toDouble(), 100.0),
        .windowDurationMinutes = durationValue.isDouble()
            ? durationValue.toInt()
            : 0,
        .resetsAt = resetsAtValue.isDouble()
            ? static_cast<qint64>(resetsAtValue.toDouble())
            : 0,
    };
}

std::optional<RateLimitSummary> RateLimitParser::parse(
    const QJsonObject &container
)
{
    const auto rateLimitsValue = container.value(QStringLiteral("rateLimits"));
    if (!rateLimitsValue.isObject()) {
        return std::nullopt;
    }

    const auto bucketsValue =
        container.value(QStringLiteral("rateLimitsByLimitId"));
    const auto buckets = bucketsValue.isObject()
        ? bucketsValue.toObject()
        : QJsonObject{};

    QJsonObject snapshot = rateLimitsValue.toObject();
    const auto codexValue = buckets.value(QStringLiteral("codex"));
    if (codexValue.isObject()) {
        snapshot = codexValue.toObject();
    }

    return parseSnapshot(snapshot, buckets);
}

RateLimitSummary RateLimitParser::parseSnapshot(
    const QJsonObject &snapshot,
    const QJsonObject &rateLimitsByLimitId
)
{
    RateLimitSummary summary;

    const auto primary = parseWindow(snapshot.value(QStringLiteral("primary")));
    const auto secondary =
        parseWindow(snapshot.value(QStringLiteral("secondary")));

    for (const auto &window : {primary, secondary}) {
        if (!window.valid) {
            continue;
        }

        if (window.windowDurationMinutes == FiveHourWindowMinutes) {
            summary.fiveHour = window;
        } else if (window.windowDurationMinutes == WeeklyWindowMinutes) {
            summary.weekly = window;
        }
    }

    // Current Codex protocol allows windowDurationMins and resetsAt to be null.
    // When the duration is absent, preserve the historical primary/secondary
    // meaning instead of dropping otherwise valid usage percentages.
    if (!summary.fiveHour.valid
        && primary.valid
        && primary.windowDurationMinutes == 0) {
        summary.fiveHour = primary;
    }

    if (!summary.weekly.valid
        && secondary.valid
        && secondary.windowDurationMinutes == 0) {
        summary.weekly = secondary;
    }

    const auto creditsValue = snapshot.value(QStringLiteral("credits"));
    if (creditsValue.isObject()) {
        summary.creditsReported = true;

        const auto credits = creditsValue.toObject();
        summary.hasCredits =
            credits.value(QStringLiteral("hasCredits")).toBool(false);
        summary.unlimitedCredits =
            credits.value(QStringLiteral("unlimited")).toBool(false);
        summary.creditsBalance =
            jsonValueToString(credits.value(QStringLiteral("balance")));
    }

    summary.planType = snapshot.value(QStringLiteral("planType")).toString();

    for (auto it = rateLimitsByLimitId.constBegin();
         it != rateLimitsByLimitId.constEnd();
         ++it) {
        if (it.key() == QStringLiteral("codex") || !it.value().isObject()) {
            continue;
        }

        const auto additionalSnapshot = it.value().toObject();
        QVariantMap limit;
        limit.insert(QStringLiteral("id"), it.key());

        const auto providedName =
            additionalSnapshot.value(QStringLiteral("limitName")).toString();
        limit.insert(
            QStringLiteral("name"),
            providedName.isEmpty() ? it.key() : providedName
        );

        QVariantList windows;
        for (const auto &candidate : {
                 parseWindow(additionalSnapshot.value(QStringLiteral("primary"))),
                 parseWindow(additionalSnapshot.value(QStringLiteral("secondary")))
             }) {
            if (candidate.valid) {
                windows.append(windowToVariant(candidate));
            }
        }

        limit.insert(QStringLiteral("windows"), windows);
        summary.additionalLimits.append(limit);
    }

    return summary;
}
