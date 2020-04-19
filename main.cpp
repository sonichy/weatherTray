#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    qSetMessagePattern("[ %{file}: %{line} ] %{message}");
    QApplication app(argc, argv);
    app.setOrganizationName("HTY");
    app.setApplicationName("WeatherTray");
    QApplication::setQuitOnLastWindowClosed(false); //关闭最后一个窗口不退出程序
    MainWindow *mainWindow = new MainWindow;
    Q_UNUSED(mainWindow);
    return app.exec();
}