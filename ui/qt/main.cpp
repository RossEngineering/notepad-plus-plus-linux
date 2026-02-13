#include <QApplication>

#include "MainWindow.h"

#ifndef NPP_APP_VERSION
#define NPP_APP_VERSION "dev"
#endif

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Notepad++ Linux"));
    app.setApplicationVersion(QStringLiteral(NPP_APP_VERSION));
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
