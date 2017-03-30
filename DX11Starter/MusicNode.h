#pragma once
#include "Mesh.h"
#include "Entity.h"
#include "Rail.h"

class MusicNode
{
public:
	MusicNode(Entity* e, Rail** r, float t, int currentR);
	~MusicNode();

	float GetTime();
	void SetTime(float t);

	void Update(float deltaTime);

private:
	Entity* entity;
	Rail** rails;

	float time; //how far down the rail node is
	int currentRail; //rail node is attached to
};

