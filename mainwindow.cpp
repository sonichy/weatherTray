#include "mainwindow.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QDateTime>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>
#include <QScrollArea>
#include <QDir>
#include <QDesktopServices>
#include <QTextBrowser>
#include <QPushButton>
#include <QGridLayout>
#include <QLineEdit>

static const QMap<QString, QString> weatherMap {
    {"NA", "55"},
    {"晴", "0"},
    {"多云", "1"},
    {"阴", "2"},
    {"阵雨", "3"},
    {"雷阵雨", "4"},
    {"小雨", "7"},
    {"中雨", "8"},
    {"大雨", "9"},
    {"中到大雨", "9"},
    {"雪", "13"},
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    ,settings(QCoreApplication::organizationName(), QCoreApplication::applicationName())
{
    setStyleSheet("QLabel { color:white; }"
                  "QScrollArea { border:none; }"
                  "QScrollBar:horizontal { background:transparent; }");
    setWindowTitle("中国天气预报");
    setFixedSize(500,303);
    move((QApplication::desktop()->width() - QApplication::desktop()->width())/2, (QApplication::desktop()->height() - QApplication::desktop()->height())/2);
    //报错：Attempting to set QLayout "" on MainWindow "", which already has a layout
    QWidget *widget = new QWidget;
    //不在任务栏显示，无最小化
    setWindowFlags(Qt::Dialog);
    //隐藏标题栏
    //setWindowFlags(Qt::FramelessWindowHint);
    //窗体透明
    //setWindowOpacity(0.8);
    //窗体背景完全透明
    setAttribute(Qt::WA_TranslucentBackground, true);
    widget->setAttribute(Qt::WA_TranslucentBackground, true);

    QVBoxLayout *vbox = new QVBoxLayout;
    QHBoxLayout *hbox = new QHBoxLayout;
    labelCity = new QLabel("城市");
    labelCity->setAlignment(Qt::AlignCenter);
    hbox->addWidget(labelCity);
    labelTemp = new QLabel("温度");
    labelTemp->setStyleSheet("font-size:40px;");
    labelTemp->setAlignment(Qt::AlignCenter);
    hbox->addWidget(labelTemp);
    labelSD = new QLabel("湿度");
    labelSD->setAlignment(Qt::AlignCenter);
    hbox->addWidget(labelSD);
    //labelWind = new QLabel("风向?\n风力?");
    //labelWind->setAlignment(Qt::AlignCenter);
    //hbox->addWidget(labelWind);
    labelPM = new QLabel("PM2.5");
    labelPM->setAlignment(Qt::AlignCenter);
    hbox->addWidget(labelPM);
    labelAQI = new QLabel("空气质量");
    labelAQI->setAlignment(Qt::AlignCenter);
    hbox->addWidget(labelAQI);
    labelUT = new QLabel("更新");
    labelUT->setAlignment(Qt::AlignCenter);
    hbox->addWidget(labelUT);
    vbox->addLayout(hbox);

    QWidget *widget1 = new QWidget;
    widget1->setFixedSize(1200,180);
    widget1->setAttribute(Qt::WA_TranslucentBackground, true);
    QGridLayout *gridLayout = new QGridLayout;
    for(int i=0; i<15; i++){
        labelDate[i] = new QLabel("1月1日\n星期一");
        labelDate[i]->setAlignment(Qt::AlignCenter);
        gridLayout->addWidget(labelDate[i],1,i);
        labelWImg[i] = new QLabel("");
        labelWImg[i]->setPixmap(QPixmap(QApplication::applicationDirPath() + "/images/55.png").scaled(50,50));
        labelWImg[i]->setAlignment(Qt::AlignCenter);
        gridLayout->addWidget(labelWImg[i],2,i);
        labelWeather[i] = new QLabel("晴\n15°C ~ 20°C\n北风1级");
        labelWeather[i]->setAlignment(Qt::AlignCenter);
        gridLayout->addWidget(labelWeather[i],3,i);
    }
    widget1->setLayout(gridLayout);
    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidget(widget1);
    vbox->addWidget(scrollArea);

    labelComment = new QLabel;
    vbox->addWidget(labelComment);
    widget->setLayout(vbox);
    setCentralWidget(widget);

    //托盘菜单
    systray = new QSystemTrayIcon(this);
    systray->setToolTip("托盘天气");
    systray->setIcon(QIcon(":/images/55.png"));
    systray->setVisible(true);
    QMenu *traymenu = new QMenu(this);
    QAction *action_forecast = new QAction("预报", traymenu);
    action_forecast->setIcon(QIcon::fromTheme("audio-volume-high"));
    QAction *action_refresh = new QAction("刷新", traymenu);
    action_refresh->setIcon(QIcon::fromTheme("view-refresh"));
    QAction *action_set = new QAction("设置", traymenu);
    action_set->setIcon(QIcon::fromTheme("set"));
    QAction *action_log = new QAction("日志", traymenu);
    action_log->setIcon(QIcon::fromTheme("document-new"));
    QAction *action_about = new QAction("关于", traymenu);
    action_about->setIcon(QIcon::fromTheme("help-about"));
    QAction *action_changelog = new QAction("更新日志", traymenu);
    action_changelog->setIcon(QIcon::fromTheme("document-new"));
    QAction *action_quit = new QAction("退出", traymenu);
    action_quit->setIcon(QIcon::fromTheme("application-exit"));
    traymenu->addAction(action_forecast);
    traymenu->addAction(action_refresh);
    traymenu->addAction(action_set);
    traymenu->addAction(action_log);
    traymenu->addAction(action_about);
    traymenu->addAction(action_changelog);
    traymenu->addAction(action_quit);
    systray->setContextMenu(traymenu);
    systray->show();
    connect(systray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));
    connect(action_forecast, SIGNAL(triggered(bool)), this, SLOT(showForecast()));
    connect(action_refresh, SIGNAL(triggered(bool)), this, SLOT(getWeather()));
    connect(action_set, SIGNAL(triggered(bool)), this, SLOT(set()));
    connect(action_log, &QAction::triggered, [=](){
          QDesktopServices::openUrl(QUrl(path));
    });
    connect(action_about, SIGNAL(triggered(bool)), this, SLOT(about()));
    connect(action_changelog, SIGNAL(triggered(bool)), this, SLOT(changelog()));
    connect(action_quit, SIGNAL(triggered()), qApp, SLOT(quit()));

    QTimer *timer = new QTimer();
    timer->setInterval(1800000);
    timer->start();
    connect(timer, SIGNAL(timeout()), this, SLOT(getWeather()));
    getWeather();
}

void MainWindow::closeEvent(QCloseEvent *event)//此函数在QWidget关闭时执行
{
    hide();
    //不退出App
    event->ignore();
}

void MainWindow::getWeather()
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString log = currentDateTime.toString("yyyy/MM/dd HH:mm:ss") + "\n";

    QUrl url;
    QString surl;
    QNetworkAccessManager manager;
    QNetworkReply *reply;
    QEventLoop loop;
    QByteArray BA;
    QJsonDocument JD;
    QJsonParseError JPE;

    city = settings.value("City", "").toString();
    if (city == "") {
        surl = "http://ip-api.com/json/?lang=zh-CN";
        url.setUrl(surl);
        reply = manager.get(QNetworkRequest(url));
        //请求结束并下载完成后，退出子事件循环
        QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        //开启子事件循环
        loop.exec();
        BA = reply->readAll();
        qDebug() << surl ;
        qDebug() << BA;
        log += surl + "\n";
        log += BA + "\n";
        JD = QJsonDocument::fromJson(BA, &JPE);
        if(JPE.error == QJsonParseError::NoError) {
            if(JD.isObject()) {
                QJsonObject obj = JD.object();
                if(obj.contains("city")) {
                    QJsonValue JV_city = obj.take("city");
                    if(JV_city.isString()) {
                        city = JV_city.toString();
                    }
                }
            }
        }
    }

    //城市名转ID
    /*
    surl = "http://hao.weidunewtab.com/tianqi/city.php?city=" + city;
    url.setUrl(surl);
    reply = manager.get(QNetworkRequest(url));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    cityID = reply->readAll();
    qDebug() << surl;
    qDebug() << cityID;
    log += surl + "\n";
    log += cityID + "\n";
    if (cityID == "") {
        labelComment->setText("错误：城市名返回城市ID为空");
        return;
    } else if(cityID == "ERROR") {
        labelComment->setText(city + " ：城市名称错误");
        return;
    } else {
        bool ok;
        int dec = cityID.toInt(&ok, 10);
        Q_UNUSED(dec);
        if(!ok){
            labelComment->setText(reply->readAll());
        }
    }
    */
    //读取本地文件代替网络API，更快更可靠。
    QFile file(":/cityID.txt");
    bool ok = file.open(QIODevice::ReadOnly);
    if (ok) {
        QTextStream TS(&file);
        QString s = TS.readAll();
        file.close();
        QStringList SL = s.split("\n");
        for(int i=0; i<SL.length(); i++){
            QString line = SL.at(i);
            if (line.contains(city)) {
                cityID = line.left(line.indexOf("="));
                break;
            }
        }
    }

    //surl = "http://hao.weidunewtab.com/myapp/weather/data/index.php?cityID=" + cityID; //失效
    surl = "http://t.weather.sojson.com/api/weather/city/"+ cityID;
    url.setUrl(surl);
    reply = manager.get(QNetworkRequest(url));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    BA = reply->readAll();
    qDebug() << surl;
    //qDebug() << BA;
    log += surl + "\n";
    log += BA + "\n";
    JD = QJsonDocument::fromJson(BA, &JPE);
    //qDebug() << "QJsonParseError:" << JPE.errorString();
    if (JPE.error == QJsonParseError::NoError) {
        if (JD.isObject()) {
            QJsonObject obj = JD.object();
            city = obj.value("cityInfo").toObject().value("city").toString().replace("市","");
            labelCity->setText(city);
            QString SUT = obj.value("cityInfo").toObject().value("updateTime").toString();
            labelUT->setText("更新\n" + SUT);
            //if (obj.contains("data")) {
            QJsonObject JO_data = JD.object().value("data").toObject();
            QString wendu = JO_data.value("wendu").toString() + "°C";
            labelTemp->setText(wendu);
            QString shidu = JO_data.value("shidu").toString();
            labelSD->setText("湿度\n" + shidu);
            //labelWind->setText(JO_data.value("wendu").toString() + "\n" + JO_data.value("WS").toString());
            QString pm25 = QString::number(JO_data.value("pm25").toInt());
            labelPM->setText("PM2.5\n" + pm25);
            QString quality = JO_data.value("quality").toString();
            QString ganmao = JO_data.value("ganmao").toString();
            labelAQI->setText("空气质量 " + quality + "\n" + ganmao);

            //if(JO_data.contains("forecast")){
            QJsonArray JA_forecast = JO_data.value("forecast").toArray();
            for (int i=0; i<15; i++) {
                labelDate[i]->setText(JA_forecast[i].toObject().value("date").toString());
                labelDate[i]->setAlignment(Qt::AlignCenter);
                QString wtype = JA_forecast[i].toObject().value("type").toString();
                qDebug() << wtype;
                QString icon_path = ":/images/" + weatherMap[wtype] + ".png";
                if(i == 0){
                    sw0 = JA_forecast[i].toObject().value("type").toString();
                    icon_path0 = icon_path;
                }
                //qDebug() << icon_path;
                QPixmap pixmap(icon_path);
                labelWImg[i]->setToolTip(wtype);
                labelWImg[i]->setPixmap(pixmap.scaled(50,50));
                labelWImg[i]->setAlignment(Qt::AlignCenter);
                labelWeather[i]->setText(wtype + "\n" + JA_forecast[i].toObject().value("high").toString() + "\n" + JA_forecast[i].toObject().value("low").toString() + "\n" + JA_forecast[i].toObject().value("fx").toString() + JA_forecast[i].toObject().value("fl").toString());
                labelWeather[i]->setAlignment(Qt::AlignCenter);
            }
            //}
            //}
            swn = city + "\n" + sw0 + "\n" + wendu + "\n湿度：" + shidu + "\nPM2.5：" + pm25 + "\n空气质量：" + quality +"\n" + ganmao + "\n更新：" + SUT;
            //qDebug() << swn;
            systray->setToolTip(swn);
            systray->setIcon(QIcon(icon_path0));
        }
    }

    // 写log
    path = QApplication::applicationDirPath() + "/log.txt";
    qDebug() << path;
    file.setFileName(path);
    if (file.open(QFile::WriteOnly)) {
        file.write(log.toUtf8());
        file.close();
    }

}

void MainWindow::trayActivated(QSystemTrayIcon::ActivationReason reason)
{
    qDebug() << reason;
    switch(reason) {
    case QSystemTrayIcon::Trigger:
        //systray->showMessage("实时天气", swn, QSystemTrayIcon::MessageIcon::Information, 9000);//图标被修改
        break;
    case QSystemTrayIcon::DoubleClick://无效
        //showForecast();
        break;
    case QSystemTrayIcon::MiddleClick:
        showForecast();
        break;
    default:
        break;
    }
}

void MainWindow::showForecast()
{
    move((QApplication::desktop()->width() - width())/2, (QApplication::desktop()->height() - height())/2);
    show();
    raise();
    activateWindow();
}

void MainWindow::about()
{
    QMessageBox MB(QMessageBox::NoIcon, "关于", "中国天气预报 2.6\n一款基于Qt的天气预报程序。\n作者：海天鹰\nE-mail: sonichy@163.com\n主页：https://github.com/sonichy\n天气API：https://www.sojson.com/blog/305.html\n\n参考：\n获取网页源码写入文件: http://blog.csdn.net/small_qch/article/details/7200271\nJOSN解析: http://blog.sina.com.cn/s/blog_a6fb6cc90101gnxm.html");
    MB.setIconPixmap(QPixmap(":/icon.png"));
    MB.exec();
}

void MainWindow::changelog()
{
    QString s = "2.6 (2020-04-18)\n天气预报增加到15天。\n滚动布局。\n增加设置城市。\n图标加入资源，方便单文件运行。\n城市名转ID，读取本地文件代替网络API，更快更可靠。\n\n2.5 (2018-10-15)\n1.更换失效的天气预报API，优化结构，解决窗口顶置问题。\n\n2.4 (2018-05-30)\n1.增加写log文件，不需要调试也可以查看API的问题了。\n\n2.3 (2018-04-13)\n1.修复：日期数据不含月转换为日期引起的日期错误。\n\n2.2 (2017-01-23)\n1.使用本地图标代替边缘有白色的网络图标，可用于暗色背景了！\n\n2.1 (2016-12-09)\n1.窗体始终居中。\n\n2.0 (2016-11-21)\n1.使用QScript库代替QJsonDocument解析JSON，开发出兼容Qt4的版本。\n2.单击图标弹出实时天气消息，弥补某些系统不支持鼠标悬浮信息的不足。\n3.由于QScriptValueIterator.value().property解析不了某个JSON，使用QScriptValue.property.property代替。\n4.托盘右键增加一个刷新菜单。\n\n1.0 (2016-11-17)\n1.动态修改天气栏托盘图标，鼠标悬浮显示实时天气，点击菜单弹出窗口显示7天天气预报。\n2.每30分钟自动刷新一次。\n3.窗体透明。";
    QDialog *dialog = new QDialog;
    dialog->setWindowTitle("更新历史");
    dialog->setFixedSize(400, 300);
    QVBoxLayout *vbox = new QVBoxLayout;
    QTextBrowser *textBrowser = new QTextBrowser;
    textBrowser->setText(s);
    textBrowser->zoomIn();
    vbox->addWidget(textBrowser);
    QHBoxLayout *hbox = new QHBoxLayout;
    QPushButton *pushButton_confirm = new QPushButton("确定");
    hbox->addStretch();
    hbox->addWidget(pushButton_confirm);
    hbox->addStretch();
    vbox->addLayout(hbox);
    dialog->setLayout(vbox);
    dialog->show();
    connect(pushButton_confirm, SIGNAL(clicked()), dialog, SLOT(accept()));
    if(dialog->exec() == QDialog::Accepted){
        dialog->close();
    }
}

void MainWindow::set()
{
    QDialog *dialog = new QDialog;
    dialog->setWindowTitle("设置");
    dialog->setFixedSize(200, 100);
    QVBoxLayout *vbox = new QVBoxLayout;
    QHBoxLayout *hbox = new QHBoxLayout;
    QLabel *label = new QLabel("城市");
    hbox->addWidget(label);
    QLineEdit *lineEdit = new QLineEdit(city);
    hbox->addWidget(lineEdit);
    vbox->addLayout(hbox);

    hbox = new QHBoxLayout;
    QPushButton *pushButton_confirm = new QPushButton("确定");
    QPushButton *pushButton_cancel = new QPushButton("取消");
    hbox->addStretch();
    hbox->addWidget(pushButton_confirm);
    hbox->addStretch();
    vbox->addLayout(hbox);
    dialog->setLayout(vbox);
    dialog->show();
    connect(pushButton_confirm, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(pushButton_cancel, SIGNAL(clicked()), dialog, SLOT(reject()));
    int dc = dialog->exec();
    if (dc == QDialog::Accepted) {
        settings.setValue("City", lineEdit->text());
        dialog->close();
        getWeather();
    } else if (dc == QDialog::Rejected) {
        dialog->close();
    }
}