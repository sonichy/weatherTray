#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QLabel>
#include <QSystemTrayIcon>
#include <QSettings>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    QString city="", cityID="", swn="", sw0="", icon_path0="";

private:
    QLabel *labelTemp, *labelCity, *labelSD, *labelWind, *labelPM, *labelAQI, *labelUT, *labelDate[15], *labelWImg[15], *labelWeather[15], *labelComment;
    QSystemTrayIcon *systray;
    QString path;
    QSettings settings;

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void getWeather();
    void showForecast();
    void set();
    void about();
    void changelog();
    void trayActivated(QSystemTrayIcon::ActivationReason reason);

};

#endif // MAINWINDOW_H