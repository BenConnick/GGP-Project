#pragma once
#include "Mesh.h"
#include "Entity.h"
#include "Rail.h"
#include "RailSet.h"

class MusicNode
{
public:
	MusicNode(Entity* e, RailSet* r, float t, int currentR);
	~MusicNode();

	float GetTime();
	int GetCurrentRail();
	void SetTime(float t);


	void Update(float deltaTime);

private:
	Entity* entity;
	//Rail** rails;
	RailSet* rails;

	float time; //how far down the rail node is
	int currentRail; //rail node is attached to
};

