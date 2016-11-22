#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QCloseEvent>
#include <QSystemTrayIcon>
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    //~MainWindow();
    void closeEvent(QCloseEvent *event);    

private slots:
    void windowAbout();
    void windowForecast();
    void getWeather();
    void iconIsActived(QSystemTrayIcon::ActivationReason);
};

#endif // MAINWINDOW_H
