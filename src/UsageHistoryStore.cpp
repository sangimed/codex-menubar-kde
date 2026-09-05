#include "UsageHistoryStore.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>

#include <cmath>
#include <utility>

namespace
{
constexpr qint64 RetentionSeconds = 7 * 24 * 60 * 60;
constexpr qint64 MinimumSampleSeconds = 5 * 60;
constexpr qsizetype MaximumSamples = 2'500;

bool samePercentage(double lhs, double rhs)
{
    return std::abs(lhs - rhs) < 0.001;
}
}

UsageHistoryStore::UsageHistoryStore(QString filePath)
{
    if (filePath.isEmpty()) {
        const QString dataRoot = QStandardPaths::writableLocation(
            QStandardPaths::GenericDataLocation
        );
        const QString directory = dataRoot + QStringLiteral("/codex-menubar-kde");
        QDir().mkpath(directory);
        m_filePath = directory + QStringLiteral("/usage-history.json");
    } else {
        m_filePath = std::move(filePath);
        QDir().mkpath(QFileInfo(m_filePath).absolutePath());
    }

    load();
    const qint64 now = QDateTime::currentSecsSinceEpoch();
    if (prune(now)) {
        save();
    }
}

QVariantList UsageHistoryStore::entries() const
{
    QVariantList result;
    result.reserve(m_entries.size());

    for (const auto &sample : m_entries) {
        QVariantMap row;
        row.insert(QStringLiteral("timestamp"), sample.timestamp);
        row.insert(QStringLiteral("hasFiveHour"), sample.hasFiveHour);
        row.insert(QStringLiteral("fiveHourUsedPercent"), sample.fiveHourUsedPercent);
        row.insert(QStringLiteral("hasWeekly"), sample.hasWeekly);
        row.insert(QStringLiteral("weeklyUsedPercent"), sample.weeklyUsedPercent);
        row.insert(QStringLiteral("credits"), sample.credits);
        result.append(row);
    }

    return result;
}

bool UsageHistoryStore::record(
    const RateLimitWindowData &fiveHour,
    const RateLimitWindowData &weekly,
    const QString &credits,
    qint64 nowSeconds
)
{
    if (nowSeconds <= 0) {
        nowSeconds = QDateTime::currentSecsSinceEpoch();
    }

    bool changed = prune(nowSeconds);

    UsageHistorySample candidate;
    candidate.timestamp = nowSeconds;
    candidate.hasFiveHour = fiveHour.valid;
    candidate.fiveHourUsedPercent = fiveHour.usedPercent;
    candidate.hasWeekly = weekly.valid;
    candidate.weeklyUsedPercent = weekly.usedPercent;
    candidate.credits = credits;

    if (!m_entries.isEmpty()) {
        const auto &last = m_entries.constLast();
        const bool valuesChanged =
            last.hasFiveHour != candidate.hasFiveHour
            || last.hasWeekly != candidate.hasWeekly
            || (candidate.hasFiveHour
                && !samePercentage(
                    last.fiveHourUsedPercent,
                    candidate.fiveHourUsedPercent
                ))
            || (candidate.hasWeekly
                && !samePercentage(
                    last.weeklyUsedPercent,
                    candidate.weeklyUsedPercent
                ))
            || last.credits != candidate.credits;

        const bool intervalElapsed =
            nowSeconds - last.timestamp >= MinimumSampleSeconds;

        if (!valuesChanged && !intervalElapsed) {
            if (changed) {
                save();
            }
            return changed;
        }
    }

    m_entries.append(candidate);
    changed = true;

    if (m_entries.size() > MaximumSamples) {
        m_entries.remove(
            0,
            m_entries.size() - MaximumSamples
        );
    }

    save();
    return changed;
}

bool UsageHistoryStore::clear()
{
    const bool hadEntries = !m_entries.isEmpty();
    m_entries.clear();
    QFile::remove(m_filePath);
    return hadEntries;
}

void UsageHistoryStore::load()
{
    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !document.isArray()) {
        return;
    }

    for (const auto &value : document.array()) {
        if (!value.isObject()) {
            continue;
        }

        const auto object = value.toObject();
        const auto timestampValue = object.value(QStringLiteral("timestamp"));
        if (!timestampValue.isDouble()) {
            continue;
        }

        UsageHistorySample sample;
        sample.timestamp = static_cast<qint64>(timestampValue.toDouble());
        sample.hasFiveHour = object.value(QStringLiteral("hasFiveHour")).toBool(false);
        sample.fiveHourUsedPercent = object.value(
            QStringLiteral("fiveHourUsedPercent")
        ).toDouble();
        sample.hasWeekly = object.value(QStringLiteral("hasWeekly")).toBool(false);
        sample.weeklyUsedPercent = object.value(
            QStringLiteral("weeklyUsedPercent")
        ).toDouble();
        sample.credits = object.value(QStringLiteral("credits")).toString();
        m_entries.append(sample);
    }
}

bool UsageHistoryStore::save() const
{
    QJsonArray array;

    for (const auto &sample : m_entries) {
        array.append(QJsonObject{
            {QStringLiteral("timestamp"), sample.timestamp},
            {QStringLiteral("hasFiveHour"), sample.hasFiveHour},
            {QStringLiteral("fiveHourUsedPercent"), sample.fiveHourUsedPercent},
            {QStringLiteral("hasWeekly"), sample.hasWeekly},
            {QStringLiteral("weeklyUsedPercent"), sample.weeklyUsedPercent},
            {QStringLiteral("credits"), sample.credits},
        });
    }

    QSaveFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
    return file.commit();
}

bool UsageHistoryStore::prune(qint64 nowSeconds)
{
    const qint64 cutoff = nowSeconds - RetentionSeconds;
    qsizetype removeCount = 0;

    while (removeCount < m_entries.size()
           && m_entries.at(removeCount).timestamp < cutoff) {
        ++removeCount;
    }

    if (removeCount == 0) {
        return false;
    }

    m_entries.remove(0, removeCount);
    return true;
}
