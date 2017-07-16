// QtScript解析JSON：http://blog.csdn.net/ccf19881030/article/details/12527607
// 获取JSON子对象：http://www.07net01.com/program/306154.html

#include "mainwindow.h"
#include <QMessageBox>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <QtNetwork>
#include <QtScript> // >=Qt4.3
#include <QGridLayout>
#include <QLabel>
#include <QSystemTrayIcon>
#include <QIcon>
#include <QAction>
#include <QMenu>
#include <QStyle>
#include <QApplication>
#include <QDesktopWidget>
QString city="",cityId="",swn="",sw1="";
QLabel *labelTemp,*labelCity,*labelSD,*labelWind,*labelPM,*labelAQI,*labelRT,*labelDate[7],*labelWImg[7],*labelWeather[7];
//QGridLayout *layout;
QSystemTrayIcon *systray;
QAction *forecastAction,*refreshAction,*backgroundAction,*aboutAction,*quitAction;

MainWindow::MainWindow(QWidget *parent)
      : QMainWindow(parent)
{
    //托盘菜单
    systray=new QSystemTrayIcon();
    systray->setToolTip("托盘天气");
    systray->setIcon(QIcon(":/icon.ico"));
    systray->setVisible(true);
    QMenu *traymenu=new QMenu();
    forecastAction=new QAction("预报",traymenu);
    QStyle* style = QApplication::style();
    QIcon icon = style->standardIcon(QStyle::SP_ComputerIcon);
    forecastAction->setIcon(icon);
    refreshAction=new QAction("刷新",traymenu);
    icon = style->standardIcon(QStyle::SP_DriveNetIcon);
    refreshAction->setIcon(icon);
    backgroundAction=new QAction("背景透明",traymenu);
    backgroundAction->setCheckable(true);
    aboutAction=new QAction("关于",traymenu);
    icon = style->standardIcon(QStyle::SP_MessageBoxInformation);
    aboutAction->setIcon(icon);
    quitAction=new QAction("退出",traymenu);
    icon = style->standardIcon(QStyle::SP_DialogCloseButton);
    quitAction->setIcon(icon);
    traymenu->addAction(forecastAction);
    traymenu->addAction(refreshAction);
    traymenu->addAction(backgroundAction);
    traymenu->addAction(aboutAction);
    traymenu->addAction(quitAction);
    systray->setContextMenu(traymenu);
    systray->show();
    connect(systray , SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconIsActived(QSystemTrayIcon::ActivationReason)));
    connect(forecastAction, SIGNAL(triggered()),this, SLOT(windowForecast()));
    connect(refreshAction, SIGNAL(triggered()),this, SLOT(getWeather()));
    connect(backgroundAction, SIGNAL(toggled(bool)),this, SLOT(changeBackground(bool)));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(windowAbout()));
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    // 报错：Attempting to set QLayout "" on MainWindow "", which already has a layout
    QWidget *widget=new QWidget;
    this->setCentralWidget(widget);
    QGridLayout *layout = new QGridLayout;
    labelCity = new QLabel("城市");
    labelCity->setAlignment(Qt::AlignCenter);
    layout->addWidget(labelCity,0,0);
    labelTemp = new QLabel("温度\n?");
    labelTemp->setStyleSheet("font-size:40px");
    labelTemp->setAlignment(Qt::AlignCenter);
    layout->addWidget(labelTemp,0,1);
    labelSD = new QLabel("湿度\n?");
    labelSD->setAlignment(Qt::AlignCenter);
    layout->addWidget(labelSD,0,2);
    labelWind = new QLabel("风向?\n风力?");
    labelWind->setAlignment(Qt::AlignCenter);
    layout->addWidget(labelWind,0,3);
    labelPM = new QLabel("PM2.5\n?");
    labelPM->setAlignment(Qt::AlignCenter);
    layout->addWidget(labelPM,0,4);
    labelAQI = new QLabel("空气质量指数\n?");
    labelAQI->setAlignment(Qt::AlignCenter);
    layout->addWidget(labelAQI,0,5);
    labelRT = new QLabel("刷新\n?");
    labelRT->setAlignment(Qt::AlignCenter);
    layout->addWidget(labelRT,0,6);
    for(int i=1;i<8;i++){
        labelDate[i-1] = new QLabel("1月1日\n星期一");
        labelDate[i-1]->setAlignment(Qt::AlignCenter);
        layout->addWidget(labelDate[i-1],1,i-1);
        labelWImg[i-1] = new QLabel("");
        QPixmap pixmap;
        pixmap.load(":/icon.ico");
        labelWImg[i-1]->setPixmap(pixmap);
        labelWImg[i-1]->setAlignment(Qt::AlignCenter);
        layout->addWidget(labelWImg[i-1],2,i-1);
        labelWeather[i-1] = new QLabel("晴\n15°C ~ 20°C\n北风1级");
        labelWeather[i-1]->setAlignment(Qt::AlignCenter);
        layout->addWidget(labelWeather[i-1],3,i-1);
    }
    widget->setLayout(layout);

    QTimer *timer=new QTimer();
    timer->setInterval(1800000);
    //timer->setInterval(60000);
    timer->start();
    QObject::connect(timer,SIGNAL(timeout()),this, SLOT(getWeather()));
    getWeather();
}

//MainWindow::~MainWindow()
//{
//}

void MainWindow::closeEvent(QCloseEvent *event)//此函数在QWidget关闭时执行
{
    this->hide();
    //不退出App
    event->ignore();
}


void MainWindow::changeBackground(bool on){
    qDebug() << on;
    setAttribute(Qt::WA_TranslucentBackground,on);
}

void MainWindow::windowForecast(){
    QDesktopWidget* desktop = QApplication::desktop();
    this->hide();
    this->move((desktop->width() - this->width())/2, (desktop->height() - this->height())/2);
    this->show();
    this->setWindowState(Qt::WindowActive);
    this->activateWindow();

}

void MainWindow::windowAbout(){    
    QMessageBox aboutMB(QMessageBox::NoIcon, "关于", "中国天气预报 2.1\n一款基于Qt的天气预报程序。\n作者：黄颖\nE-mail: sonichy@163.com\n主页：sonichy.96.lt\n致谢：\nsina.com\nweidu.com\nlinux028@deepin.org\n\n2.1 (2016-12-09)\n1.窗体始终居中。\n\n2.0 (2016-11-21)\n1.使用QScript库代替QJsonDocument解析JSON，开发出兼容Qt4的版本。\n2.单击图标弹出实时天气消息，弥补某些系统不支持鼠标悬浮信息的不足。\n3.由于QScriptValueIterator解析不了某个JSON，使用QScriptValue.property.property代替。\n4.托盘右键增加一个刷新菜单。\n\n1.0 (2016-11-17)\n1.动态修改天气栏托盘图标，鼠标悬浮显示实时天气，点击菜单弹出窗口显示7天天气预报。\n2.每30分钟自动刷新一次。\n3.窗体透明。");
    aboutMB.setIconPixmap(QPixmap(":/icon.ico"));
    aboutMB.exec();
}

void MainWindow::iconIsActived(QSystemTrayIcon::ActivationReason reason)
{
    //qDebug() << reason;
    switch(reason)
    {
    case QSystemTrayIcon::Trigger:
    {
        systray->showMessage("实时天气", swn, QSystemTrayIcon::MessageIcon::Information, 9000);
        //systray->showMessage("实时天气", swn, QSystemTrayIcon::Information, 9000);
        break;
    }
    case QSystemTrayIcon::DoubleClick:
        break;
    default:
        break;
    }
}

void MainWindow::getWeather(){
    QDateTime currentDateTime = QDateTime::currentDateTime();
    qDebug() << "getWeather()" << currentDateTime;
    QString URLSTR = "http://int.dpool.sina.com.cn/iplookup/iplookup.php?format=json";
    QUrl url(URLSTR);
    QNetworkAccessManager manager;
    QEventLoop loop;
    QNetworkReply *reply;
    reply = manager.get(QNetworkRequest(url));
    //请求结束并下载完成后，退出子事件循环
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    //开启子事件循环
    loop.exec();
    QString codeContent = reply->readAll();
    qDebug() << URLSTR;
    qDebug() << codeContent;
    QScriptEngine engine;
    QScriptValue sc = engine.evaluate("value="+codeContent);
    city = sc.property("city").toString();
    labelCity->setText(city);

    URLSTR="http://hao.weidunewtab.com/tianqi/city.php?city="+city;
    url.setUrl(URLSTR);
    reply = manager.get(QNetworkRequest(url));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    cityId = reply->readAll();
    qDebug() << URLSTR + " => " << cityId;

    URLSTR="http://hao.weidunewtab.com/myapp/weather/data/index.php?cityID="+cityId;
    url.setUrl(URLSTR);
    reply = manager.get(QNetworkRequest(url));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    codeContent = reply->readAll();
    qDebug() << URLSTR;
    qDebug() << codeContent;
    sc = engine.evaluate("value="+codeContent);    
    sw1 =sc.property("weatherinfo").property("weather1").toString();
    /*
    URLSTR="http://m.weather.com.cn/weather_img/"+ QString::number(sc.property("weatherinfo").property("img1").toNumber()) + ".gif";
    qDebug() << URLSTR;
    url.setUrl(URLSTR);
    reply = manager.get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    QPixmap pixmap;
    pixmap.loadFromData(reply->readAll());
    systray->setIcon(QIcon(pixmap));
    */
    QImage image;
    URLSTR="images/"+ QString::number(sc.property("weatherinfo").property("img1").toNumber()) + ".png";
    image.load(URLSTR);
    systray->setIcon(QIcon(QPixmap::fromImage(image)));

    QDateTime date = QDateTime::fromString(sc.property("weatherinfo").property("date_y").toString(), "yyyy年M月d");
    for(int i=1;i<8;i++){
        labelDate[i-1]->setText(date.addDays(i-1).toString("M-d")+"\n"+date.addDays(i-1).toString("dddd"));
        labelDate[i-1]->setAlignment(Qt::AlignCenter);
        /*
        URLSTR="http://m.weather.com.cn/weather_img/"+ QString::number(sc.property("weatherinfo").property("img"+QString::number(2*i-1)).toNumber()) + ".gif";
        url.setUrl(URLSTR);
        reply = manager.get(QNetworkRequest(url));
        QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();        
        QPixmap pixmap;
        pixmap.loadFromData(reply->readAll());
        labelWImg[i-1]->setPixmap(pixmap);
        labelWImg[i-1]->setAlignment(Qt::AlignCenter);
        */
        URLSTR="images/"+ QString::number(sc.property("weatherinfo").property("img"+QString::number(2*i-1)).toNumber()) + ".png";
        image.load(URLSTR);
        labelWImg[i-1]->setPixmap(QPixmap::fromImage(image.scaled(50,50)));
        labelWImg[i-1]->setAlignment(Qt::AlignCenter);

        labelWeather[i-1]->setText(sc.property("weatherinfo").property("weather"+QString::number(i)).toString()+ "\n" + sc.property("weatherinfo").property("temp"+QString::number(i)).toString() + "\n" + sc.property("weatherinfo").property("wind"+QString::number(i)).toString());
        labelWeather[i-1]->setAlignment(Qt::AlignCenter);
    }

    URLSTR="http://hao.weidunewtab.com/myapp/weather/data/indexInTime.php?cityID="+cityId;
    url.setUrl(URLSTR);
    reply = manager.get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    codeContent = reply->readAll();
    qDebug() << URLSTR;
    qDebug() << codeContent;
    sc = engine.evaluate("value="+codeContent);
    swn = sc.property("weatherinfo").property("city").toString() + "\n" + sw1 + "\n" + sc.property("weatherinfo").property("temp").toString() + "°C\n湿度：" + sc.property("weatherinfo").property("SD").toString() + "\n" + sc.property("weatherinfo").property("WD").toString() + sc.property("weatherinfo").property("WS").toString() + "\nPM2.5：" + sc.property("weatherinfo").property("pm25").toString() + "\n空气质量指数：" + QString::number(sc.property("weatherinfo").property("aqiLevel").toNumber()) + "\n刷新：" + currentDateTime.toString("HH:mm:ss");
    systray->setToolTip(swn);
    labelTemp->setText(sc.property("weatherinfo").property("temp").toString()+"°C");
    labelSD->setText("湿度\n" + sc.property("weatherinfo").property("SD").toString());
    labelWind->setText(sc.property("weatherinfo").property("WD").toString() + "\n" + sc.property("weatherinfo").property("WS").toString());
    labelPM->setText("PM2.5\n" + sc.property("weatherinfo").property("pm25").toString());
    labelAQI->setText("空气质量指数\n" + QString::number(sc.property("weatherinfo").property("aqiLevel").toNumber()));
    labelRT->setText("刷新\n" + currentDateTime.toString("HH:mm:ss"));

}
