#include "CodexBackend.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcessEnvironment>
#include <QStandardPaths>

#include <utility>

namespace
{
constexpr int MinRefreshSeconds = 15;
constexpr int MaxRefreshSeconds = 300;

bool isExecutableFile(const QString &path)
{
    const QFileInfo info(path);
    return info.exists() && info.isFile() && info.isExecutable();
}
}

CodexBackend::CodexBackend(QObject *parent)
    : QObject(parent)
{
    m_refreshTimer.setInterval(m_refreshIntervalSeconds * 1000);
    m_refreshTimer.setSingleShot(false);
    connect(&m_refreshTimer, &QTimer::timeout, this, &CodexBackend::refresh);

    m_reconnectTimer.setSingleShot(true);
    connect(
        &m_reconnectTimer,
        &QTimer::timeout,
        this,
        &CodexBackend::restartAfterFailure
    );

    connect(&m_process, &QProcess::started, this, &CodexBackend::handleStarted);
    connect(
        &m_process,
        &QProcess::readyReadStandardOutput,
        this,
        &CodexBackend::handleReadyRead
    );
    connect(
        &m_process,
        &QProcess::readyReadStandardError,
        this,
        &CodexBackend::handleStandardError
    );
    connect(
        &m_process,
        qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
        this,
        &CodexBackend::handleFinished
    );
    connect(
        &m_process,
        &QProcess::errorOccurred,
        this,
        &CodexBackend::handleProcessError
    );
}

CodexBackend::~CodexBackend()
{
    stop();
}

bool CodexBackend::connected() const
{
    return m_connected;
}

bool CodexBackend::loading() const
{
    return m_loading;
}

QString CodexBackend::errorString() const
{
    return m_errorString;
}

bool CodexBackend::hasFiveHour() const
{
    return m_fiveHour.valid;
}

double CodexBackend::fiveHourUsedPercent() const
{
    return m_fiveHour.usedPercent;
}

qint64 CodexBackend::fiveHourResetsAt() const
{
    return m_fiveHour.resetsAt;
}

bool CodexBackend::hasWeekly() const
{
    return m_weekly.valid;
}

double CodexBackend::weeklyUsedPercent() const
{
    return m_weekly.usedPercent;
}

qint64 CodexBackend::weeklyResetsAt() const
{
    return m_weekly.resetsAt;
}

bool CodexBackend::creditsReported() const
{
    return m_creditsReported;
}

bool CodexBackend::hasCredits() const
{
    return m_hasCredits;
}

bool CodexBackend::unlimitedCredits() const
{
    return m_unlimitedCredits;
}

QString CodexBackend::creditsBalance() const
{
    return m_creditsBalance;
}

QString CodexBackend::planType() const
{
    return m_planType;
}

QVariantList CodexBackend::additionalLimits() const
{
    return m_additionalLimits;
}

int CodexBackend::refreshIntervalSeconds() const
{
    return m_refreshIntervalSeconds;
}

void CodexBackend::setRefreshIntervalSeconds(int seconds)
{
    const int clamped = qBound(MinRefreshSeconds, seconds, MaxRefreshSeconds);
    if (clamped == m_refreshIntervalSeconds) {
        return;
    }

    m_refreshIntervalSeconds = clamped;
    m_refreshTimer.setInterval(m_refreshIntervalSeconds * 1000);
    Q_EMIT refreshIntervalSecondsChanged();
}

void CodexBackend::start()
{
    if (m_process.state() != QProcess::NotRunning) {
        return;
    }

    m_stopping = false;
    m_reconnectAttempts = 0;
    startProcess();
}

void CodexBackend::stop()
{
    m_stopping = true;
    m_refreshTimer.stop();
    m_reconnectTimer.stop();

    m_initialized = false;
    m_connected = false;
    Q_EMIT stateChanged();

    if (m_process.state() != QProcess::NotRunning) {
        m_process.kill();
    }
}

void CodexBackend::refresh()
{
    if (!m_initialized) {
        return;
    }

    m_loading = true;
    Q_EMIT stateChanged();
    sendRateLimitRead();
}

void CodexBackend::startProcess()
{
    const QString executable = resolveCodexExecutable();
    if (executable.isEmpty()) {
        setError(
            QStringLiteral(
                "Codex CLI was not found. Install Codex and make sure `codex` "
                "is available in PATH, or set CODEX_EXECUTABLE."
            )
        );
        m_loading = false;
        Q_EMIT stateChanged();
        return;
    }

    m_stdoutBuffer.clear();
    m_initialized = false;
    m_connected = false;
    m_loading = true;
    m_errorString.clear();
    m_nextRequestId = 3;
    Q_EMIT stateChanged();

    m_process.setProgram(executable);
    m_process.setArguments({QStringLiteral("app-server"), QStringLiteral("--stdio")});
    m_process.setProcessEnvironment(processEnvironment(executable));
    m_process.start();
}

void CodexBackend::handleStarted()
{
    sendInitialize();
}

void CodexBackend::handleReadyRead()
{
    m_stdoutBuffer.append(m_process.readAllStandardOutput());

    while (true) {
        const qsizetype newline = m_stdoutBuffer.indexOf('\n');
        if (newline < 0) {
            break;
        }

        QByteArray line = m_stdoutBuffer.left(newline).trimmed();
        m_stdoutBuffer.remove(0, newline + 1);

        if (line.isEmpty()) {
            continue;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(line, &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
            qWarning() << "Ignoring malformed Codex app-server output:"
                       << parseError.errorString();
            continue;
        }

        handleMessage(document.object());
    }
}

void CodexBackend::handleStandardError()
{
    const QByteArray output = m_process.readAllStandardError().trimmed();
    if (!output.isEmpty()) {
        qWarning().noquote() << "codex app-server:" << output;
    }
}

void CodexBackend::handleFinished(
    int exitCode,
    QProcess::ExitStatus exitStatus
)
{
    m_refreshTimer.stop();
    m_initialized = false;
    m_connected = false;

    if (!m_stopping) {
        setError(
            QStringLiteral("Codex app-server exited unexpectedly (code %1).")
                .arg(exitCode)
        );
        scheduleRestart();
    }

    if (exitStatus == QProcess::CrashExit) {
        qWarning() << "Codex app-server crashed";
    }

    Q_EMIT stateChanged();
}

void CodexBackend::handleProcessError(QProcess::ProcessError error)
{
    if (m_stopping) {
        return;
    }

    setError(
        QStringLiteral("Unable to run Codex app-server: %1")
            .arg(m_process.errorString())
    );

    if (error == QProcess::FailedToStart) {
        m_loading = false;
        scheduleRestart();
        Q_EMIT stateChanged();
    }
}

void CodexBackend::restartAfterFailure()
{
    if (!m_stopping && m_process.state() == QProcess::NotRunning) {
        startProcess();
    }
}

void CodexBackend::scheduleRestart()
{
    if (m_stopping || m_reconnectTimer.isActive()) {
        return;
    }

    const int exponent = qMin(m_reconnectAttempts, 5);
    const int delaySeconds = qMin(1 << exponent, 30);
    ++m_reconnectAttempts;
    m_reconnectTimer.start(delaySeconds * 1000);
}

void CodexBackend::setError(QString message)
{
    if (m_errorString == message) {
        return;
    }

    m_errorString = std::move(message);
    Q_EMIT stateChanged();
}

QString CodexBackend::resolveCodexExecutable() const
{
    const auto environment = QProcessEnvironment::systemEnvironment();
    const QString override =
        environment.value(QStringLiteral("CODEX_EXECUTABLE"));
    if (!override.isEmpty() && isExecutableFile(override)) {
        return override;
    }

    const QString fromPath =
        QStandardPaths::findExecutable(QStringLiteral("codex"));
    if (!fromPath.isEmpty()) {
        return fromPath;
    }

    const QString home = QDir::homePath();
    QStringList candidates = {
        home + QStringLiteral("/.codex/packages/standalone/current/bin/codex"),
        home + QStringLiteral("/.local/bin/codex"),
        home + QStringLiteral("/.npm-global/bin/codex"),
        home + QStringLiteral("/.volta/bin/codex"),
        home + QStringLiteral("/.asdf/shims/codex"),
        home + QStringLiteral("/.local/share/mise/shims/codex"),
        QStringLiteral("/usr/local/bin/codex"),
        QStringLiteral("/usr/bin/codex"),
    };

    const QDir nvmDirectory(home + QStringLiteral("/.nvm/versions/node"));
    const auto nvmVersions = nvmDirectory.entryList(
        QDir::Dirs | QDir::NoDotAndDotDot,
        QDir::Name | QDir::Reversed
    );
    for (const QString &version : nvmVersions) {
        candidates.append(
            nvmDirectory.absoluteFilePath(
                version + QStringLiteral("/bin/codex")
            )
        );
    }

    for (const QString &candidate : candidates) {
        if (isExecutableFile(candidate)) {
            return candidate;
        }
    }

    return {};
}

QProcessEnvironment CodexBackend::processEnvironment(
    const QString &executable
) const
{
    auto environment = QProcessEnvironment::systemEnvironment();

    QStringList pathEntries;
    pathEntries.append(QFileInfo(executable).absolutePath());

    const QString home = QDir::homePath();
    pathEntries.append({
        home + QStringLiteral("/.local/bin"),
        home + QStringLiteral("/.npm-global/bin"),
        home + QStringLiteral("/.volta/bin"),
        home + QStringLiteral("/.asdf/shims"),
        home + QStringLiteral("/.local/share/mise/shims"),
        QStringLiteral("/usr/local/bin"),
        QStringLiteral("/usr/bin"),
        QStringLiteral("/bin"),
    });

    const QString inheritedPath = environment.value(QStringLiteral("PATH"));
    if (!inheritedPath.isEmpty()) {
        pathEntries.append(inheritedPath.split(QLatin1Char(':')));
    }

    pathEntries.removeDuplicates();
    environment.insert(QStringLiteral("PATH"), pathEntries.join(QLatin1Char(':')));
    return environment;
}

void CodexBackend::sendMessage(const QJsonObject &message)
{
    if (m_process.state() != QProcess::Running) {
        return;
    }

    QByteArray payload = QJsonDocument(message).toJson(QJsonDocument::Compact);
    payload.append('\n');
    m_process.write(payload);
}

void CodexBackend::sendInitialize()
{
    QJsonObject clientInfo{
        {QStringLiteral("name"), QStringLiteral("codex-menubar-kde")},
        {QStringLiteral("title"), QStringLiteral("Codex MenuBar KDE")},
        {QStringLiteral("version"), QStringLiteral(CODEX_MENUBAR_KDE_VERSION)},
    };

    QJsonObject capabilities{
        {
            QStringLiteral("optOutNotificationMethods"),
            QJsonArray{QStringLiteral("remoteControl/status/changed")}
        },
    };

    sendMessage({
        {QStringLiteral("method"), QStringLiteral("initialize")},
        {QStringLiteral("id"), 1},
        {
            QStringLiteral("params"),
            QJsonObject{
                {QStringLiteral("clientInfo"), clientInfo},
                {QStringLiteral("capabilities"), capabilities},
            }
        },
    });
}

void CodexBackend::sendRateLimitRead()
{
    sendMessage({
        {QStringLiteral("method"), QStringLiteral("account/rateLimits/read")},
        {QStringLiteral("id"), m_nextRequestId++},
    });
}

void CodexBackend::handleMessage(const QJsonObject &message)
{
    const auto errorValue = message.value(QStringLiteral("error"));
    if (errorValue.isObject()) {
        const auto error = errorValue.toObject();
        setError(
            QStringLiteral("Codex RPC error: %1")
                .arg(error.value(QStringLiteral("message")).toString(
                    QStringLiteral("Unknown error")
                ))
        );
        m_loading = false;
        Q_EMIT stateChanged();
        return;
    }

    const auto idValue = message.value(QStringLiteral("id"));
    if (idValue.isDouble()) {
        const int id = idValue.toInt();

        if (id == 1 && !m_initialized) {
            sendMessage({
                {QStringLiteral("method"), QStringLiteral("initialized")},
            });

            m_initialized = true;
            m_connected = true;
            m_errorString.clear();
            m_reconnectAttempts = 0;
            m_refreshTimer.start();
            Q_EMIT stateChanged();

            sendRateLimitRead();
            return;
        }

        const auto resultValue = message.value(QStringLiteral("result"));
        if (resultValue.isObject()) {
            const auto parsed = RateLimitParser::parse(resultValue.toObject());
            if (parsed.has_value()) {
                applySummary(*parsed, false);
            }
        }

        m_loading = false;
        Q_EMIT stateChanged();
        return;
    }

    if (message.value(QStringLiteral("method")).toString()
        != QStringLiteral("account/rateLimits/updated")) {
        return;
    }

    const auto paramsValue = message.value(QStringLiteral("params"));
    if (!paramsValue.isObject()) {
        return;
    }

    const auto parsed = RateLimitParser::parse(paramsValue.toObject());
    if (parsed.has_value()) {
        applySummary(*parsed, true);
        m_loading = false;
        Q_EMIT stateChanged();
    }
}

void CodexBackend::applySummary(
    const RateLimitSummary &summary,
    bool preserveMetadata
)
{
    m_fiveHour = summary.fiveHour;
    m_weekly = summary.weekly;

    if (!preserveMetadata || summary.creditsReported) {
        m_creditsReported = summary.creditsReported;
        m_hasCredits = summary.hasCredits;
        m_unlimitedCredits = summary.unlimitedCredits;
        m_creditsBalance = summary.creditsBalance;
    }

    if (!preserveMetadata || !summary.planType.isEmpty()) {
        m_planType = summary.planType;
    }

    if (!preserveMetadata || !summary.additionalLimits.isEmpty()) {
        m_additionalLimits = summary.additionalLimits;
    }

    m_errorString.clear();
    Q_EMIT usageChanged();
}
