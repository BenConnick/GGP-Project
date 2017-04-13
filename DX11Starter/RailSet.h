#include "Rail.h"

#pragma once
class RailSet
{
public:
	RailSet(Rail** railArray);
	RailSet(Mesh* model, Material* mat, std::vector<Entity*>* entities);
	~RailSet();

	Rail* GetRail(int index);

	static int CONST railCount = 3; // the number of rails in play
	//static int CONST defaultRail = 1; //the rail to use when no user input (currently center rail)


private:

	Rail** rails; //pointer to group of rail objects
};

