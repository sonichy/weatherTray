// 获取网页源码写入文件: http://blog.csdn.net/small_qch/article/details/7200271

#include "mainwindow.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QtCore>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // <Qt5解决中文乱码
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForTr(codec);
    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());

    // 关闭最后一个窗口不退出程序
    QApplication::setQuitOnLastWindowClosed(false);
    MainWindow *window=new MainWindow;
    window->setWindowTitle("中国天气预报");
    window->setFixedSize(600,240);

    // 水平垂直居中
    //QDesktopWidget* desktop = QApplication::desktop();
    //window->move((desktop->width() - window->width())/2, (desktop->height() - window->height())/2);

    // 移除最小化
    window->setWindowFlags((window->windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMinimizeButtonHint );
    // 不在任务栏显示
    window->setWindowFlags(Qt::Tool);
    // 隐藏标题栏
    //window->setWindowFlags(Qt::FramelessWindowHint);
    // 背景透明
    //window->setAttribute(Qt::WA_TranslucentBackground, true);
    // 窗体透明
    window->setWindowOpacity(0.9);

    return app.exec();
}
