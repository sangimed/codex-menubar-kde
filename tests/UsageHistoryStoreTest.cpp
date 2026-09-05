#include "UsageHistoryStore.h"

#include <QTemporaryDir>
#include <QtTest>

class UsageHistoryStoreTest final : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void samplesAtMostEveryFiveMinutesWhenUnchanged();
    void samplesImmediatelyWhenValuesChange();
    void prunesEntriesOlderThanSevenDays();
};

void UsageHistoryStoreTest::samplesAtMostEveryFiveMinutesWhenUnchanged()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    UsageHistoryStore store(directory.filePath(QStringLiteral("history.json")));
    RateLimitWindowData fiveHour{
        .valid = true,
        .usedPercent = 25.0,
        .windowDurationMinutes = 300,
        .resetsAt = 1000,
    };
    RateLimitWindowData weekly{
        .valid = true,
        .usedPercent = 50.0,
        .windowDurationMinutes = 10'080,
        .resetsAt = 2000,
    };

    QVERIFY(store.record(fiveHour, weekly, QStringLiteral("10"), 1'000));
    QVERIFY(!store.record(fiveHour, weekly, QStringLiteral("10"), 1'120));
    QCOMPARE(store.entries().size(), 1);

    QVERIFY(store.record(fiveHour, weekly, QStringLiteral("10"), 1'301));
    QCOMPARE(store.entries().size(), 2);
}

void UsageHistoryStoreTest::samplesImmediatelyWhenValuesChange()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    UsageHistoryStore store(directory.filePath(QStringLiteral("history.json")));
    RateLimitWindowData fiveHour{
        .valid = true,
        .usedPercent = 25.0,
        .windowDurationMinutes = 300,
        .resetsAt = 1000,
    };

    QVERIFY(store.record(fiveHour, {}, {}, 1'000));
    fiveHour.usedPercent = 26.0;
    QVERIFY(store.record(fiveHour, {}, {}, 1'010));
    QCOMPARE(store.entries().size(), 2);
}

void UsageHistoryStoreTest::prunesEntriesOlderThanSevenDays()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    UsageHistoryStore store(directory.filePath(QStringLiteral("history.json")));
    RateLimitWindowData fiveHour{
        .valid = true,
        .usedPercent = 25.0,
        .windowDurationMinutes = 300,
        .resetsAt = 1000,
    };

    QVERIFY(store.record(fiveHour, {}, {}, 100));
    QVERIFY(store.record(fiveHour, {}, {}, 100 + 7 * 24 * 60 * 60 + 1));

    const auto entries = store.entries();
    QCOMPARE(entries.size(), 1);
    QCOMPARE(entries.constFirst().toMap().value(QStringLiteral("timestamp")).toLongLong(),
             qint64(100 + 7 * 24 * 60 * 60 + 1));
}

QTEST_MAIN(UsageHistoryStoreTest)

#include "UsageHistoryStoreTest.moc"
