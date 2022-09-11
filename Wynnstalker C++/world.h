#pragma once

#include <QString>
#include <QStringList>
#include <QRect>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDateTime>
#include <QRegularExpression>
#include <QVector>

constexpr int MAXWORLDPLAYERS = 45;

class world
{
private:
	QString name;
	qint64 uptime;
	QStringList players;
	QRect rect;

public:
	world();
	world(const world&);
	~world();

	inline const QString getName() { return name; };
	inline const qint64 getUptime() { return uptime; };
	inline const QStringList getPlayers() { return players; };
	inline const QRect getRect() { return rect; };

	inline void setName(QString name) { this->name = name; };
	inline void setUptime(qint64 uptime) { this->uptime = uptime; };
	inline void setPlayers(QStringList players) { this->players = players; };
	inline void setRect(QRect rect) { this->rect = rect; };

	QString convertUptime();
	static void sortWorlds(QVector<world>*, bool (*function)(world&, world&));
	static bool cmpNumber(world&, world&);
	static bool cmpPlayers(world&, world&);
	static bool cmpUptime(world&, world&);

	QVBoxLayout* layout;
	QPushButton* button;
	QLabel* uptimeLabel;
};

enum WorldSort 
{
	NUMBER,
	PLAYERS,
	UPTIME
};

