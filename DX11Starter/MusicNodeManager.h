#pragma once
#include "Mesh.h"
#include "Entity.h"
#include "Player.h"
#include "RailSet.h"
#include "Rail.h"
#include "MusicNode.h"
#include "Material.h"
#include "Recycler.h"

class MusicNodeManager
{
public:
	MusicNodeManager(Player* p, RailSet* r, Mesh* defaultNodeMesh, Material* defaultNodeMaterial, std::vector<Entity*>* e);
	~MusicNodeManager();

	void AddNode(int rail, float time);
	void AddNode(Entity* e, int rail, float time);
	void Update(float deltaTime);
	void RemoveNode(int index);
private:
	std::vector<MusicNode*> nodes;
	RailSet* rails;

	Player* player;

	//default resources for creating generic musicnodes
	Mesh* nodeMesh;
	Material* nodeMat;

	std::vector<Entity*>* entities;
	Recycler* recycler;
};

