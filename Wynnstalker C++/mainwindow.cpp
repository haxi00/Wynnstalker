#include "mainwindow.h"

mainwindow::mainwindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    stackedWidget = new QStackedWidget;
    menuWidget = new QWidget();
    searchWidget = new QWidget();
    showWidget = new QWidget();
    infoWidget = new QWidget();

    setupMenu();
    setupSearch();
    setupShow();
    //setupInfo();

    stackedWidget->setBaseSize(qApp->screens()[0]->size());
    stackedWidget->addWidget(menuWidget);
    stackedWidget->addWidget(searchWidget);
    stackedWidget->addWidget(showWidget);
    stackedWidget->addWidget(infoWidget);
    stackedWidget->setCurrentWidget(menuWidget);

    setCentralWidget(stackedWidget);
    stackedWidget->setCurrentWidget(menuWidget);

    currentWidget = MENU;
    timerAPI();
    timer->start(3000);
}

mainwindow::~mainwindow()
{
    delete manager;
    delete timer;
}

void mainwindow::menuSearch_clicked()
{
    stackedWidget->setCurrentWidget(searchWidget);
    currentWidget = SEARCH;
}

void mainwindow::menuShow_clicked()
{
    stackedWidget->setCurrentWidget(showWidget);
    currentWidget = SHOW;
    timerAPI();
}

void mainwindow::menuInfo_clicked()
{
    //stackedWidget->setCurrentWidget(infoWidget);
    //currentWidget = INFO;
}

void mainwindow::menuQuit_clicked()
{
    QApplication::exit();
}

void mainwindow::managerFinished(QNetworkReply* reply)
{
    if (currentWidget == MENU)
    {
        if (reply->error())
        {
            qDebug() << reply->errorString();
            return;
        }
        else
        {
            QString answer = reply->readAll();
            QJsonDocument jsonResponse = QJsonDocument::fromJson(answer.toUtf8());
            QJsonObject jsonObject = jsonResponse.object();
            QJsonValue jsonValue = jsonObject["players_online"].toInt();

            menuPlayersOnline->setText("Online: " + QString::number(jsonValue.toInt()));

            qDebug() << menuPlayersOnline->text();

            /*QList<QByteArray> headerList = reply->rawHeaderList();
            foreach(QByteArray head, headerList) 
            {
                qDebug() << head << ":" << reply->rawHeader(head);
            }*/
        }
    }
    else if (currentWidget == SHOW)
    {
        if (reply->error())
        {
            qDebug() << reply->errorString();
            return;
        }
        else 
        {
            QString answer = reply->readAll();
            QJsonDocument jsonResponse = QJsonDocument::fromJson(answer.toUtf8());
            QJsonObject jsonObject = jsonResponse.object();
            QJsonObject jsonWorldObjects = jsonObject["servers"].toObject();
            QStringList worldList = jsonWorldObjects.keys();

            qsizetype actualWorlds = 0;
            for (qsizetype i = 0; i < worldList.size(); i++)
                if (worldList[i] != "TEST" && worldList[i] != "YT")
                    actualWorlds++;
            worlds.resize(actualWorlds);
            showWorldCounter->setText("    " + QString::number(actualWorlds) + "\nWorlds");

            for (qsizetype i = 0; i < worldList.size(); i++)
                if (worldList[i] != "TEST" && worldList[i] != "YT")
                    worlds[i].setName(worldList[i]);

            QJsonObject tempObj;
            QJsonArray tempArray;
            for (qsizetype i = 0; i < worlds.size(); i++)
            {
                tempObj = jsonWorldObjects[worlds[i].getName()].toObject();
                worlds[i].setUptime(tempObj["firstSeen"].toInteger());
                worlds[i].uptimeLabel->setText(worlds[i].convertUptime());

                QStringList tempNameList;
                tempArray = tempObj["players"].toArray();
                for (qsizetype j = 0; j < tempArray.size(); j++)
                    tempNameList.append(tempArray[j].toString());
                worlds[i].setPlayers(tempNameList);
                worlds[i].button->setText(worlds[i].getName() + "\n" + QString::number(worlds[i].getPlayers().size()) + "/" + QString::number(MAXWORLDPLAYERS-5));
            }

            if (showWorldSort == NUMBER)
                world::sortWorlds(&worlds, world::cmpNumber);
            else if (showWorldSort == PLAYERS)
                world::sortWorlds(&worlds, world::cmpPlayers);
            else if (showWorldSort == UPTIME)
                world::sortWorlds(&worlds, world::cmpUptime);

            if (currentWidget == SHOW && worlds.size() > 0)
            {
                int rows, columns, rowLength, columnLength;

                if (worlds.size() % (worlds.size() / 10 + 3) != 0)
                    rows = worlds.size() / (worlds.size() / 10 + 3) + 1;
                else rows = worlds.size() / (worlds.size() / 10 + 3);
                if (worlds.size() % rows != 0)
                    columns = worlds.size() / rows + 1;
                else columns = worlds.size() / rows;

                for (int i = 0; i < rows; i++)
                {
                    for (int j = 0; j < columns && i * columns + j < worlds.size(); j++)
                    {
                        showWorldLayout->removeItem(worlds[i * columns + j].layout);
                        worlds[i * columns + j].layout->setObjectName("layout_" + worlds[i * columns + j].getName());
                        showWorldLayout->addLayout(worlds[i * columns + j].layout, i, j, Qt::AlignBottom);

                        worlds[i * columns + j].uptimeLabel->setStyleSheet("font-size: 17px; font-weight:600; color: white;");

                        if (worlds[i * columns + j].getPlayers().size() < 35)
                            worlds[i * columns + j].button->setStyleSheet("font-size: 20px; font-weight:800; color: white; background-color: rgb(85, 135, 20);");
                        else if (worlds[i * columns + j].getPlayers().size() >= 35 && worlds[i * columns + j].getPlayers().size() < 40)
                            worlds[i * columns + j].button->setStyleSheet("font-size: 20px; font-weight:800; color: white; background-color: rgb(210, 148, 6);");
                        else if (worlds[i * columns + j].getPlayers().size() >= 40 && worlds[i * columns + j].getPlayers().size() <= MAXWORLDPLAYERS)
                            worlds[i * columns + j].button->setStyleSheet("font-size: 20px; font-weight:800; color: white; background-color: rgb(150, 43, 28);");
                        else 
                            worlds[i * columns + j].button->setStyleSheet("font-size: 20px; font-weight:800; color: white; background-color: rgb(179, 0, 179);");
                    }
                }
            }
        }
    }
    reply->deleteLater();
}

void mainwindow::timerAPI()
{
    if(currentWidget == MENU)
    {
        request.setUrl(QUrl(PLAYERSUM));
        manager->get(request);
    }
    else if (currentWidget == SHOW)
    {
        request.setUrl(QUrl(SERVERLIST));
        manager->get(request);
    }
}

void mainwindow::searchBack_clicked()
{
    stackedWidget->setCurrentWidget(menuWidget);
    currentWidget = MENU;
}

void mainwindow::showBack_clicked()
{
    stackedWidget->setCurrentWidget(menuWidget);
    currentWidget = MENU;
}

void mainwindow::keyPressEvent(QKeyEvent *event)
{
    if (currentWidget == MENU)
    {
        if(event->key() == Qt::Key_Escape)
            QApplication::exit();
    }
    else if (currentWidget == SHOW || currentWidget == SEARCH)
    {
        if (event->key() == Qt::Key_Escape)
        {
            stackedWidget->setCurrentWidget(menuWidget);
            currentWidget = MENU;
        }
    }
}

void mainwindow::setupMenu()
{
    //Initialize Variables
    menuLayout = new QVBoxLayout();
    menuSearchButton = new QPushButton();
    menuShowButton = new QPushButton();
    menuInfoButton = new QPushButton();
    menuQuitButton = new QPushButton();
    menuPlayersOnline = new QLabel();
    manager = new QNetworkAccessManager();
    timer = new QTimer();

    //Set Text
    menuPlayersOnline->setText("Online: ");
    menuSearchButton->setText("Search Players");
    menuShowButton->setText("Show Worlds");
    menuInfoButton->setText("Info");
    menuQuitButton->setText("Quit");

    //Set Cursor when hovering over Buttons
    menuSearchButton->setCursor(Qt::PointingHandCursor);
    menuShowButton->setCursor(Qt::PointingHandCursor);
    menuInfoButton->setCursor(Qt::PointingHandCursor);
    menuQuitButton->setCursor(Qt::PointingHandCursor);

    //Set Color and Font Style
    menuPlayersOnline->setStyleSheet("font-size: 20px; font-weight:400; color: white;");
    menuSearchButton->setStyleSheet("font-size: 120px; font-weight:800; color: white; background-color: black");
    menuShowButton->setStyleSheet("font-size: 120px; font-weight:800; color: white; background-color: black");
    menuInfoButton->setStyleSheet("font-size: 120px; font-weight:800; color: white; background-color: black");
    menuQuitButton->setStyleSheet("font-size: 120px; font-weight:800; color: red; background-color: black");

    //Set Menu Screen black
    menuWidget->setStyleSheet("background-color: black");

    //Add Buttons to a vertical Layout
    menuLayout->addWidget(menuPlayersOnline, 0, Qt::AlignLeft | Qt::AlignTop);
    menuLayout->addWidget(menuSearchButton, 1, Qt::AlignHCenter | Qt::AlignTop);
    menuLayout->addWidget(menuShowButton, 1, Qt::AlignHCenter | Qt::AlignTop);
    menuLayout->addWidget(menuInfoButton, 1, Qt::AlignHCenter | Qt::AlignTop);
    menuLayout->addWidget(menuQuitButton, 1, Qt::AlignHCenter | Qt::AlignTop);

    menuWidget->setLayout(menuLayout);

    connect(menuSearchButton, SIGNAL(clicked()), this, SLOT(menuSearch_clicked()));
    connect(menuShowButton, SIGNAL(clicked()), this, SLOT(menuShow_clicked()));
    connect(menuInfoButton, SIGNAL(clicked()), this, SLOT(menuInfo_clicked()));
    connect(menuQuitButton, SIGNAL(clicked()), this, SLOT(menuQuit_clicked()));

    connect(manager, &QNetworkAccessManager::finished, this, &mainwindow::managerFinished);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerAPI()));
}

void mainwindow::setupSearch()
{
    searchLayout = new QGridLayout();
    searchThrowerLayout = new QVBoxLayout();
    searchHunterLayout = new QVBoxLayout();

    searchThrowerLabel = new QLabel();
    searchHunterLabel = new QLabel();
    searchLabel = new QLabel();
    searchTextBox = new QLineEdit();
    searchBackButton = new QPushButton();

    searchThrowerLabel->setText("Throwers");
    searchHunterLabel->setText("Hunters");
    searchLabel->setText("Search:");
    searchBackButton->setText("Back");

    searchLabel->setStyleSheet("font-size: 30px; font-weight:400; color: white;");
    searchBackButton->setStyleSheet("font-size: 50px; font-weight:800; color: white; border-style: outset; border-width: 5px; background-color: black");
    searchThrowerLabel->setStyleSheet("font-size: 30px; font-weight:400; color: green; background-color: black");
    searchHunterLabel->setStyleSheet("font-size: 30px; font-weight:400; color: red; background-color: black");
    searchTextBox->setStyleSheet("QLineEdit{ "
                                        "background-color: black;"
                                        "border: 2px solid white;"
                                        "padding: 0 8px;"
                                        "selection-background-color: grey;"                     
                                        "font-size: 30px;}"
                                        "QLineEdit:focus { "
                                        "background: rgba(100, 100, 100, 150);}");
    searchWidget->setStyleSheet("background-color: black");

    /*searchThrowerLayout->addWidget(searchThrowerLabel);
    searchHunterLayout->addWidget(searchHunterLabel);*/

    searchLayout->addWidget(searchLabel, 0, 0, Qt::AlignTop | Qt::AlignRight);
    searchLayout->addWidget(searchTextBox, 0, 1, 0, 2, Qt::AlignTop);
    searchLayout->addWidget(searchBackButton, 0, 3, Qt::AlignRight);
    searchLayout->addWidget(searchThrowerLabel, 1, 0, Qt::AlignTop);
    searchLayout->addWidget(searchHunterLabel, 1, 2, Qt::AlignTop);
    /*searchLayout->addLayout(searchThrowerLayout, 1, 0, -1, 1, Qt::AlignTop);
    searchLayout->addLayout(searchHunterLayout, 1, 2, -1, 3, Qt::AlignTop);*/
    
    searchWidget->setLayout(searchLayout);

    connect(searchBackButton, SIGNAL(clicked()), this, SLOT(searchBack_clicked()));
}

void mainwindow::setupShow()
{
    showLayout = new QGridLayout();
    showWorldLayout = new QGridLayout();
    showSortLayout = new QVBoxLayout();

    showBackButton = new QPushButton();
    showSortNumber = new QPushButton();
    showSortPlayer = new QPushButton();
    showSortUptime = new QPushButton();

    showWorldCounter = new QLabel();
    showSortby = new QLabel();

    showWorldSort = NUMBER;

    showBackButton->setText("Back");
    showSortNumber->setText("Number");
    showSortPlayer->setText("Player");
    showSortUptime->setText("Uptime");
    showSortby->setText("Sort by");

    //Set Cursor when hovering over Buttons
    showBackButton->setCursor(Qt::PointingHandCursor);
    showSortNumber->setCursor(Qt::PointingHandCursor);
    showSortPlayer->setCursor(Qt::PointingHandCursor);
    showSortUptime->setCursor(Qt::PointingHandCursor);

    //Set Color and Font Style
    showBackButton->setStyleSheet("font-size: 50px; font-weight:800; color: white; border-style: outset;border-width: 5px; background-color: black");
    showSortNumber->setStyleSheet("font-size: 30px; font-weight:800; color: white; border-style: outset;border-width: 5px; background-color: black");
    showSortPlayer->setStyleSheet("font-size: 30px; font-weight:800; color: white; border-style: outset;border-width: 5px; background-color: black");
    showSortUptime->setStyleSheet("font-size: 30px; font-weight:800; color: white; border-style: outset;border-width: 5px; background-color: black");
    showWorldCounter->setStyleSheet("font-size: 40px; font-weight:800; color: white; background-color: black");
    showSortby->setStyleSheet("font-size: 20px; font-weight:600; color: white; background-color: black");
    showWidget->setStyleSheet("background-color: black");

    showSortLayout->addWidget(showSortby, 0, Qt::AlignCenter | Qt::AlignBottom);
    showSortLayout->addWidget(showSortNumber);
    showSortLayout->addWidget(showSortPlayer);
    showSortLayout->addWidget(showSortUptime);

    showLayout->addLayout(showWorldLayout, 0, 0, 4, 6);
    showLayout->addWidget(showBackButton, 0, 6, 1, 1);
    showLayout->addWidget(showWorldCounter, 1, 6, 2, 1, Qt::AlignCenter);
    showLayout->addLayout(showSortLayout, 2, 6, -1, 1);

    showWidget->setLayout(showLayout);

    connect(showBackButton, SIGNAL(clicked()), this, SLOT(showBack_clicked()));
    connect(showSortNumber, SIGNAL(clicked()), this, SLOT(showNumber_clicked()));
    connect(showSortPlayer, SIGNAL(clicked()), this, SLOT(showPlayer_clicked()));
    connect(showSortUptime, SIGNAL(clicked()), this, SLOT(showUptime_clicked()));
}

void mainwindow::setupInfo()
{

}
