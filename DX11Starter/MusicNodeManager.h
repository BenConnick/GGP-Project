#pragma once
#include "Mesh.h"
#include "Entity.h"
#include "Rail.h"
#include "MusicNode.h"
#include "MAterial.h"

class MusicNodeManager
{
public:
	MusicNodeManager(Rail** r, Mesh* defaultNodeMesh, Material* defaultNodeMaterial);
	~MusicNodeManager();

	void AddNode(int rail, float time);
	void AddNode(Entity* e, int rail, float time);
	void Update(float deltaTime);
	void RemoveNode(int index);
private:
	std::vector<MusicNode*> nodes;
	Rail** rails;

	Mesh* nodeMesh;
	Material* nodeMat;
};

