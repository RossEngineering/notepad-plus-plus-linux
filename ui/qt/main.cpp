#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QStringList>

#include "MainWindow.h"

#ifndef NPP_APP_VERSION
#define NPP_APP_VERSION "dev"
#endif

namespace {

struct StartupOptions {
    bool openRecent = false;
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

}  // namespace

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Notepad++ Linux"));
    app.setApplicationVersion(QStringLiteral(NPP_APP_VERSION));
    const StartupOptions startupOptions = ParseStartupOptions(app.arguments());
    MainWindow mainWindow;

    if (startupOptions.openRecent && startupOptions.filePaths.isEmpty()) {
        mainWindow.OpenMostRecentFileFromSession();
    }
    for (const QString &path : startupOptions.filePaths) {
        mainWindow.OpenPathFromCli(path);
    }

    mainWindow.show();
    return app.exec();
}
