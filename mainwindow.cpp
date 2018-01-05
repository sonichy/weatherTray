#include "mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
      : QMainWindow(parent)
{

}

void MainWindow::closeEvent(QCloseEvent *event)//此函数在QWidget关闭时执行
{
    hide();
    //不退出App
    event->ignore();
}
