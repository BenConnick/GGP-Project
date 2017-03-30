#include "MusicNodeManager.h"



MusicNodeManager::MusicNodeManager(Rail** r, Mesh* defaultNodeMesh, Material* defaultNodeMaterial)
{
	nodes = std::vector<MusicNode*>();
	rails = r;
	nodeMesh = defaultNodeMesh;
	nodeMat = defaultNodeMaterial;
}


MusicNodeManager::~MusicNodeManager()
{
}

void MusicNodeManager::AddNode(int rail, float time)
{
	Entity* e = new Entity(nodeMesh,nodeMat);
	AddNode(e, rail, time);
}

void MusicNodeManager::AddNode(Entity * e, int rail, float time)
{
	MusicNode* n = new MusicNode(e,rails, time, rail);
	nodes.push_back(n);
}

void MusicNodeManager::Update(float deltaTime)
{
	for(auto node : nodes){
	//for (int i = 0; i < nodes.size; i++) {
		node->Update(deltaTime);

		//for the time being we will reset time on nodes so that they loop infinitely
		if (node->GetTime() <= 0.0f) {
			node->SetTime(6.0f);
		}
		//later we will remove nodes instead
	}
}
