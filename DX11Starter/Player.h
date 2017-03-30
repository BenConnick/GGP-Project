#include "Mesh.h"
#include "Entity.h"
#include "Rail.h"

#pragma once
class Player
{
public:
	Player(Entity* e, Rail** r);
	~Player();

	Entity* entity;

	Rail** rails; //pointer to group of rail objects

	//increment rail up or down
	void MoveLeft();
	void MoveRight();
	//return to default rail
	void MoveDefault();

private:
	int railCount = 3; // the number of rails in play
	int defaultRail = 1; //the rail to use when no user input (currently center rail)
	int currentRail = defaultRail; //rail in use
};

