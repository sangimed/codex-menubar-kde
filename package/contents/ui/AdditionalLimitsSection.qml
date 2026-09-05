import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

ColumnLayout {
    id: root

    required property var limits
    required property int percentageMode

    visible: limits && limits.length > 0
    spacing: Kirigami.Units.smallSpacing

    function shownPercent(usedPercent) {
        return Math.round(percentageMode === 1 ? usedPercent : 100 - usedPercent)
    }

    function percentageCaption() {
        return percentageMode === 1 ? i18n("used") : i18n("remaining")
    }

    function windowTitle(minutes) {
        if (minutes === 300) {
            return i18n("5-hour")
        }
        if (minutes === 10080) {
            return i18n("Weekly")
        }
        if (!minutes) {
            return i18n("Quota window")
        }
        if (minutes % 1440 === 0) {
            return i18n("%1d window", minutes / 1440)
        }
        if (minutes % 60 === 0) {
            return i18n("%1h window", minutes / 60)
        }
        return i18n("%1m window", minutes)
    }

    PlasmaComponents.Label {
        text: i18n("Additional Codex limits (%1)", root.limits.length)
        font.bold: true
        Layout.fillWidth: true
    }

    Repeater {
        model: root.limits

        delegate: ColumnLayout {
            id: limitDelegate
            required property var modelData

            Layout.fillWidth: true
            spacing: 2

            PlasmaComponents.Label {
                text: limitDelegate.modelData.name
                opacity: 0.8
                font.bold: true
                font.pointSize: Kirigami.Theme.smallFont.pointSize
            }

            Repeater {
                model: limitDelegate.modelData.windows

                delegate: RowLayout {
                    required property var modelData
                    Layout.fillWidth: true

                    PlasmaComponents.Label {
                        text: root.windowTitle(modelData.windowDurationMinutes)
                        Layout.fillWidth: true
                        opacity: 0.7
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                    }

                    PlasmaComponents.Label {
                        text: i18n(
                            "%1% %2",
                            root.shownPercent(modelData.usedPercent),
                            root.percentageCaption()
                        )
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                    }
                }
            }
        }
    }
}
