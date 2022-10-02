#include "mainwindow.h"

mainwindow::mainwindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    //Initialize widgets
    stackedWidget = new QStackedWidget;
    menuWidget = new QWidget();
    searchWidget = new QWidget();
    showWidget = new QWidget();
    infoWidget = new QWidget();

    //Setup widgets
    setupMenu();
    setupSearch();
    setupShow();
    //setupInfo();

    //Add widgets to a stacked widget
    stackedWidget->setBaseSize(qApp->screens()[0]->size());
    stackedWidget->addWidget(menuWidget);
    stackedWidget->addWidget(searchWidget);
    stackedWidget->addWidget(showWidget);
    stackedWidget->addWidget(infoWidget);
    stackedWidget->setCurrentWidget(menuWidget);
    setCentralWidget(stackedWidget);

    //Set menu as start widget
    stackedWidget->setCurrentWidget(menuWidget);
    currentWidget = MENU;

    //Call the API a first time and then every 3 seconds when the timer is over (timerAPI() working as Slot)
    api_call();
    timerAPI->start(3000);
}

mainwindow::~mainwindow()
{
    delete manager;
    delete timerAPI;
}

void mainwindow::manager_finished(QNetworkReply* reply)
{
    //Error check for the APIs answer
    if (reply->error())
    {
        qDebug() << reply->errorString();
        return;
    }

    //String containing the APIs answer
    QString answer = reply->readAll();

    //Write API Data in a file, truncate flag clears file before writing, text flag changes something with the carriage return which is probably not needed
    if (!apiFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        qDebug() << "ERROR: Writing into apiFile. error():  " << apiFile.error();
    else
    {
        QTextStream out(&apiFile);
        out << answer;
        apiFile.close();
    }

    QJsonDocument jsonResponse = QJsonDocument::fromJson(answer.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();

    //While in the menu the API only returns the count of players on the server
    if (reply->request().url() == QUrl(PLAYERSUM))
    {
        //Extract the count of players from the JSON string
        QJsonValue jsonValue = jsonObject["players_online"].toInt();

        menuPlayersOnline->setText("Online: " + QString::number(jsonValue.toInt()));

        qDebug() << menuPlayersOnline->text();

        //Possibility to check API reply header for rate-limit
        /*QList<QByteArray> headerList = reply->rawHeaderList();
        foreach(QByteArray head, headerList) 
        {
           qDebug() << head << ":" << reply->rawHeader(head);
        }*/
    }
    //Get all worlds and their players while in "Show Worlds" or "Search Players"
    else if (reply->request().url() == QUrl(SERVERLIST))
    {
        //Extract all world names and their objects(containing the players) from the JSON string
        QJsonObject jsonWorldObjects = jsonObject["servers"].toObject();
        QStringList worldList = jsonWorldObjects.keys();

        //Variable to work with the actual world count excluding the test- and youtube-world
        qsizetype actualWorlds = 0;
        for (qsizetype i = 0; i < worldList.size(); i++)
            if (worldList[i] != "TEST" && worldList[i] != "YT")
                actualWorlds++;
        worlds.resize(actualWorlds);
        showWorldCounter->setText("    " + QString::number(actualWorlds) + "\nWorlds");

        //Set every worlds name
        actualWorlds = 0;
        for (qsizetype i = 0; i < worldList.size(); i++)
            if (worldList[i] != "TEST" && worldList[i] != "YT")
                worlds[actualWorlds++].setName(worldList[i]);

        //Extract every worlds player and set the worlds uptime and playercount
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

        //What the worlds should be sorted by (worldname, playercout, worlds uptime)
        if (showWorldSort == NUMBER)
            world::sortWorlds(&worlds, world::cmpNumber);
        else if (showWorldSort == PLAYERS)
            world::sortWorlds(&worlds, world::cmpPlayers);
        else if (showWorldSort == UPTIME)
            world::sortWorlds(&worlds, world::cmpUptime);

        //Displaying the worlds
        if (currentWidget == SHOW && worlds.size() > 0)
        {
            //Weird calculations to have a decent mix between rows and columns for the world layout on screen
            int rows, columns, rowLength, columnLength;
            if (worlds.size() % (worlds.size() / 10 + 3) != 0)
                rows = worlds.size() / (worlds.size() / 10 + 3) + 1;
            else rows = worlds.size() / (worlds.size() / 10 + 3);
            if (worlds.size() % rows != 0)
                columns = worlds.size() / rows + 1;
            else columns = worlds.size() / rows;

            //Loop working from row to row
            for (int i = 0; i < rows; i++)
            {
                //Loop working from column to column
                for (int j = 0; j < columns && i * columns + j < worlds.size(); j++)
                {
                    //Removes world from layout from the last cycle (layouts dont reparent layouts in qt as far as i understood, so you have to delete them yourself)
                    showWorldLayout->removeItem(worlds[i * columns + j].layout);
                    //Add new world to the layout
                    showWorldLayout->addLayout(worlds[i * columns + j].layout, i, j, Qt::AlignBottom);
                    //Styles the uptime shown below a world
                    worlds[i * columns + j].uptimeLabel->setStyleSheet("font-size: 17px; font-weight:600; color: white;");

                    //Style and color the worlds respectively to their playercount
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
        if (currentWidget == SEARCH)
            searchPaintPlayers();
    }
    //When searching a player
    else
    {
        QString request = jsonObject["search"].toString();
        QJsonArray playerList = jsonObject["players"].toArray();
        bool found = false;
        if (!playerList.size() || (playerList.size() == 1 && QString::compare(request, playerList[0].toString(), Qt::CaseInsensitive)))
        {
            searchPopup->setText(request + " not found");
            searchPopupPaint();
        }
        else if (playerList.size() > 1)
        {
            foreach(QJsonValue playerName, playerList)
            {
                if (!QString::compare(request, playerName.toString(), Qt::CaseInsensitive))
                {
                    found = true;
                    player temp;
                    temp.name->setText(playerName.toString());
                    if (searchThrowOrHunt)
                        searchPlayersThrower.append(temp);
                    else searchPlayersHunter.append(temp);
                    searchAddPlayer(playerName.toString());
                    searchPaintPlayers();
                    break;
                }
            }
            if(!found)
            {
                searchPopup->setText(request + " not found");
                searchPopupPaint();
            }
        }
        else if (playerList.size() == 1)
        {
            player temp;
            temp.name->setText(playerList[0].toString());
            if (searchThrowOrHunt)
                searchPlayersThrower.append(temp);
            else searchPlayersHunter.append(temp);
            searchAddPlayer(playerList[0].toString());
            searchPaintPlayers();
        }
    }
    reply->deleteLater();
}

void mainwindow::api_call()
{
    //Function gets called every 3 seconds to get the needed data for the current active widget from the API
    if(currentWidget == MENU)
    {
        request.setUrl(QUrl(PLAYERSUM));
        manager->get(request);
    }
    else if (currentWidget == SEARCH || currentWidget == SHOW)
    {
        request.setUrl(QUrl(SERVERLIST));
        manager->get(request);
    }
}

void mainwindow::keyPressEvent(QKeyEvent *event)
{
    //Program can be closed by pressing the escape button in the menu
    if (currentWidget == MENU)
    {
        if(event->key() == Qt::Key_Escape)
            QApplication::exit();
    }
    //Switching back to the menu when pressing the escape button
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
    //Initialize variables
    menuLayout = new QVBoxLayout();
    menuSearchButton = new QPushButton();
    menuShowButton = new QPushButton();
    menuInfoButton = new QPushButton();
    menuQuitButton = new QPushButton();
    menuPlayersOnline = new QLabel();
    manager = new QNetworkAccessManager();
    timerAPI = new QTimer();
    apiFile.setFileName(FILE_API);

    //Set text
    menuPlayersOnline->setText("Online: ");
    menuSearchButton->setText("Search Players");
    menuShowButton->setText("Show Worlds");
    menuInfoButton->setText("Info");
    menuQuitButton->setText("Quit");

    //Set cursor when hovering over buttons
    menuSearchButton->setCursor(Qt::PointingHandCursor);
    menuShowButton->setCursor(Qt::PointingHandCursor);
    menuInfoButton->setCursor(Qt::PointingHandCursor);
    menuQuitButton->setCursor(Qt::PointingHandCursor);

    //Set color and font style
    menuPlayersOnline->setStyleSheet("font-size: 20px; font-weight:400; color: white;");
    menuSearchButton->setStyleSheet("font-size: 120px; font-weight:800; color: white; background-color: black");
    menuShowButton->setStyleSheet("font-size: 120px; font-weight:800; color: white; background-color: black");
    menuInfoButton->setStyleSheet("font-size: 120px; font-weight:800; color: white; background-color: black");
    menuQuitButton->setStyleSheet("font-size: 120px; font-weight:800; color: red; background-color: black");

    //Set menu screen black
    menuWidget->setStyleSheet("background-color: black");

    //Add buttons to a vertical layout
    menuLayout->addWidget(menuPlayersOnline, 0, Qt::AlignLeft | Qt::AlignTop);
    menuLayout->addWidget(menuSearchButton, 1, Qt::AlignHCenter | Qt::AlignTop);
    menuLayout->addWidget(menuShowButton, 1, Qt::AlignHCenter | Qt::AlignTop);
    menuLayout->addWidget(menuInfoButton, 1, Qt::AlignHCenter | Qt::AlignTop);
    menuLayout->addWidget(menuQuitButton, 1, Qt::AlignHCenter | Qt::AlignTop);

    //Set the widgets main layout
    menuWidget->setLayout(menuLayout);

    //Configure Signals/Slots for this widget
    connect(menuSearchButton, SIGNAL(clicked()), this, SLOT(menuSearch_clicked()));
    connect(menuShowButton, SIGNAL(clicked()), this, SLOT(menuShow_clicked()));
    connect(menuInfoButton, SIGNAL(clicked()), this, SLOT(menuInfo_clicked()));
    connect(menuQuitButton, SIGNAL(clicked()), this, SLOT(menuQuit_clicked()));

    connect(manager, &QNetworkAccessManager::finished, this, &mainwindow::manager_finished);
    connect(timerAPI, SIGNAL(timeout()), this, SLOT(api_call()));
}

void mainwindow::setupSearch()
{
    searchLayout = new QGridLayout();
    searchThrowerLayout = new QVBoxLayout();
    searchHunterLayout = new QVBoxLayout();
    searchThrowerButton = new QPushButton();
    searchHunterButton = new QPushButton();
    searchLabel = new QLabel();
    searchTextBox = new QLineEdit();
    searchBackButton = new QPushButton();
    searchPopup = new QLabel(searchWidget, Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    searchPopupTimer = new QTimer();
    searchThrowerFile.setFileName(FILE_THROWER);
    searchHunterFile.setFileName(FILE_HUNTER);
    searchThrowOrHunt = true;

    searchThrowerButton->setText("Throwers");
    searchHunterButton->setText("Hunters");
    searchLabel->setText("Search:");
    searchBackButton->setText("Back");

    searchLabel->setStyleSheet("font-size: 30px; font-weight: 400; color: white;");
    searchBackButton->setStyleSheet("font-size: 50px; font-weight:800; color: white; border-style: outset; border-width: 5px; background-color: black");
    searchThrowerButton->setStyleSheet("font-size: 30px; font-weight:400; color: green; background-color: black");
    searchHunterButton->setStyleSheet("font-size: 30px; font-weight:400; color: red; background-color: black");
    searchTextBox->setStyleSheet("QLineEdit{ "
                                    "background-color: rgba(0, 255, 0, 50);"
                                    "border: 2px solid white;"
                                    "padding: 0 8px;"
                                    "selection-background-color: grey;"
                                    "font-size: 30px;}"
                                    "QLineEdit:focus { "
                                    "background: rgba(0, 255, 0, 50);}");
    searchPopup->setStyleSheet("font-size: 20px; font-weight: 400; border: 2px solid rgba(105, 105, 105, 100); border-radius: 5px; padding: 2px; background-color: rgba(105, 105, 105, 100); color: rgba(255, 255, 255, 200)");
    searchWidget->setStyleSheet("background-color: black");
    
    searchBackButton->setCursor(Qt::PointingHandCursor);
    searchThrowerButton->setCursor(Qt::PointingHandCursor);
    searchHunterButton->setCursor(Qt::PointingHandCursor);

    /*searchThrowerLayout->addWidget(searchThrowerLabel);
    searchHunterLayout->addWidget(searchHunterLabel);*/

    searchLayout->addWidget(searchLabel, 0, 0, Qt::AlignTop | Qt::AlignRight);
    searchLayout->addWidget(searchTextBox, 0, 1, 0, 2, Qt::AlignTop);
    searchLayout->addWidget(searchBackButton, 0, 4, Qt::AlignRight);
    searchLayout->addWidget(searchThrowerButton, 1, 0);
    searchLayout->addWidget(searchHunterButton, 1, 2);
    searchLayout->addLayout(searchThrowerLayout, 2, 0, -1, 1);
    searchLayout->addLayout(searchHunterLayout, 2, 2, -1, 3);
    
    searchWidget->setLayout(searchLayout);

    searchGetPlayers();

    connect(searchBackButton, SIGNAL(clicked()), this, SLOT(searchBack_clicked()));
    connect(searchThrowerButton, SIGNAL(clicked()), this, SLOT(searchThrower_clicked()));
    connect(searchHunterButton, SIGNAL(clicked()), this, SLOT(searchHunter_clicked()));
    connect(searchTextBox, SIGNAL(returnPressed()), this, SLOT(searchText_entered()));
    connect(searchPopupTimer, SIGNAL(timeout()), this, SLOT(searchPopup_finished()));
}

void mainwindow::setupShow()
{
    //Initialize all needed variabels
    showLayout = new QGridLayout();
    showWorldLayout = new QGridLayout();
    showSortLayout = new QVBoxLayout();
    showBackButton = new QPushButton();
    showSortNumber = new QPushButton();
    showSortPlayer = new QPushButton();
    showSortUptime = new QPushButton();
    showWorldCounter = new QLabel();
    showSortby = new QLabel();

    //Set worlds "sort by" flag
    showWorldSort = NUMBER;

    //Set buttons and labels text
    showBackButton->setText("Back");
    showSortNumber->setText("Number");
    showSortPlayer->setText("Player");
    showSortUptime->setText("Uptime");
    showSortby->setText("Sort by");

    //Set cursor when hovering over buttons
    showBackButton->setCursor(Qt::PointingHandCursor);
    showSortNumber->setCursor(Qt::PointingHandCursor);
    showSortPlayer->setCursor(Qt::PointingHandCursor);
    showSortUptime->setCursor(Qt::PointingHandCursor);

    //Set color and font style
    showBackButton->setStyleSheet("font-size: 50px; font-weight:800; color: white; border-style: outset;border-width: 5px; background-color: black");
    showSortNumber->setStyleSheet("font-size: 30px; font-weight:800; color: white; border-style: outset;border-width: 5px; background-color: black");
    showSortPlayer->setStyleSheet("font-size: 30px; font-weight:800; color: white; border-style: outset;border-width: 5px; background-color: black");
    showSortUptime->setStyleSheet("font-size: 30px; font-weight:800; color: white; border-style: outset;border-width: 5px; background-color: black");
    showWorldCounter->setStyleSheet("font-size: 40px; font-weight:800; color: white; background-color: black");
    showSortby->setStyleSheet("font-size: 20px; font-weight:600; color: white; background-color: black");
    showWidget->setStyleSheet("background-color: black");

    //Add sort buttons and sort by label to their layout
    showSortLayout->addWidget(showSortby, 0, Qt::AlignCenter | Qt::AlignBottom);
    showSortLayout->addWidget(showSortNumber);
    showSortLayout->addWidget(showSortPlayer);
    showSortLayout->addWidget(showSortUptime);

    //Add everything to the final big layout for this widget
    showLayout->addLayout(showWorldLayout, 0, 0, 4, 6);
    showLayout->addWidget(showBackButton, 0, 6, 1, 1);
    showLayout->addWidget(showWorldCounter, 1, 6, 2, 1, Qt::AlignCenter);
    showLayout->addLayout(showSortLayout, 2, 6, -1, 1);

    //Set widgets layout
    showWidget->setLayout(showLayout);

    //Configure Signals/Slots for this widget
    connect(showBackButton, SIGNAL(clicked()), this, SLOT(showBack_clicked()));
    connect(showSortNumber, SIGNAL(clicked()), this, SLOT(showNumber_clicked()));
    connect(showSortPlayer, SIGNAL(clicked()), this, SLOT(showPlayer_clicked()));
    connect(showSortUptime, SIGNAL(clicked()), this, SLOT(showUptime_clicked()));
}

void mainwindow::setupInfo()
{

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
    api_call();
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

void mainwindow::searchThrower_clicked()
{
    searchThrowOrHunt = true;
    searchTextBox->setStyleSheet("QLineEdit{ "
                                    "background-color: rgba(0, 255, 0, 50);"
                                    "border: 2px solid white;"
                                    "padding: 0 8px;"
                                    "selection-background-color: grey;"
                                    "font-size: 30px;}"
                                    "QLineEdit:focus { "
                                    "background: rgba(0, 255, 0, 50);}");
}

void mainwindow::searchHunter_clicked()
{
    searchThrowOrHunt = false;
    searchTextBox->setStyleSheet("QLineEdit{ "
                                    "background-color: rgba(255, 0, 0, 50);"
                                    "border: 2px solid white;"
                                    "padding: 0 8px;"
                                    "selection-background-color: grey;"
                                    "font-size: 30px;}"
                                    "QLineEdit:focus { "
                                    "background: rgba(255, 0, 0, 50);}");
}

void mainwindow::searchBack_clicked()
{
    stackedWidget->setCurrentWidget(menuWidget);
    currentWidget = MENU;
}

void mainwindow::searchText_entered()
{
    QString enteredText = searchTextBox->text();
    searchTextBox->setText("");

    bool found = false;

    if (searchThrowOrHunt)
    {
        for (int i = 0; i < searchPlayersThrower.size(); i++)
            if (!QString::compare(enteredText, searchPlayersThrower[i].name->text(), Qt::CaseInsensitive))
            {
                found = true;
                return;
            }
    }
    else
    {
        for (int i = 0; i < searchPlayersHunter.size(); i++)
            if (!QString::compare(enteredText, searchPlayersHunter[i].name->text(), Qt::CaseInsensitive))
            {
                found = true;
                return;
            }
    }

    if (!found)
    {
        request.setUrl(QUrl(PLAYERSEARCH + enteredText));
        manager->get(request);
    }
}

void mainwindow::searchPopup_finished()
{
    searchPopup->hide();
    searchPopupTimer->stop();
}

void mainwindow::searchAddPlayer(QString player)
{
    QFile file;
    if (searchThrowOrHunt)
        file.setFileName(FILE_THROWER);
    else
        file.setFileName(FILE_HUNTER);

    if (!file.open(QIODevice::Append | QIODevice::Text))
    {
        qDebug() << "Error opening File " << file.fileName() << " in searchAddPlayer()";
        return;
    }

    QTextStream out(&file);
    if (file.size())
        out << "\n" << player;
    else out << player;

    file.close();
}

void mainwindow::searchGetPlayers()
{
    QFile fileThrower(FILE_THROWER);
    QFile fileHunter(FILE_HUNTER);

    if (!fileThrower.open(QIODevice::ReadWrite | QIODevice::Text) || !fileHunter.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qDebug() << "Error opening File " << fileThrower.fileName() << " or " << fileHunter.fileName()  << " in searchGetPlayer()";
        return;
    }

    QTextStream throwStream(&fileThrower);

    if (!fileThrower.size())
        goto fileThrowerEmpty;

    searchPlayersThrower.resize(0);
    while (!throwStream.atEnd())
    {
        player temp;
        temp.name->setText(throwStream.readLine());
        searchPlayersThrower.append(temp);
    }

fileThrowerEmpty:

    fileThrower.close();

    if (!fileHunter.size())
    {
        fileHunter.close();
        return;
    }

    //New QTextStream needed, cant change its file after constructing
    QTextStream huntStream(&fileHunter);

    searchPlayersHunter.resize(0);
    while (!huntStream.atEnd())
    {
        player temp;
        temp.name->setText(huntStream.readLine());
        searchPlayersHunter.append(temp);
    }
}

void mainwindow::searchPaintPlayers()
{
    if (searchPlayersThrower.size())
        for (int i = 0; i < searchPlayersThrower.size(); i++)
        {
            searchThrowerLayout->removeItem(searchPlayersThrower[i].layout);
            searchThrowerLayout->addLayout(searchPlayersThrower[i].layout);
        }
    if(searchPlayersHunter.size())
        for (int i = 0; i < searchPlayersHunter.size(); i++)
        {
            searchHunterLayout->removeItem(searchPlayersHunter[i].layout);
            searchHunterLayout->addLayout(searchPlayersHunter[i].layout);
        }
}

void mainwindow::searchPopupPaint()
{
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect();
    searchPopup->setGraphicsEffect(effect);
    searchPopup->setAlignment(Qt::AlignCenter);
    QPropertyAnimation* a = new QPropertyAnimation(effect, "opacity");
    a->setDuration(500);
    a->setStartValue(0);
    a->setEndValue(1);
    a->setEasingCurve(QEasingCurve::InBack);
    a->start(QPropertyAnimation::DeleteWhenStopped);
    searchPopup->move(QPoint(geometry().x() + size().width()/2 - size().width()/20, geometry().y() + size().height() / 1.5));
    searchPopup->show();
    searchPopupTimer->start(2000);
}

void mainwindow::showBack_clicked()
{
    stackedWidget->setCurrentWidget(menuWidget);
    currentWidget = MENU;
}

void mainwindow::showNumber_clicked()
{
    if (showWorldSort == NUMBER)
        return;

    showWorldSort = NUMBER;
    timerAPI->stop();
    api_call();
    timerAPI->start(3000);
}

void mainwindow::showPlayer_clicked()
{
    if (showWorldSort == PLAYERS)
        return;

    showWorldSort = PLAYERS;
    timerAPI->stop();
    api_call();
    timerAPI->start(3000);
}

void mainwindow::showUptime_clicked()
{
    if (showWorldSort == UPTIME)
        return;

    showWorldSort = UPTIME;
    timerAPI->stop();
    api_call();
    timerAPI->start(3000);
}
