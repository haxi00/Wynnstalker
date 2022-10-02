#pragma once

#include "ui_mainwindow.h"
#include "constants.h"
#include "world.h"
#include "player.h"

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
#include <QLineEdit>
#include <QFile>

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
    void manager_finished(QNetworkReply*);
    void api_call();
    
    void menuSearch_clicked();
    void menuShow_clicked();
    void menuInfo_clicked();
    void menuQuit_clicked();

    void searchBack_clicked();
    void searchThrower_clicked();
    void searchHunter_clicked();
    void searchText_entered();
    void searchPopup_finished();

    void showBack_clicked();
    void showNumber_clicked();
    void showPlayer_clicked();
    void showUptime_clicked();

protected:
    void keyPressEvent(QKeyEvent*) override;

private:
    void setupMenu();
    void setupSearch();
    void setupShow();
    void setupInfo();

    void searchAddPlayer(QString);
    void searchGetPlayers();
    void searchPaintPlayers();
    void searchPopupPaint();

    page currentWidget;

    Ui::mainwindowClass ui;

    QStackedWidget* stackedWidget;
    QWidget* menuWidget;
    QWidget* searchWidget;
    QWidget* showWidget;
    QWidget* infoWidget;

    QVector<world> worlds;
    QNetworkAccessManager* manager;
    QNetworkRequest request;
    QTimer* timerAPI;

    QFile apiFile;

    QVBoxLayout* menuLayout;

    QPushButton* menuSearchButton;
    QPushButton* menuShowButton;
    QPushButton* menuInfoButton;
    QPushButton* menuQuitButton;

    QLabel* menuPlayersOnline;

    QGridLayout* searchLayout;
    QVBoxLayout* searchThrowerLayout;
    QVBoxLayout* searchHunterLayout;

    QPushButton* searchThrowerButton;
    QPushButton* searchHunterButton;
    QLabel* searchLabel;
    QLineEdit* searchTextBox;
    QPushButton* searchBackButton;
    QLabel* searchPopup;

    QFile searchThrowerFile;
    QFile searchHunterFile;
    bool searchThrowOrHunt;
    QVector<player> searchPlayersThrower;
    QVector<player> searchPlayersHunter;

    QTimer* searchPopupTimer;

    QGridLayout* showLayout;
    QGridLayout* showWorldLayout;
    QVBoxLayout* showSortLayout;

    QPushButton* showBackButton;
    QPushButton* showSortNumber;
    QPushButton* showSortPlayer;
    QPushButton* showSortUptime;

    QLabel* showWorldCounter;
    QLabel* showSortby;

    WorldSort showWorldSort;
};
