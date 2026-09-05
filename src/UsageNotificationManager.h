#pragma once

#include "RateLimitParser.h"

class UsageNotificationManager
{
public:
    void notifyThresholdCrossings(
        const RateLimitWindowData &previousFiveHour,
        const RateLimitWindowData &currentFiveHour,
        const RateLimitWindowData &previousWeekly,
        const RateLimitWindowData &currentWeekly,
        int threshold
    ) const;

private:
    void notifyIfCrossed(
        const QString &name,
        const RateLimitWindowData &previous,
        const RateLimitWindowData &current,
        int threshold
    ) const;

    void sendNotification(const QString &body) const;
};
