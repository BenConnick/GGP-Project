#include "MusicNode.h"
#include "ParticleManager.h"

MusicNode::MusicNode(Entity* e, RailSet* r, float t, int currentR)
{
	rails = r;
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
	ParticleManager::GetInstance().EmitMedParticle(rails->GetRail(currentRail)->GetAttachPoint(0), XMFLOAT3(0, 0, -10));
}

void MusicNode::Update(float deltaTime)
{
	if (state == NodeState::DEAD) { return; }
	time -= deltaTime;
	entity->SetPosition(rails->GetRail(currentRail)->GetAttachPoint(time));

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