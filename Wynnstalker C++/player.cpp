#include "player.h"

player::player()
{
	name = new QLabel();
	world = new QLabel();
	deleteButton = new QPushButton();
	layout = new QHBoxLayout();

	deleteButton->setText("X");

	name->setStyleSheet("font-size: 20px; font-weight:600; color: white; background-color: black");
	deleteButton->setStyleSheet("font-size: 20px; border: 1xp solid red; font-weight:600; color: red; background-color: black");

	layout->addWidget(deleteButton);
	layout->addWidget(name);
	layout->addWidget(world);
}

player::player(const player& rhs)
{
	name = rhs.name;
	world = rhs.world;
	deleteButton = rhs.deleteButton;
	layout = rhs.layout;
}
