import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

ColumnLayout {
    id: root

    required property var history
    required property int percentageMode
    required property var backend

    spacing: Kirigami.Units.smallSpacing

    function shownPercent(usedPercent) {
        return percentageMode === 1 ? usedPercent : 100 - usedPercent
    }

    onHistoryChanged: chart.requestPaint()
    onPercentageModeChanged: chart.requestPaint()

    RowLayout {
        Layout.fillWidth: true

        PlasmaComponents.Label {
            text: i18n("7-day history")
            font.bold: true
            Layout.fillWidth: true
        }

        PlasmaComponents.Label {
            text: i18np("%1 sample", "%1 samples", root.history.length)
            opacity: 0.65
            font.pointSize: Kirigami.Theme.smallFont.pointSize
        }

        PlasmaComponents.ToolButton {
            visible: root.history.length > 0
            icon.name: "edit-clear-history"
            Accessible.name: i18n("Clear history")
            onClicked: root.backend.clearHistory()
        }
    }

    Canvas {
        id: chart

        Layout.fillWidth: true
        Layout.preferredHeight: Kirigami.Units.gridUnit * 5
        visible: root.history.length > 0

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()

        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            if (!root.history || root.history.length === 0) {
                return
            }

            const left = 2
            const right = Math.max(left + 1, width - 2)
            const top = 4
            const bottom = Math.max(top + 1, height - 4)
            const firstTimestamp = Number(root.history[0].timestamp)
            const lastTimestamp = Number(root.history[root.history.length - 1].timestamp)
            const span = Math.max(1, lastTimestamp - firstTimestamp)

            const textColor = Kirigami.Theme.textColor
            ctx.lineWidth = 1
            ctx.strokeStyle = Qt.rgba(
                textColor.r,
                textColor.g,
                textColor.b,
                0.12
            )
            for (let percent = 0; percent <= 100; percent += 25) {
                const y = bottom - (percent / 100) * (bottom - top)
                ctx.beginPath()
                ctx.moveTo(left, y)
                ctx.lineTo(right, y)
                ctx.stroke()
            }

            function drawSeries(hasKey, valueKey, color) {
                ctx.lineWidth = 2
                ctx.strokeStyle = color
                ctx.lineJoin = "round"
                ctx.lineCap = "round"
                ctx.beginPath()

                let drawing = false
                for (let i = 0; i < root.history.length; ++i) {
                    const sample = root.history[i]
                    if (!sample[hasKey]) {
                        drawing = false
                        continue
                    }

                    const x = left
                        + ((Number(sample.timestamp) - firstTimestamp) / span)
                        * (right - left)
                    const percentage = Math.max(
                        0,
                        Math.min(100, root.shownPercent(Number(sample[valueKey])))
                    )
                    const y = bottom - (percentage / 100) * (bottom - top)

                    if (!drawing) {
                        ctx.moveTo(x, y)
                        drawing = true
                    } else {
                        ctx.lineTo(x, y)
                    }
                }

                ctx.stroke()
            }

            drawSeries(
                "hasFiveHour",
                "fiveHourUsedPercent",
                Kirigami.Theme.highlightColor
            )
            drawSeries(
                "hasWeekly",
                "weeklyUsedPercent",
                Kirigami.Theme.positiveTextColor
            )
        }
    }

    PlasmaComponents.Label {
        visible: root.history.length === 0
        Layout.fillWidth: true
        text: i18n("History will appear after the first usage sample.")
        opacity: 0.65
        font.pointSize: Kirigami.Theme.smallFont.pointSize
        wrapMode: Text.WordWrap
    }

    RowLayout {
        visible: root.history.length > 0
        spacing: Kirigami.Units.largeSpacing

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Rectangle {
                width: Kirigami.Units.smallSpacing * 2
                height: 3
                radius: 1.5
                color: Kirigami.Theme.highlightColor
            }

            PlasmaComponents.Label {
                text: i18n("5-hour")
                opacity: 0.7
                font.pointSize: Kirigami.Theme.smallFont.pointSize
            }
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Rectangle {
                width: Kirigami.Units.smallSpacing * 2
                height: 3
                radius: 1.5
                color: Kirigami.Theme.positiveTextColor
            }

            PlasmaComponents.Label {
                text: i18n("Weekly")
                opacity: 0.7
                font.pointSize: Kirigami.Theme.smallFont.pointSize
            }
        }

        Item {
            Layout.fillWidth: true
        }
    }
}
