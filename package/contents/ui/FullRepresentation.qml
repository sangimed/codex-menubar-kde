import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

Item {
    id: full

    required property var backend
    required property int percentageMode

    property double clockTick: Date.now()

    Layout.minimumWidth: Kirigami.Units.gridUnit * 19
    Layout.preferredWidth: Kirigami.Units.gridUnit * 22
    Layout.minimumHeight: Math.min(
        wrapper.implicitHeight,
        Kirigami.Units.gridUnit * 18
    )
    Layout.preferredHeight: Math.min(
        wrapper.implicitHeight,
        Kirigami.Units.gridUnit * 30
    )

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

    QQC2.ScrollView {
        id: scroll
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth

        Item {
            id: wrapper
            width: scroll.availableWidth
            implicitHeight: content.implicitHeight + Kirigami.Units.largeSpacing * 2

            ColumnLayout {
                id: content
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
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
                                : (backend.loading
                                    ? i18n("Connecting…")
                                    : i18n("Disconnected"))
                            opacity: 0.7
                            font.pointSize: Kirigami.Theme.smallFont.pointSize
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
                            ? i18n(
                                "Resets in %1",
                                full.countdown(backend.fiveHourResetsAt)
                            )
                            : i18n("Codex did not report this quota window.")
                        opacity: 0.7
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
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
                            ? i18n(
                                "Resets in %1",
                                full.countdown(backend.weeklyResetsAt)
                            )
                            : i18n("Codex did not report this quota window.")
                        opacity: 0.7
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                    }
                }

                Kirigami.Separator {
                    visible: backend.additionalLimits.length > 0
                    Layout.fillWidth: true
                }

                AdditionalLimitsSection {
                    Layout.fillWidth: true
                    limits: backend.additionalLimits
                    percentageMode: full.percentageMode
                }

                Kirigami.Separator {
                    Layout.fillWidth: true
                }

                UsageHistorySection {
                    Layout.fillWidth: true
                    history: backend.history
                    percentageMode: full.percentageMode
                    backend: full.backend
                }

                Kirigami.Separator {
                    Layout.fillWidth: true
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

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
                        opacity: 0.55
                        elide: Text.ElideMiddle
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                    }
                }
            }
        }
    }
}
