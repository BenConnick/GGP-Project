#include "MusicNodeManager.h"



MusicNodeManager::MusicNodeManager(Player* p, RailSet* r, Mesh* defaultNodeMesh, Material* defaultNodeMaterial)
{
	player = p;
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
		if (node == NULL) { break; }
		node->Update(deltaTime);

		//for the time being we will reset time on nodes so that they loop infinitely
		if (node->GetTime() <= 0.0f) {
			//check for intersection
			if (player != NULL) {
				if (node->GetCurrentRail() == player->GetCurrentRail()) {
					//intersection
					printf("Intersection");
					
				}
			}
			node->SetTime(6.0f);
		}
		//later we will remove nodes instead
	}
}
