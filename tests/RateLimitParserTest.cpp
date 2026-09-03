#include "RateLimitParser.h"

#include <QJsonDocument>
#include <QtTest>

class RateLimitParserTest final : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void parsesTopLevelSnapshot();
    void prefersCodexBucket();
    void parsesNullableWindowMetadata();
    void rejectsMissingRateLimits();
};

void RateLimitParserTest::parsesTopLevelSnapshot()
{
    const QByteArray json = R"JSON(
        {
            "rateLimits": {
                "primary": {
                    "usedPercent": 82,
                    "windowDurationMins": 300,
                    "resetsAt": 1788125155
                },
                "secondary": {
                    "usedPercent": 26,
                    "windowDurationMins": 10080,
                    "resetsAt": 1788693894
                },
                "credits": {
                    "hasCredits": true,
                    "unlimited": false,
                    "balance": "240"
                },
                "planType": "plus"
            }
        }
    )JSON";

    const auto document = QJsonDocument::fromJson(json);
    const auto summary = RateLimitParser::parse(document.object());

    QVERIFY(summary.has_value());
    QVERIFY(summary->fiveHour.valid);
    QCOMPARE(summary->fiveHour.usedPercent, 82.0);
    QCOMPARE(summary->fiveHour.windowDurationMinutes, 300);

    QVERIFY(summary->weekly.valid);
    QCOMPARE(summary->weekly.usedPercent, 26.0);
    QCOMPARE(summary->weekly.windowDurationMinutes, 10'080);

    QVERIFY(summary->creditsReported);
    QVERIFY(summary->hasCredits);
    QVERIFY(!summary->unlimitedCredits);
    QCOMPARE(summary->creditsBalance, QStringLiteral("240"));
    QCOMPARE(summary->planType, QStringLiteral("plus"));
}

void RateLimitParserTest::prefersCodexBucket()
{
    const QByteArray json = R"JSON(
        {
            "rateLimits": {
                "primary": {
                    "usedPercent": 1,
                    "windowDurationMins": 300,
                    "resetsAt": 1
                }
            },
            "rateLimitsByLimitId": {
                "codex": {
                    "primary": {
                        "usedPercent": 42,
                        "windowDurationMins": 300,
                        "resetsAt": 2
                    }
                },
                "codex-mini": {
                    "limitName": "Codex Mini",
                    "primary": {
                        "usedPercent": 12,
                        "windowDurationMins": 300,
                        "resetsAt": 3
                    }
                }
            }
        }
    )JSON";

    const auto document = QJsonDocument::fromJson(json);
    const auto summary = RateLimitParser::parse(document.object());

    QVERIFY(summary.has_value());
    QCOMPARE(summary->fiveHour.usedPercent, 42.0);
    QCOMPARE(summary->additionalLimits.size(), 1);

    const auto additional = summary->additionalLimits.constFirst().toMap();
    QCOMPARE(additional.value(QStringLiteral("id")).toString(), QStringLiteral("codex-mini"));
    QCOMPARE(additional.value(QStringLiteral("name")).toString(), QStringLiteral("Codex Mini"));
}

void RateLimitParserTest::parsesNullableWindowMetadata()
{
    const QByteArray json = R"JSON(
        {
            "rateLimits": {
                "primary": {
                    "usedPercent": 37,
                    "windowDurationMins": null,
                    "resetsAt": null
                },
                "secondary": {
                    "usedPercent": 64,
                    "windowDurationMins": null,
                    "resetsAt": null
                }
            }
        }
    )JSON";

    const auto document = QJsonDocument::fromJson(json);
    const auto summary = RateLimitParser::parse(document.object());

    QVERIFY(summary.has_value());
    QVERIFY(summary->fiveHour.valid);
    QCOMPARE(summary->fiveHour.usedPercent, 37.0);
    QCOMPARE(summary->fiveHour.windowDurationMinutes, 0);
    QCOMPARE(summary->fiveHour.resetsAt, qint64(0));

    QVERIFY(summary->weekly.valid);
    QCOMPARE(summary->weekly.usedPercent, 64.0);
    QCOMPARE(summary->weekly.windowDurationMinutes, 0);
    QCOMPARE(summary->weekly.resetsAt, qint64(0));
}

void RateLimitParserTest::rejectsMissingRateLimits()
{
    const auto summary = RateLimitParser::parse(QJsonObject{});
    QVERIFY(!summary.has_value());
}

QTEST_MAIN(RateLimitParserTest)

#include "RateLimitParserTest.moc"
