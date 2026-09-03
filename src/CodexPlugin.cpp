#include "CodexBackend.h"

#include <QByteArray>
#include <QQmlExtensionPlugin>
#include <qqml.h>

class CodexMenuBarKdePlugin final : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(
            QByteArray(uri)
            == QByteArrayLiteral("io.github.sangimed.codexmenubarkde")
        );

        qmlRegisterType<CodexBackend>(uri, 1, 0, "CodexBackend");
    }
};

#include "CodexPlugin.moc"
