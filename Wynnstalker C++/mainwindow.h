#pragma once

#include "ui_mainwindow.h"
#include "apis.h"
#include "world.h"

#include <QtWidgets>
#include <QStackedwidget>
#include <QNetworkAccessManager.h>
#include <QNetworkReply.h>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QRegion>
#include <QKeyEvent>

enum page
{
    MENU,
    SEARCH,
    SHOW,
    INFO
};

class mainwindow : public QMainWindow
{
    Q_OBJECT

public:
    mainwindow(QWidget *parent = nullptr);
    ~mainwindow();

private slots:
    void menuSearch_clicked();
    void menuShow_clicked();
    void menuInfo_clicked();
    void menuQuit_clicked();
    void managerFinished(QNetworkReply*);
    void timerAPI();

    void showBack_clicked();
    inline void showNumber_clicked() { showWorldSort = NUMBER; }
    inline void showPlayer_clicked() { showWorldSort = PLAYERS; }
    inline void showUptime_clicked() { showWorldSort = UPTIME; }

protected:
    void keyPressEvent(QKeyEvent*) override;

private:
    void setupMenu();
    void setupSearch();
    void setupShow();
    void setupInfo();

    page currentWidget;

    Ui::mainwindowClass ui;

    QStackedWidget* stackedWidget;
    QWidget* menuWidget;
    QWidget* searchWidget;
    QWidget* showWidget;
    QWidget* infoWidget;

    QVBoxLayout* menuLayout;

    QPushButton* menuSearchButton;
    QPushButton* menuShowButton;
    QPushButton* menuInfoButton;
    QPushButton* menuQuitButton;

    QLabel* menuPlayersOnline;

    QNetworkAccessManager* manager;
    QNetworkRequest request;

    QTimer* timer;

    QGridLayout* showLayout;
    QGridLayout* showWorldLayout;
    QVBoxLayout* showSortLayout;

    QPushButton* showBackButton;
    QPushButton* showSortNumber;
    QPushButton* showSortPlayer;
    QPushButton* showSortUptime;

    QLabel* showWorldCounter;
    QLabel* showSortby;

    QVector<world> worlds;
    WorldSort showWorldSort;
};
