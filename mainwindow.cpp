#include "mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
      : QMainWindow(parent)
{

}

void MainWindow::closeEvent(QCloseEvent *event)//此函数在QWidget关闭时执行
{
    this->hide();
    //不退出App
    event->ignore(); // 报错：invalid use of incomplete type 'class QCloseEvent'，头文件里没有：#include <QCloseEvent>
}
