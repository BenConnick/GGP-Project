#include "MusicNodeManager.h"



MusicNodeManager::MusicNodeManager(Player* p, RailSet* r, Mesh* defaultNodeMesh, Material* defaultNodeMaterial)
{

	recycler = &Recycler::GetInstance();
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
	//loop through all nodes
	for(int i = 0; i < nodes.size(); i++){
		MusicNode* node = nodes[i];
		//for(auto node : nodes){
		if (node == NULL) { continue; } //handle null

		//update node
		node->Update(deltaTime);

		//check if node is in range of player
		if (node->GetTime() <= 0.0f && node->GetState() == NodeState::LIVE) {

			//check for intersection
			if (player != NULL) {
				//player is on same rail (hit)
				if (node->GetCurrentRail() == player->GetCurrentRail()) {
					//intersection
					node->Hit();
					player->Hit();
				}
				else { //player missed node
					node->Miss();
				}
			}
			//node->SetTime(6.0f);
		}
		if (node->GetState() == NodeState::DEAD) {
			recycler->Deactivate(node);
			for (int j = 0; j < nodes.size()-1; j++) {
				nodes[i+j] = nodes[i+j+1];
			}
			nodes.pop_back();
			//node->SetTime(6.0f);
		}
		//later we will remove nodes instead
	}
}
