#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QCloseEvent>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

private:
    void closeEvent(QCloseEvent *event);
};

#endif // MAINWINDOW_H
