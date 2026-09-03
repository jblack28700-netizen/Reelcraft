#include <QApplication>

#include "application/Application.h"
#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Application application;
    application.initialize();

    MainWindow window;
    window.show();

    return app.exec();
}
