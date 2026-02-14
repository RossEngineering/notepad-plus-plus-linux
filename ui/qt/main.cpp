#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStringList>
#include <QStandardPaths>
#include <QTimer>
#include <QtGlobal>

#include <chrono>

#include "MainWindow.h"

#ifndef NPP_APP_VERSION
#define NPP_APP_VERSION "dev"
#endif

namespace {

struct StartupOptions {
    bool openRecent = false;
    bool safeModeNoExtensions = false;
    bool startupTraceEnabled = false;
    QString startupTracePath;
    QStringList filePaths;
};

StartupOptions ParseStartupOptions(const QStringList &arguments) {
    StartupOptions options;
    for (int index = 1; index < arguments.size(); ++index) {
        const QString argument = arguments.at(index);
        if (argument == QStringLiteral("--open-recent")) {
            options.openRecent = true;
            continue;
        }
        if (argument == QStringLiteral("--new-file")) {
            continue;
        }
        if (argument == QStringLiteral("--safe-mode") ||
            argument == QStringLiteral("--safe-mode-no-extensions")) {
            options.safeModeNoExtensions = true;
            continue;
        }
        if (argument == QStringLiteral("--startup-trace")) {
            options.startupTraceEnabled = true;
            continue;
        }
        if (argument.startsWith(QStringLiteral("--startup-trace="))) {
            options.startupTraceEnabled = true;
            options.startupTracePath = argument.mid(QStringLiteral("--startup-trace=").size());
            continue;
        }
        if (argument.startsWith(QLatin1Char('-'))) {
            continue;
        }

        QFileInfo fileInfo(argument);
        QString normalizedPath;
        if (fileInfo.isAbsolute()) {
            normalizedPath = QDir::cleanPath(fileInfo.absoluteFilePath());
        } else {
            normalizedPath = QDir::cleanPath(QDir::current().absoluteFilePath(argument));
        }
        options.filePaths.push_back(normalizedPath);
    }
    return options;
}

QString ResolveStartupTracePath(const QString &configuredPath) {
    if (!configuredPath.trimmed().isEmpty()) {
        QFileInfo configured(configuredPath);
        if (configured.isAbsolute()) {
            return configured.absoluteFilePath();
        }
        return QDir::current().absoluteFilePath(configuredPath);
    }

    QString base;
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
    base = QStandardPaths::writableLocation(QStandardPaths::StateLocation);
#endif
    if (base.isEmpty()) {
        base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    }
    if (base.isEmpty()) {
        base = QDir::currentPath();
    }
    QDir traceDir(base);
    traceDir.mkpath(QStringLiteral("traces"));
    const QString stamp = QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz"));
    return traceDir.absoluteFilePath(QStringLiteral("traces/startup-trace-%1.json").arg(stamp));
}

}  // namespace

int main(int argc, char *argv[]) {
    using Clock = std::chrono::steady_clock;
    const auto startupBegin = Clock::now();
    QJsonArray startupTraceEvents;
    const auto addTraceEvent = [&](const char *name) {
        const auto now = Clock::now();
        const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - startupBegin).count();
        QJsonObject event;
        event.insert(QStringLiteral("event"), QString::fromUtf8(name));
        event.insert(QStringLiteral("elapsedMs"), static_cast<qint64>(elapsedMs));
        startupTraceEvents.append(event);
    };

    QApplication app(argc, argv);
    addTraceEvent("qapplication_initialized");
    app.setApplicationName(QStringLiteral("Notepad++ Linux"));
    app.setApplicationVersion(QStringLiteral(NPP_APP_VERSION));
    const StartupOptions startupOptions = ParseStartupOptions(app.arguments());
    addTraceEvent("startup_options_parsed");
    MainWindow mainWindow(startupOptions.safeModeNoExtensions);
    addTraceEvent("main_window_constructed");

    if (startupOptions.openRecent && startupOptions.filePaths.isEmpty()) {
        mainWindow.OpenMostRecentFileFromSession();
    }
    addTraceEvent("startup_open_recent_processed");
    for (const QString &path : startupOptions.filePaths) {
        mainWindow.OpenPathFromCli(path);
    }
    addTraceEvent("startup_cli_paths_processed");

    mainWindow.show();
    addTraceEvent("main_window_shown");

    if (startupOptions.startupTraceEnabled) {
        const QString tracePath = ResolveStartupTracePath(startupOptions.startupTracePath);
        const qint64 argumentCount = app.arguments().size();
        QTimer::singleShot(0, &app, [startupTraceEvents, tracePath, argumentCount, startupBegin]() mutable {
            QJsonObject root;
            root.insert(QStringLiteral("schemaVersion"), 1);
            root.insert(QStringLiteral("capturedAtUtc"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
            root.insert(
                QStringLiteral("appVersion"),
                QCoreApplication::applicationVersion().trimmed().isEmpty()
                    ? QStringLiteral("dev")
                    : QCoreApplication::applicationVersion().trimmed());
            root.insert(QStringLiteral("argumentCount"), argumentCount);

            const auto eventLoopNow = Clock::now();
            const auto eventLoopElapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                eventLoopNow - startupBegin).count();
            QJsonObject tickEvent;
            tickEvent.insert(QStringLiteral("event"), QStringLiteral("first_event_loop_tick"));
            tickEvent.insert(QStringLiteral("elapsedMs"), static_cast<qint64>(eventLoopElapsedMs));
            startupTraceEvents.append(tickEvent);
            root.insert(QStringLiteral("eventLoopTickElapsedMs"), static_cast<qint64>(eventLoopElapsedMs));
            root.insert(QStringLiteral("events"), startupTraceEvents);

            QSaveFile file(tracePath);
            if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                const QByteArray json = QJsonDocument(root).toJson(QJsonDocument::Indented);
                file.write(json);
                file.commit();
            }
        });
    }

    return app.exec();
}
