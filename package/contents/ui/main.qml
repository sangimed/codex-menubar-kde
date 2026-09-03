import QtQuick
import org.kde.plasma.plasmoid

import io.github.sangimed.codexmenubarkde 1.0 as Codex

PlasmoidItem {
    id: root

    toolTipMainText: i18n("Codex usage")
    toolTipSubText: backend.connected
        ? i18n("Connected to the local Codex app-server")
        : (backend.errorString || i18n("Connecting to Codex…"))

    preferredRepresentation: compactRepresentation

    Codex.CodexBackend {
        id: backend
        refreshIntervalSeconds: Plasmoid.configuration.refreshInterval
    }

    compactRepresentation: CompactRepresentation {
        backend: backend
        plasmoidItem: root
        percentageMode: Plasmoid.configuration.percentageMode
        displayMode: Plasmoid.configuration.displayMode
        showCredits: Plasmoid.configuration.showCredits
    }

    fullRepresentation: FullRepresentation {
        backend: backend
        percentageMode: Plasmoid.configuration.percentageMode
    }

    Component.onCompleted: backend.start()
    Component.onDestruction: backend.stop()
}
