#include "MusicNodeManager.h"



MusicNodeManager::MusicNodeManager(Player* p, std::vector<XMFLOAT3> _rails, Mesh* defaultNodeMesh, Material* defaultNodeMaterial, std::vector<Entity*>* e, SMParser* smp, Camera* cam)
{

	recycler = &Recycler::GetInstance();
	player = p;
	nodes = std::vector<MusicNode*>();
	rails = _rails;
	nodeMesh = defaultNodeMesh;
	nodeMat = defaultNodeMaterial;
	entities = e;
	parser = smp;
	cameraRef = cam;
}


MusicNodeManager::~MusicNodeManager()
{
	for (auto node : nodes) {
		delete node;
	}
}

//places a node on given rail at given time-position
//attempts to recycle a node or else initializes a new one
void MusicNodeManager::AddNode(int rail, float time)
{
	MusicNode* node = recycler->ReactivateNode();
	if (node == NULL) {
		Entity* e = new Entity(nodeMesh, nodeMat);
		node = new MusicNode(e, &rails, time, rail, cameraRef);
		entities->push_back(e);
		nodes.push_back(node);
	}
	else {
		node->SetTime(time);
		node->SetRail(rail);
	}
	node->GetEntity()->Activate();
	
}

void MusicNodeManager::AddNode(Entity * e, int rail, float time)
{
	MusicNode* n = new MusicNode(e, &rails, time, rail, cameraRef);
	nodes.push_back(n);
}

void MusicNodeManager::Update(float deltaTime)
{
	myTimer += deltaTime;
	
	int numNotes = parser->GetMeasure(parser->measureNum)->size();
	float secPerBeat = 4 * 60.0 / parser->BPMS;

	// create entities dynamically
	float max = secPerBeat / numNotes;
	if (myTimer > max) {
		counter++;
		if (counter >= numNotes) {
			counter = 0;
			parser->measureNum++;
		}
		myTimer -= max;

		// FOR DEMONSTRATION ONLY
		int value = parser->GetNote(parser->measureNum, counter);

		if (value > -1) {
			// new at front
			printf("%d\n", value);
			AddNode(value, 1);
		}
	}

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
			/*
			printf("%d\n", i);
			if (nodes.size() > 1) {
				for (int j = i; j < nodes.size() - 2; j++) {
					//printf("%d\n", nodes.size());
					nodes[j] = nodes[j + 1];
				}
			}
			nodes.pop_back();
			*/
		}
		//later we will remove nodes instead
	}
}
