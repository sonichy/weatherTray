#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QCloseEvent>
#include <QLabel>
#include <QGridLayout>
#include <QSystemTrayIcon>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    QString city="", cityID="", swn="", sw0="", icon_path0="";

private:
    QLabel *labelTemp, *labelCity, *labelSD, *labelWind, *labelPM, *labelAQI, *labelUT, *labelDate[5], *labelWImg[5], *labelWeather[5], *labelComment;
    QSystemTrayIcon *systray;
    void closeEvent(QCloseEvent *event);

private slots:
    void getWeather();
    void showForecast();
    void about();
    void trayActivated(QSystemTrayIcon::ActivationReason reason);
};

#endif // MAINWINDOW_H