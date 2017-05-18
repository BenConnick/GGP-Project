#pragma once
#include "Mesh.h"
#include "Entity.h"
#include "Player.h"
#include "RailSet.h"
#include "Rail.h"
#include "MusicNode.h"
#include "Material.h"
#include "Recycler.h"
#include "SMParser.h"

class MusicNodeManager
{
public:
	MusicNodeManager(Player* p, std::vector<XMFLOAT3> rails, Mesh* defaultNodeMesh, Material* defaultNodeMaterial, std::vector<Entity*>* e, SMParser* smp);
	~MusicNodeManager();

	void AddNode(int rail, float time);
	void AddNode(Entity* e, int rail, float time);
	void Update(float deltaTime);
	void RemoveNode(int index);
private:
	std::vector<MusicNode*> nodes;
	std::vector<XMFLOAT3> rails;

	Player* player;

	float myTimer = 0.0f;
	int counter = 0;

	//default resources for creating generic musicnodes
	Mesh* nodeMesh;
	Material* nodeMat;

	std::vector<Entity*>* entities;
	Recycler* recycler;
	SMParser* parser;
};

