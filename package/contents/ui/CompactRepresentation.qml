import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.plasmoid

Item {
    id: compact

    required property var backend
    required property int percentageMode
    required property int displayMode
    required property bool showCredits

    Layout.minimumWidth: content.implicitWidth + Kirigami.Units.smallSpacing * 2
    Layout.preferredWidth: Layout.minimumWidth
    Layout.fillHeight: true

    function shownPercent(usedPercent) {
        return Math.round(percentageMode === 1 ? usedPercent : 100 - usedPercent)
    }

    function summaryText() {
        if (displayMode === 3) {
            return ""
        }

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
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Icon {
            visible: compact.displayMode === 3
            source: "applications-development"
            implicitWidth: Kirigami.Units.iconSizes.smallMedium
            implicitHeight: implicitWidth
        }

        PlasmaComponents.Label {
            visible: compact.displayMode !== 3
            text: compact.summaryText()
            textFormat: Text.PlainText
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        cursorShape: Qt.PointingHandCursor
        onClicked: plasmoid.expanded = !plasmoid.expanded
    }
}
