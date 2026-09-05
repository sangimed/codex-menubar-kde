import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

Item {
    id: compact

    required property var backend
    required property var plasmoidItem
    required property int percentageMode
    required property int displayMode
    required property bool showCredits

    implicitWidth: content.implicitWidth + Kirigami.Units.smallSpacing * 2
    implicitHeight: Math.max(
        Kirigami.Units.iconSizes.smallMedium,
        content.implicitHeight
    )

    Layout.minimumWidth: implicitWidth
    Layout.preferredWidth: implicitWidth
    Layout.minimumHeight: implicitHeight
    Layout.fillHeight: true

    function shownPercent(usedPercent) {
        return Math.round(percentageMode === 1 ? usedPercent : 100 - usedPercent)
    }

    function summaryText() {
        const parts = []

        if ((displayMode === 0 || displayMode === 1) && backend.hasFiveHour) {
            parts.push(shownPercent(backend.fiveHourUsedPercent) + "%")
        }

        if ((displayMode === 0 || displayMode === 2) && backend.hasWeekly) {
            parts.push("W" + shownPercent(backend.weeklyUsedPercent) + "%")
        }

        if (showCredits && backend.creditsReported && backend.hasCredits) {
            parts.push(backend.unlimitedCredits ? "∞" : backend.creditsBalance)
        }

        if (parts.length > 0) {
            return parts.join(" · ")
        }

        if (backend.loading) {
            return i18n("Codex…")
        }

        return backend.connected ? i18n("Codex") : i18n("Codex !")
    }

    RowLayout {
        id: content
        anchors.centerIn: parent
        spacing: Math.max(3, Kirigami.Units.smallSpacing - 1)

        Kirigami.Icon {
            source: Qt.resolvedUrl("../images/codex-menubar-kde.svg")
            implicitWidth: Kirigami.Units.iconSizes.small
            implicitHeight: implicitWidth
            Layout.alignment: Qt.AlignVCenter
        }

        PlasmaComponents.Label {
            visible: compact.displayMode !== 3
            text: compact.summaryText()
            textFormat: Text.PlainText
            verticalAlignment: Text.AlignVCenter
            Layout.alignment: Qt.AlignVCenter
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        cursorShape: Qt.PointingHandCursor
        onClicked: compact.plasmoidItem.expanded = !compact.plasmoidItem.expanded
    }
}
