#include "MusicNode.h"
#include "ParticleManager.h"

MusicNode::MusicNode(Entity* e, std::vector<XMFLOAT3>* positions, float t, int currentR)
{
	rails = positions;
	entity = e;
	entity->SetScale({ defaultScale,defaultScale,defaultScale });
	time = t;
	currentRail = currentR;
}


MusicNode::~MusicNode()
{
}

Entity* MusicNode::GetEntity() {
	return entity;
}
float MusicNode::GetTime()
{
	return time;
}
int MusicNode::GetCurrentRail()
{
	return currentRail;
}
NodeState MusicNode::GetState() {
	return state;
}
void MusicNode::SetTime(float t)
{
	time = t;
	state = NodeState::LIVE;
	entity->SetScale({ defaultScale,defaultScale,defaultScale });
	currentScale = defaultScale;
}void MusicNode::SetRail(int r)
{
	currentRail = r;
	XMFLOAT3 pos = posFromTime(0);
	ParticleManager::GetInstance().EmitMedParticle(XMFLOAT3(pos.x,pos.y,pos.z+20), XMFLOAT3(0, 0, -20));
}

void MusicNode::Update(float deltaTime)
{
	if (state == NodeState::DEAD) { return; }
	time -= deltaTime;
	XMFLOAT3 pos = posFromTime(time);
	entity->SetPosition(XMFLOAT3(pos.x, pos.y - 5, pos.z));

	//miss animation // shrink to nothing
	if (state == NodeState::MISS) {
		//shrink
		currentScale -= deltaTime*animationSpeed;
		//cant shrink any farther
		if (currentScale < 0.0f) { 
			currentScale = 0.0f; 
			state = NodeState::DEAD;
		}
		//apply new scale
		entity->SetScale({ currentScale,currentScale,currentScale });
	}
	//hit animation // node is unseeable under the player, no animation plays, but this might be a good place to put SFX later
	if (state == NodeState::HIT) {
		state = NodeState::DEAD;
	}
}
void MusicNode::Hit()
{
	state = NodeState::HIT;
}
void MusicNode::Miss()
{
	state = NodeState::MISS;
}

XMFLOAT3 MusicNode::posFromTime(float t) {
	XMFLOAT3 pos = rails->at(currentRail);
	XMFLOAT3 point = { pos.x,pos.y,pos.z + t * 20 };
	return point;
}