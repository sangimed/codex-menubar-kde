import QtQuick
import org.kde.plasma.plasmoid

import io.github.sangimed.codexmenubarkde 1.0 as Codex

PlasmoidItem {
    id: root

    Plasmoid.icon: "codex-menubar-kde"

    toolTipMainText: i18n("Codex usage")
    toolTipSubText: codexBackend.connected
        ? i18n("Connected to the local Codex app-server")
        : (codexBackend.errorString || i18n("Connecting to Codex…"))

    preferredRepresentation: compactRepresentation

    Codex.CodexBackend {
        id: codexBackend
        refreshIntervalSeconds: Plasmoid.configuration.refreshInterval
        notificationsEnabled: Plasmoid.configuration.notificationsEnabled
        notificationThreshold: Plasmoid.configuration.notificationThreshold
    }

    compactRepresentation: CompactRepresentation {
        backend: codexBackend
        plasmoidItem: root
        percentageMode: Plasmoid.configuration.percentageMode
        displayMode: Plasmoid.configuration.displayMode
        showCredits: Plasmoid.configuration.showCredits
    }

    fullRepresentation: FullRepresentation {
        backend: codexBackend
        percentageMode: Plasmoid.configuration.percentageMode
    }

    Component.onCompleted: codexBackend.start()
    Component.onDestruction: codexBackend.stop()
}
