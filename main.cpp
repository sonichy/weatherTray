// 获取网页源码写入文件: http://blog.csdn.net/small_qch/article/details/7200271
// JOSN解析: http://blog.sina.com.cn/s/blog_a6fb6cc90101gnxm.html
#include "mainwindow.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QtCore>
#include <QtNetwork>
#include <QLabel>
#include <QJsonDocument>    //>=Qt 5.0
#include <QSystemTrayIcon>
#include <QIcon>
#include <QAction>
#include <QMenu>
#include <QStyle>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMessageBox>
QString city="",cityId="",swn="",swf="";
QAction *forecastAction,*refreshAction,*aboutAction,*quitAction;
QLabel *labelTemp,*labelCity,*labelSD,*labelWind,*labelPM,*labelAQI,*labelRT,*labelDate[7],*labelWImg[7],*labelWeather[7];
QGridLayout *layout;
QSystemTrayIcon *systray;
MainWindow *window;

void windowForecast(){
    window->hide();
    window->show();
    //this->setWindowState(Qt::WindowActive);
    //this->activateWindow();    
}

void windowAbout(){
    QMessageBox aboutMB(QMessageBox::NoIcon, "关于", "中国天气预报 2.0\n一款基于Qt的天气预报程序。\n作者：黄颖\nE-mail: sonichy@163.com\n主页：sonichy.96.lt\n致谢：\nsina.com\nweidu.com\nlinux028@deepin.org\n\n2.0 (2016-11-21)\n1.使用QScript库代替QJsonDocument解析JSON，开发出兼容Qt4的版本。\n2.单击图标弹出实时天气消息，弥补某些系统不支持鼠标悬浮信息的不足。\n3.由于QScriptValueIterator.value().property解析不了某个JSON，使用QScriptValue.property.property代替。\n4.托盘右键增加一个刷新菜单。\n\n1.0 (2016-11-17)\n1.动态修改天气栏托盘图标，鼠标悬浮显示实时天气，点击菜单弹出窗口显示7天天气预报。\n2.每30分钟自动刷新一次。\n3.窗体透明。");
    aboutMB.setIconPixmap(QPixmap(":/icon.ico"));
    aboutMB.exec();
}

void iconIsActived(QSystemTrayIcon::ActivationReason reason)
{
    //qDebug() << reason;
    switch(reason)
    {
    case QSystemTrayIcon::Trigger:
    {
        systray->showMessage("实时天气", swn, QSystemTrayIcon::MessageIcon::Information, 8000);
        break;
    }
    case QSystemTrayIcon::DoubleClick:
        break;
    default:
        break;
    }
}

void getWeather(){
    QDateTime currentDateTime = QDateTime::currentDateTime();
    qDebug() << "getWeather()" << currentDateTime;
    QString URLSTR = "http://int.dpool.sina.com.cn/iplookup/iplookup.php?format=json";
    QUrl url(URLSTR);
    QNetworkAccessManager manager;
    QEventLoop loop;
    QNetworkReply *reply;

    reply = manager.get(QNetworkRequest(url));
    //请求结束并下载完成后，退出子事件循环
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    //开启子事件循环
    loop.exec();
    QString codeContent = reply->readAll();
    qDebug() << URLSTR + " -> " << codeContent;
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(codeContent.toLatin1(), &json_error);
    if(json_error.error == QJsonParseError::NoError)
    {
        if(parse_doucment.isObject())
        {
            QJsonObject obj = parse_doucment.object();
            if(obj.contains("city"))
            {
                QJsonValue city_value = obj.take("city");
                if(city_value.isString())
                {
                    city = city_value.toString();
                    labelCity->setText(city);
                }
            }
        }
    }

    URLSTR="http://hao.weidunewtab.com/tianqi/city.php?city="+city;
    url.setUrl(URLSTR);
    reply = manager.get(QNetworkRequest(url));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    cityId = reply->readAll();
    qDebug() << URLSTR + " => " << cityId;

    URLSTR="http://hao.weidunewtab.com/myapp/weather/data/indexInTime.php?cityID="+cityId;
    url.setUrl(URLSTR);
    reply = manager.get(QNetworkRequest(url));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    codeContent = reply->readAll();
    qDebug() << URLSTR;
    qDebug() << codeContent;
    parse_doucment = QJsonDocument::fromJson(codeContent.toLatin1(), &json_error);
    if(json_error.error == QJsonParseError::NoError)
    {
        if(parse_doucment.isObject())
        {
            QJsonObject obj = parse_doucment.object();
            if(obj.contains("weatherinfo")){
                QJsonObject::iterator it;
                it = obj.find("weatherinfo");
                QJsonObject weatherinfoObj = it.value().toObject();
                swn = "城市："+weatherinfoObj.value("city").toString()+"\n温度："+weatherinfoObj.value("temp").toString()+"°C\n湿度："+weatherinfoObj.value("SD").toString()+"\n风力："+weatherinfoObj.value("WD").toString()+weatherinfoObj.value("WS").toString()+"\nPM2.5："+weatherinfoObj.value("pm25").toString()+"\n空气质量指数："+ QString::number(weatherinfoObj.value("aqiLevel").toInt())+"\n刷新："+currentDateTime.toString("HH:mm:ss");
                qDebug() << swn;
                systray->setToolTip(swn);                
                labelTemp->setText("温度\n"+weatherinfoObj.value("temp").toString()+"°C");
                labelSD->setText("湿度\n"+weatherinfoObj.value("SD").toString());
                labelWind->setText(weatherinfoObj.value("WD").toString()+"\n"+weatherinfoObj.value("WS").toString());
                labelPM->setText("PM2.5\n"+weatherinfoObj.value("pm25").toString());
                labelAQI->setText("空气质量指数\n"+QString::number(weatherinfoObj.value("aqiLevel").toInt()));
                labelRT->setText("刷新\n"+currentDateTime.toString("HH:mm:ss"));
            }
        }
    }

    URLSTR="http://hao.weidunewtab.com/myapp/weather/data/index.php?cityID="+cityId;
    url.setUrl(URLSTR);
    reply = manager.get(QNetworkRequest(url));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    codeContent = reply->readAll();
    qDebug() << URLSTR;
    qDebug() << codeContent;
    parse_doucment = QJsonDocument::fromJson(codeContent.toLatin1(), &json_error);
    if(json_error.error == QJsonParseError::NoError)
    {
        if(parse_doucment.isObject())
        {
            QJsonObject obj = parse_doucment.object();
            if(obj.contains("weatherinfo")){
                QJsonObject::iterator it;
                it = obj.find("weatherinfo");
                QJsonObject weatherinfoObj = it.value().toObject();
                swf = weatherinfoObj.value("weather1").toString();
                //label->setText(swn+"\n天气："+swf);
                URLSTR="http://m.weather.com.cn/weather_img/"+ QString::number(weatherinfoObj.value("img1").toInt()) + ".gif";
                qDebug() << URLSTR;
                url.setUrl(URLSTR);
                reply = manager.get(QNetworkRequest(url));
                QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
                loop.exec();
                QPixmap pixmap;
                pixmap.loadFromData(reply->readAll());
                systray->setIcon(QIcon(pixmap));                

                QDateTime date = QDateTime::fromString(weatherinfoObj.value("date_y").toString(), "yyyy年M月d");
                //qDebug() << date;
                for(int i=1;i<8;i++){
                    labelDate[i-1]->setText(date.addDays(i-1).toString("M-d")+"\n"+date.addDays(i-1).toString("dddd"));
                    labelDate[i-1]->setAlignment(Qt::AlignCenter);
                    URLSTR="http://m.weather.com.cn/weather_img/"+ QString::number(weatherinfoObj.value("img"+QString::number(2*i-1)).toInt()) + ".gif";
                    url.setUrl(URLSTR);
                    reply = manager.get(QNetworkRequest(url));
                    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
                    loop.exec();
                    QPixmap pixmap;
                    pixmap.loadFromData(reply->readAll());
                    labelWImg[i-1]->setPixmap(pixmap);
                    labelWImg[i-1]->setAlignment(Qt::AlignCenter);
                    labelWeather[i-1]->setText(weatherinfoObj.value("weather"+QString::number(i)).toString()+"\n"+weatherinfoObj.value("temp"+QString::number(i)).toString()+"\n"+weatherinfoObj.value("wind"+QString::number(i)).toString());
                    labelWeather[i-1]->setAlignment(Qt::AlignCenter);
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    //关闭最后一个窗口不退出程序
    QApplication::setQuitOnLastWindowClosed(false);
    window=new MainWindow;
    window->setWindowTitle("中国天气预报");
    window->setFixedSize(500,200);

    //水平垂直居中
    QDesktopWidget* desktop = QApplication::desktop();
    window->move((desktop->width() - window->width())/2, (desktop->height() - window->height())/2);    
    // 报错：Attempting to set QLayout "" on MainWindow "", which already has a layout
    QWidget *widget=new QWidget;
    window->setCentralWidget(widget);
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

    layout = new QGridLayout;
    labelCity = new QLabel("城市");
    labelCity->setAlignment(Qt::AlignCenter);
    layout->addWidget(labelCity,0,0);
    labelTemp = new QLabel("温度\n?");
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
    aboutAction=new QAction("关于",traymenu);
    icon = style->standardIcon(QStyle::SP_MessageBoxInformation);
    aboutAction->setIcon(icon);
    quitAction=new QAction("退出",traymenu);
    icon = style->standardIcon(QStyle::SP_DialogCloseButton);
    quitAction->setIcon(icon);
    traymenu->addAction(forecastAction);
    traymenu->addAction(refreshAction);
    traymenu->addAction(aboutAction);
    traymenu->addAction(quitAction);
    systray->setContextMenu(traymenu);
    systray->show();
    QObject::connect(systray, &QSystemTrayIcon::activated, &iconIsActived);
    QObject::connect(forecastAction, &QAction::triggered, &windowForecast);
    QObject::connect(refreshAction, &QAction::triggered, &getWeather);
    QObject::connect(aboutAction, &QAction::triggered, &windowAbout);
    QObject::connect(quitAction, SIGNAL(triggered()), &app, SLOT(quit()));

    QTimer *timer=new QTimer();
    timer->setInterval(1800000);
    //timer->setInterval(60000);
    timer->start();
    QObject::connect(timer, &QTimer::timeout, &app, &getWeather);
    getWeather();

    return app.exec();
}
