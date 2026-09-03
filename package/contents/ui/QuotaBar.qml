import QtQuick
import org.kde.kirigami as Kirigami

Item {
    id: root

    property real value: 0

    implicitHeight: Kirigami.Units.smallSpacing * 2

    Rectangle {
        anchors.fill: parent
        radius: height / 2
        color: Kirigami.Theme.alternateBackgroundColor
        opacity: 0.9
    }

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: parent.width * Math.max(0, Math.min(100, root.value)) / 100
        radius: height / 2
        color: Kirigami.Theme.highlightColor
        visible: width > 0
    }
}
