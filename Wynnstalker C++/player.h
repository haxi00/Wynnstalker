#pragma once

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

class player
{
public:
	player();
	player(const player&);

	QLabel* name;
	QLabel* world;
	QPushButton* deleteButton;
	QHBoxLayout* layout;
};

