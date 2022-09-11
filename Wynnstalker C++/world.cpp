#include "world.h"

world::world()
{
	uptimeLabel = new QLabel;
	layout = new QVBoxLayout;
	button = new QPushButton;

	layout->addWidget(button, 0, Qt::AlignCenter);
	layout->addWidget(uptimeLabel, 0, Qt::AlignCenter);
}

world::world(const world& rhs)
{

	uptimeLabel = new QLabel;
	layout = new QVBoxLayout;
	button = new QPushButton;

	name = rhs.name;
	uptime = rhs.uptime;
	players = rhs.players;
	rect = rhs.rect;
	button = rhs.button;
	uptimeLabel = rhs.uptimeLabel;
	layout = rhs.layout;
}

world::~world()
{
}

QString world::convertUptime()
{
	double hours = (double)(QDateTime::currentDateTime().currentMSecsSinceEpoch() - getUptime())/3600000;
	int minutes = (hours - (int)hours) * 60;
	if ((int)hours && minutes)
		return QString::number((int)hours) + "h " + QString::number(minutes) + "min";
	else if ((int)hours && !minutes)
		return QString::number((int)hours) + "h";
	else return QString::number(minutes) + "min";
}

void world::sortWorlds(QVector<world> *worlds, bool (*function)(world&, world&))
{
	bool change;
	do
	{
		change = false;
		for (qsizetype i = 0; i < worlds->size() - 1; i++)
			if (function((*worlds)[i], (*worlds)[i + 1]))
			{
				world tempWorld = (*worlds)[i];
				(*worlds)[i] = (*worlds)[i + 1];
				(*worlds)[i + 1] = tempWorld;
				change = true;
			}
	} while (change);
}

bool world::cmpNumber(world& a, world& b)
{
	QRegularExpression delimiter("WC");
	QStringList l = a.getName().split(delimiter);
	QStringList r = b.getName().split(delimiter);
	return l[1].toInt() > r[1].toInt() ? true : false;
}

bool world::cmpPlayers(world& a, world& b)
{
	return a.players.size() > b.players.size() ? true : false;
}

bool world::cmpUptime(world& a, world& b)
{
	return a.uptime < b.uptime ? true : false;
}