#pragma once

#include "RateLimitParser.h"

#include <QString>
#include <QVariantList>
#include <QVector>

struct UsageHistorySample
{
    qint64 timestamp = 0;
    bool hasFiveHour = false;
    double fiveHourUsedPercent = 0.0;
    bool hasWeekly = false;
    double weeklyUsedPercent = 0.0;
    QString credits;
};

class UsageHistoryStore
{
public:
    explicit UsageHistoryStore(QString filePath = {});

    QVariantList entries() const;

    bool record(
        const RateLimitWindowData &fiveHour,
        const RateLimitWindowData &weekly,
        const QString &credits,
        qint64 nowSeconds = 0
    );

    bool clear();

private:
    void load();
    bool save() const;
    bool prune(qint64 nowSeconds);

    QString m_filePath;
    QVector<UsageHistorySample> m_entries;
};
