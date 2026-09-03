import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

Item {
    id: full

    required property var backend
    required property int percentageMode

    property double clockTick: Date.now()

    Layout.minimumWidth: Kirigami.Units.gridUnit * 18
    Layout.minimumHeight: content.implicitHeight
    Layout.preferredWidth: Kirigami.Units.gridUnit * 21
    Layout.preferredHeight: content.implicitHeight

    function shownPercent(usedPercent) {
        return Math.round(percentageMode === 1 ? usedPercent : 100 - usedPercent)
    }

    function percentageCaption() {
        return percentageMode === 1 ? i18n("used") : i18n("remaining")
    }

    function countdown(epochSeconds) {
        if (!epochSeconds) {
            return i18n("Unknown")
        }

        let seconds = Math.max(0, Math.floor(epochSeconds - clockTick / 1000))
        const days = Math.floor(seconds / 86400)
        seconds %= 86400
        const hours = Math.floor(seconds / 3600)
        seconds %= 3600
        const minutes = Math.floor(seconds / 60)
        const secs = seconds % 60

        if (days > 0) {
            return i18n("%1d %2h", days, hours)
        }

        return String(hours).padStart(2, "0")
            + ":" + String(minutes).padStart(2, "0")
            + ":" + String(secs).padStart(2, "0")
    }

    Timer {
        interval: 1000
        repeat: true
        running: true
        onTriggered: full.clockTick = Date.now()
    }

    ColumnLayout {
        id: content
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Icon {
                source: Qt.resolvedUrl("../images/codex-menubar-kde.svg")
                implicitWidth: Kirigami.Units.iconSizes.medium
                implicitHeight: implicitWidth
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                Kirigami.Heading {
                    text: i18n("Codex usage")
                    level: 2
                    Layout.fillWidth: true
                }

                PlasmaComponents.Label {
                    Layout.fillWidth: true
                    text: backend.connected
                        ? i18n("Connected")
                        : (backend.loading ? i18n("Connecting…") : i18n("Disconnected"))
                    opacity: 0.7
                }
            }

            PlasmaComponents.Button {
                icon.name: "view-refresh"
                text: backend.loading ? i18n("Refreshing…") : i18n("Refresh")
                enabled: backend.connected && !backend.loading
                onClicked: backend.refresh()
            }
        }

        PlasmaComponents.Label {
            visible: !!backend.errorString
            Layout.fillWidth: true
            text: backend.errorString
            color: Kirigami.Theme.negativeTextColor
            wrapMode: Text.WordWrap
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            RowLayout {
                Layout.fillWidth: true

                PlasmaComponents.Label {
                    text: i18n("5-hour window")
                    Layout.fillWidth: true
                    font.bold: true
                }

                PlasmaComponents.Label {
                    text: backend.hasFiveHour
                        ? i18n(
                            "%1% %2",
                            full.shownPercent(backend.fiveHourUsedPercent),
                            full.percentageCaption()
                        )
                        : i18n("Unavailable")
                }
            }

            QuotaBar {
                Layout.fillWidth: true
                value: backend.hasFiveHour
                    ? full.shownPercent(backend.fiveHourUsedPercent)
                    : 0
            }

            PlasmaComponents.Label {
                text: backend.hasFiveHour
                    ? i18n("Resets in %1", full.countdown(backend.fiveHourResetsAt))
                    : i18n("Codex did not report this quota window.")
                opacity: 0.75
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            RowLayout {
                Layout.fillWidth: true

                PlasmaComponents.Label {
                    text: i18n("Weekly window")
                    Layout.fillWidth: true
                    font.bold: true
                }

                PlasmaComponents.Label {
                    text: backend.hasWeekly
                        ? i18n(
                            "%1% %2",
                            full.shownPercent(backend.weeklyUsedPercent),
                            full.percentageCaption()
                        )
                        : i18n("Unavailable")
                }
            }

            QuotaBar {
                Layout.fillWidth: true
                value: backend.hasWeekly
                    ? full.shownPercent(backend.weeklyUsedPercent)
                    : 0
            }

            PlasmaComponents.Label {
                text: backend.hasWeekly
                    ? i18n("Resets in %1", full.countdown(backend.weeklyResetsAt))
                    : i18n("Codex did not report this quota window.")
                opacity: 0.75
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            PlasmaComponents.Label {
                visible: !!backend.planType
                text: i18n("Plan: %1", backend.planType)
            }

            PlasmaComponents.Label {
                visible: backend.creditsReported
                text: backend.hasCredits
                    ? i18n(
                        "Credits: %1",
                        backend.unlimitedCredits ? "∞" : backend.creditsBalance
                    )
                    : i18n("Credits: unavailable")
            }

            PlasmaComponents.Label {
                visible: !!backend.codexExecutable
                Layout.fillWidth: true
                text: i18n("Codex CLI: %1", backend.codexExecutable)
                opacity: 0.6
                elide: Text.ElideMiddle
            }
        }
    }
}
