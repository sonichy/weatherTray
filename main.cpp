#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qSetMessagePattern("[ %{file}: %{line} ] %{message}");
    //关闭最后一个窗口不退出程序
    QApplication::setQuitOnLastWindowClosed(false);
    MainWindow *window = new MainWindow;
    Q_UNUSED(window);
    return app.exec();
}