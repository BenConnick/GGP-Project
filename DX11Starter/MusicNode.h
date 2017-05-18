#pragma once
#include "Mesh.h"
#include "Entity.h"
#include "Rail.h"
#include "Camera.h"

//node is either alive on rails, playing a hit or miss animation, or is done and ready to be removed
enum NodeState {
	LIVE, HIT, MISS, DEAD
};

class MusicNode
{
public:
	MusicNode(Entity* e, std::vector<XMFLOAT3>* positions, float t, int currentR, Camera* cam);
	~MusicNode();

	//Getters and setters
	Entity* GetEntity();
	float GetTime();
	int GetCurrentRail();
	void SetTime(float t);
	void SetRail(int r);
	NodeState GetState();

	//standard update every frame
	void Update(float deltaTime);

	//interactions with player
	void Hit();
	void Miss();
private:
	Entity* entity; 
	std::vector<XMFLOAT3>* rails;

	XMFLOAT3 posFromTime(float t);

	NodeState state;
	float time; //how far down the rail node is
	int currentRail; //rail node is attached to

	float defaultScale = 0.5f;
	float currentScale = defaultScale;

	float CONST animationSpeed = 8.0f;

	Camera* camera;
};
