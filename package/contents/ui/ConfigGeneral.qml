import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.FormLayout {
    id: page

    property alias cfg_percentageMode: percentageMode.currentIndex
    property alias cfg_displayMode: displayMode.currentIndex
    property alias cfg_showCredits: showCredits.checked
    property alias cfg_refreshInterval: refreshInterval.value

    QQC2.ComboBox {
        id: percentageMode
        Kirigami.FormData.label: i18n("Percentages:")
        model: [
            i18n("Remaining"),
            i18n("Used")
        ]
    }

    QQC2.ComboBox {
        id: displayMode
        Kirigami.FormData.label: i18n("Panel display:")
        model: [
            i18n("5-hour and weekly"),
            i18n("5-hour only"),
            i18n("Weekly only"),
            i18n("Icon only")
        ]
    }

    QQC2.CheckBox {
        id: showCredits
        Kirigami.FormData.label: i18n("Credits:")
        text: i18n("Show credit balance in the panel")
    }

    QQC2.SpinBox {
        id: refreshInterval
        Kirigami.FormData.label: i18n("Fallback refresh:")
        from: 15
        to: 300
        stepSize: 5
        editable: true
        textFromValue: function(value) {
            return i18n("%1 seconds", value)
        }
    }
}
