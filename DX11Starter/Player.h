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

	void Update();

	//increment rail up or down
	void MoveLeft();
	void MoveRight();
	//return to default rail
	void MoveDefault();

	static int CONST railCount = 3; // the number of rails in play
	static int CONST defaultRail = 1; //the rail to use when no user input (currently center rail)

private:
	
	int currentRail = defaultRail; //rail in use

	int prevMove; //usable to determine previous move state, and prevent slidng across all rails with a single button press
	//presently we are going with a 3-rail, middle reset approach, so this is not being used

	bool defaultReset=true;//reset to default position when no input enabled
};

