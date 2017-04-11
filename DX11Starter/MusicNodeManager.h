#pragma once
#include "Mesh.h"
#include "Entity.h"
#include "Player.h"
#include "RailSet.h"
#include "Rail.h"
#include "MusicNode.h"
#include "MAterial.h"

class MusicNodeManager
{
public:
	MusicNodeManager(Player* p, RailSet* r, Mesh* defaultNodeMesh, Material* defaultNodeMaterial);
	~MusicNodeManager();

	void AddNode(int rail, float time);
	void AddNode(Entity* e, int rail, float time);
	void Update(float deltaTime);
	void RemoveNode(int index);
private:
	std::vector<MusicNode*> nodes;
	//Rail** rails;
	RailSet* rails;

	Player* player;
	Mesh* nodeMesh;
	Material* nodeMat;
};

