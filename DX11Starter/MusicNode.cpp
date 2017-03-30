#include "MusicNode.h"



MusicNode::MusicNode(Entity* e, Rail** r, float t, int currentR)
{
	rails = r;
	entity = e;
	time = t;
	currentRail = currentR;

}


MusicNode::~MusicNode()
{
}

float MusicNode::GetTime()
{
	return time;
}

void MusicNode::SetTime(float t)
{
	time = t;
}

void MusicNode::Update(float deltaTime)
{
	time -= deltaTime;
	entity->SetPosition(rails[currentRail]->GetAttachPoint(time));
}