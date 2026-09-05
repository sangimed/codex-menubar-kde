import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

ColumnLayout {
    id: root

    required property var history
    required property int percentageMode
    required property var backend

    readonly property bool hasHistory: !!history && history.length > 0
    readonly property bool hasEnoughHistory: !!history && history.length >= 2

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
            visible: root.hasHistory
            icon.name: "edit-clear-history"
            Accessible.name: i18n("Clear history")
            onClicked: root.backend.clearHistory()
        }
    }

    RowLayout {
        visible: !root.hasEnoughHistory
        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.smallSpacing
        Layout.bottomMargin: Kirigami.Units.smallSpacing
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Icon {
            source: "view-history"
            implicitWidth: Kirigami.Units.iconSizes.small
            implicitHeight: implicitWidth
            opacity: 0.65
        }

        PlasmaComponents.Label {
            Layout.fillWidth: true
            text: root.hasHistory
                ? i18n("Collecting history… the chart will appear after the next sample.")
                : i18n("History will appear after usage samples are collected.")
            opacity: 0.65
            font.pointSize: Kirigami.Theme.smallFont.pointSize
            wrapMode: Text.WordWrap
        }
    }

    Canvas {
        id: chart

        Layout.fillWidth: true
        Layout.preferredHeight: Kirigami.Units.gridUnit * 4
        visible: root.hasEnoughHistory

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()

        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            if (!root.hasEnoughHistory) {
                return
            }

            const left = 3
            const right = Math.max(left + 1, width - 3)
            const top = 5
            const bottom = Math.max(top + 1, height - 5)
            const firstTimestamp = Number(root.history[0].timestamp)
            const lastTimestamp = Number(root.history[root.history.length - 1].timestamp)
            const span = Math.max(1, lastTimestamp - firstTimestamp)

            const textColor = Kirigami.Theme.textColor
            ctx.lineWidth = 1
            ctx.strokeStyle = Qt.rgba(
                textColor.r,
                textColor.g,
                textColor.b,
                0.10
            )

            for (let percent = 0; percent <= 100; percent += 50) {
                const y = bottom - (percent / 100) * (bottom - top)
                ctx.beginPath()
                ctx.moveTo(left, y)
                ctx.lineTo(right, y)
                ctx.stroke()
            }

            function pointFor(sample, valueKey) {
                const x = left
                    + ((Number(sample.timestamp) - firstTimestamp) / span)
                    * (right - left)
                const percentage = Math.max(
                    0,
                    Math.min(100, root.shownPercent(Number(sample[valueKey])))
                )
                const y = bottom - (percentage / 100) * (bottom - top)
                return { x: x, y: y }
            }

            function drawSeries(hasKey, valueKey, color) {
                const points = []

                for (let i = 0; i < root.history.length; ++i) {
                    const sample = root.history[i]
                    if (sample[hasKey]) {
                        points.push(pointFor(sample, valueKey))
                    }
                }

                if (points.length === 0) {
                    return
                }

                ctx.lineWidth = 2
                ctx.strokeStyle = color
                ctx.lineJoin = "round"
                ctx.lineCap = "round"
                ctx.beginPath()
                ctx.moveTo(points[0].x, points[0].y)

                for (let i = 1; i < points.length; ++i) {
                    ctx.lineTo(points[i].x, points[i].y)
                }

                ctx.stroke()

                ctx.fillStyle = color
                for (let i = 0; i < points.length; ++i) {
                    ctx.beginPath()
                    ctx.arc(points[i].x, points[i].y, 2.5, 0, Math.PI * 2)
                    ctx.fill()
                }
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

    RowLayout {
        visible: root.hasEnoughHistory
        Layout.fillWidth: true
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
